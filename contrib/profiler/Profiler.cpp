#define __PROFILER_SMP__
#define __PROFILER_CONSOLIDATE_THREADS__

#define css_outline_color "#848484"
#define css_thread_style "background-color:#EEEEEE;margin-top:8px;"
#define css_title_row "<tr class=\"header\"><td class=\"left\">Function</td><td>Calls</td><td>MCycles</td><td>Avg</td><td>Self MCycles</td><td class=\"right\">Self Avg</td></tr>\n"
#define css_totals_row "<tr class=\"header\"><td class=\"left\">Function</td><td>Calls</td><td>Self MCycles</td><td class=\"right\">Self Avg</td></tr>\n"

#if defined(_WIN32)
	#define _CRT_SECURE_NO_WARNINGS
	#define copystring _strdup
	#include <windows.h>
#else
	#define copystring strdup
	#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

#include "Profiler.h"

#if defined(__ICC) || defined(__ICL)
	#pragma warning( disable: 1684 ) // (size_t )name >> 5
	#pragma warning( disable: 1011 ) // missing return statement at end of non-void function
#endif

#undef threadlocal

#if defined(_MSC_VER)
	#define YIELD() Sleep(0);
	#define PRINTFU64() "%I64u"
	#define PATHSLASH() '\\'
	#define threadlocal __declspec(thread)
	#define snprintf _snprintf

	#undef inline
	#define inline __forceinline
#else
	#include <sched.h>
	#define YIELD() sched_yield();
	#define PRINTFU64() "%llu"
	#define PATHSLASH() '/'
	#define threadlocal __thread
#endif

#if !defined(__PROFILER_SMP__)
	#undef threadlocal
	#define threadlocal
#endif 

namespace Profiler {

	#if defined(__PROFILER_SMP__)
		#if defined(_MSC_VER)

			template< class type >
			inline bool CAS( volatile type &ptr_, const type old_, const type new_ ) {
				__asm {
					mov eax, [old_]
					mov edx, [new_]
					mov ecx, [ptr_]
					lock cmpxchg dword ptr [ecx], edx
					sete al
				}
			}

		#elif defined(__GNUC__) || defined(__ICC)

			template< class type >
			inline bool CAS( volatile type &ptr_, const type old_, const type new_ ) {
				u8 ret;
				__asm__ __volatile__ (
					"  lock\n"
					"  cmpxchgl %2,%1\n"
					"  sete %0\n"
						: "=q" (ret), "=m" (ptr_)
						: "r" (new_), "m" (ptr_), "a" (old_)
						: "memory"
				);
				return ret;
			}

		#else
			#error Define a compare-and-swap / full memory barrier implementation!
		#endif

		struct CASLock {
			void Acquire() { while ( !CAS( mLock, u32(0), u32(1) ) ) YIELD() }
			void Release() { while ( !CAS( mLock, u32(1), u32(0) ) ) YIELD() }
			bool TryAcquire() { return CAS( mLock, u32(0), u32(1) ); }
			bool TryRelease() { return CAS( mLock, u32(1), u32(0) ); }
			u32 Value() const { return mLock; }
		//protected:
			volatile u32 mLock;
		};
	#else
		struct CASLock {
			void Acquire() {}
			void Release() {}
			bool TryAcquire() { return false; }
			bool TryRelease() { return false; }
			u32 Value() const { return 0; }
			u32 dummy;
		};
	#endif

	u32 nextpow2( u32 x ) {
		x |= ( x >>  1 );
		x |= ( x >>  2 );
		x |= ( x >>  4 );
		x |= ( x >>  8 );
		x |= ( x >> 16 );
		return ( x + 1 );
	}

	template< class type >
	inline void zeroarray( type *array, size_t count ) {
		memset( array, 0, count * sizeof( type ) );
	}

	template< class type >
	inline type *makepointer( type *base, size_t byteoffset ) {
		return (type *)((const char *)base + byteoffset);
	}

	template< class type >
	inline void swapitems( type &a, type &b ) {
		type tmp = a;
		a = b;
		b = tmp;
	}

	#undef min
	#undef max

	template< class type >
	inline const type &min( const type &a, const type &b ) {
		return ( a < b ) ? a : b;
	}

	template< class type >
	inline const type &max( const type &a, const type &b ) {
		return ( a < b ) ? b : a;
	}

	/*
	=============
	Buffer - Don't use for anything with a constructor/destructor. Doesn't shrink on popping
	=============
	*/

	template< class type >
	struct Buffer {
		Buffer() : mBuffer(NULL), mAlloc(0), mItems(0) { Resize( 4 ); }
		Buffer( u32 size ) : mBuffer(NULL), mAlloc(0), mItems(0) { Resize( size ); }
		~Buffer() { free( mBuffer ); }

		void Clear() { mItems = ( 0 ); }
		type *Data() { return ( mBuffer ); }
		void EnsureCapacity( u32 capacity ) { if ( capacity >= mAlloc ) Resize( capacity * 2 ); }
		type *Last() { return ( &mBuffer[ mItems - 1 ] ); }
		void Push( const type &item ) { EnsureCapacity( mItems + 1 ); mBuffer[ mItems++ ] = ( item ); }
		type &Pop() { return ( mBuffer[ --mItems ] ); }

		void Resize( u32 newsize ) {
			mAlloc = nextpow2( newsize );
			mBuffer = (type *)realloc( mBuffer, mAlloc * sizeof( type ) );
		}

		u32 Size() const { return mItems; }

		template< class Compare >
		void Sort( Compare comp ) {
			if ( mItems <= 1 )
				return;
			
			Buffer scratch( mItems );

			// merge sort with scratch buffer
			type *src = Data(), *dst = scratch.Data();
			for( u32 log = 2; log < mItems * 2; log *= 2 ) {
				type *out = dst;
				for( u32 i = 0; i < mItems; i += log ) {
					u32 lo = i, lo2 = min( i + log / 2, mItems );
					u32 hi = lo2, hi2 = min( lo + log, mItems );
					while ( ( lo < lo2 ) && ( hi < hi2 ) )
						*out++ = ( comp( src[lo], src[hi] ) ) ? src[lo++] : src[hi++];
					while ( lo < lo2 ) *out++ = src[lo++];
					while ( hi < hi2 ) *out++ = src[hi++];
				}

				swapitems( src, dst );
			}

			if ( src != mBuffer )
				swapitems( mBuffer, scratch.mBuffer );
		}

		template< class Mapto >
		void ForEachByRef( Mapto &mapto, u32 limit ) {
			limit = ( limit < mItems ) ? limit : mItems;
			u32 last = limit - 1;
			for ( u32 i = 0; i < limit; ++i )
				mapto( mBuffer[ i ], i == last );
		}

		template< class Mapto >	void ForEach( Mapto mapto, u32 limit ) { ForEachByRef( mapto, limit ); }
		template< class Mapto >	void ForEach( Mapto mapto ) { ForEachByRef( mapto, mItems ); }

		type &operator[] ( u32 index ) { return ( mBuffer[ index ] ); }
		const type &operator[] ( u32 index ) const { return ( mBuffer[ index ] ); }

	protected:
		type *mBuffer;
		u32 mAlloc, mItems;
	};	


	/*
	===================
	ColorRamp for HTML
	===================
	*/

	struct ColorF {
		ColorF() {}
		ColorF( f32 r_, f32 g_, f32 b_ ) : r(r_), g(g_), b(b_) {}
		f32 r, g, b;
	};

	struct ColorRamp {
		struct Marker {
			Marker() {}
			Marker( const ColorF &color_, f32 value_ ) : color(color_), value(value_) {}
			ColorF color;
			f32 value;
		};

		ColorRamp() {}

		void clear() {
			mColors.Clear();
		}
		
		const char *value( f32 pos ) const {
			ColorF base(0, 0, 0);
			u32 pre = 0, post = 0;
			for ( pre = 0; pre < mColors.Size() - 1; pre++ )
				if ( mColors[pre+1].value >= pos )
					break;
			post = pre + 1;
			if ( ( pre < mColors.Size() ) && ( post < mColors.Size() ) ) {
				const Marker &a = mColors[pre], &b = mColors[post];
				f32 dist = ( b.value - a.value ), posw = ( pos - a.value ), bw = ( posw / dist ), aw = 1 - bw;
				base = ColorF( a.color.r * aw + b.color.r * bw, a.color.g * aw + b.color.g * bw, a.color.b * aw + b.color.b * bw );
			}
			u8 r = u8(base.r * 255.0f), g = u8(base.g * 255.0f), b = u8(base.b * 255.0f);
			static threadlocal char buffer[8][32], bufferon = 0;
			sprintf( buffer[bufferon&7], "#%02x%02x%02x", r, g, b );
			return buffer[bufferon++&7];
		}

		ColorRamp &push( const ColorF &color, f32 value ) { mColors.Push( Marker( color, value ) ); return *this; }

		Buffer<Marker> mColors;
	};

	/*
	=============
	Caller
	=============
	*/

	#pragma pack(push,1)
	struct Caller {
		struct foreach {
			// Adds each Caller to the specified buckets
			struct AddToNewBuckets {
				AddToNewBuckets( Caller **buckets, u32 bucket_count ) : mBuckets(buckets), mBucketCount(bucket_count) {}
				void operator()( Caller *item ) {
					FindEmptyChildSlot( mBuckets, mBucketCount, item->mName ) = item;
				}
				Caller **mBuckets;
				u32 mBucketCount;
			};


			// Destructs a Caller
			struct Deleter { 
				void operator()( Caller *item ) { 
					delete item;
				} 
			};

			// Merges a Caller with the root
			struct Merger {
				Merger( Caller *root ) : mRoot(root) {}
				void addFrom( Caller *item ) { (*this)( item ); }
				void operator()( Caller *item ) {
					Caller *child = mRoot->FindOrCreate( item->GetName() );
					child->GetTimer() += item->GetTimer();
					child->SetParent( item->GetParent() );
					item->ForEachNonEmpty( Merger( child ) );
				}
				Caller *mRoot;
			};

			// Prints a Caller
			struct Printer {
				Printer( u32 indent ) : mIndent(indent) { }
				void operator()( Caller *item, bool islast ) const {
					item->Print( mIndent, islast );
				}
				u32 mIndent;
			};

			struct PrinterHtml {
				PrinterHtml( FILE *f, u32 indent ) : mFile(f), mIndent(indent) { }
				void operator()( Caller *item, bool islast ) const {
					item->PrintHtml( mFile, mIndent, islast );
				}
				FILE *mFile;
				u32 mIndent;
			};

			struct SoftReset { 
				void operator()( Caller *item ) { 
					item->GetTimer().SoftReset();
					item->ForEach( SoftReset() );
				} 
			};

			// Sums Caller's ticks 
			struct SumTicks {
				SumTicks() : sum(0) {}
				void operator()( Caller *item ) { 
					sum += ( item->mTimer.ticks );
				}
				u64 sum;
			};

			struct UpdateTopMaxStats {
				UpdateTopMaxStats() { maxStats.reset(); }
				void operator()( Caller *item, bool islast ) {
					if ( !item->GetParent() )
						return;
					maxStats.check( Max::Calls, item->mTimer.calls );
					maxStats.check( Max::Ms, Timer::ms( item->mTimer.ticks ) );
					maxStats.check( Max::Avg, item->mTimer.avgms() );
				}
			};
		}; // foreach


		struct compare {
			struct Ticks { 
				bool operator()( const Caller *a, const Caller *b ) const { 
					return ( a->mTimer.ticks > b->mTimer.ticks ); 
				} 
			};

			struct SelfTicks { 
				bool operator()( const Caller *a, const Caller *b ) const { 
					return ( ( a->mTimer.ticks - a->mChildTicks ) > ( b->mTimer.ticks - b->mChildTicks ) ); 
				} 
			};

			struct Calls { 
				bool operator()( const Caller *a, const Caller *b ) const { 
					return ( a->mTimer.calls > b->mTimer.calls ); 
				} 
			};
		}; // sort


		/*
			Since Caller.mTimer.ticks is inclusive of all children, summing the first level
			children of a Caller to Caller.mChildTicks is an accurate total of the complete
			child tree.

			mTotals is used to keep track of total ticks by Caller excluding children
		*/
		struct ComputeChildTicks {
			ComputeChildTicks( Caller &totals ) : mTotals(totals) { maxStats.reset(); }
			void operator()( Caller *item ) {
				foreach::SumTicks sumchildren;
				item->ForEachByRefNonEmpty( sumchildren );
				item->mChildTicks = ( sumchildren.sum );

				u64 selfticks = ( item->mTimer.ticks >= item->mChildTicks ) ? ( item->mTimer.ticks - item->mChildTicks ) : 0;
				Caller &totalitem = ( *mTotals.FindOrCreate( item->mName ) );
				totalitem.mTimer.ticks += selfticks;
				totalitem.mTimer.calls += item->mTimer.calls;
				totalitem.SetParent( item->GetParent() );

				// don't include the root node in the max stats
				if ( item->GetParent() ) {
					maxStats.check( Max::SelfMs, Timer::ms( selfticks ) );
					maxStats.check( Max::Calls, item->mTimer.calls );
					maxStats.check( Max::Ms, Timer::ms( item->mTimer.ticks ) );
					maxStats.check( Max::Avg, item->mTimer.avgms() );
					maxStats.check( Max::SelfAvg, average( Timer::ms( selfticks ), item->mTimer.calls ) );
				}

				// compute child ticks for all children of children of this caller
				item->ForEachByRefNonEmpty( *this );
			}
			Caller &mTotals;
		};

		/*
			Format a Caller's information. ComputeChildTicks will need to be used on the Root
			to generate mChildTicks for all Callers
		*/
		struct Format {
			Format( const char *prefix ) : mPrefix(prefix) {}
			void operator()( Caller *item, bool islast ) const {
				u64 ticks = item->mTimer.ticks;
				f64 ms = Timer::ms( ticks );
				printf( "%s %.2f mcycles, %d calls, %.0f cycles avg, %.2f%%: %s\n", 
					mPrefix, ms, item->mTimer.calls, item->mTimer.avg(), average( ticks * 100, mGlobalDuration ), item->mName );
			}
			const char *mPrefix;
		};

		struct FormatHtml {
			FormatHtml( FILE *f, Buffer<const char *> &prefix ) : mFile(f), mPrefix(prefix) {}
			void operator()( Caller *item ) const {
				fprintf( mFile, "\t<tr %s><td><table class=\"tree\"><tr>", !item->GetParent() ? "style=\"" css_thread_style "\"": "class=\"h\"" );
				for ( u32 i = 0; i < mPrefix.Size(); i++ )
					fprintf( mFile, "<td><img src=\"%s\" /></td>", mPrefix[i] );
				u64 ticks = item->mTimer.ticks;
				f64 ms = Timer::ms( ticks ), globalpct = average( ticks * 100, mGlobalDuration );
				f64 childms = Timer::ms( item->mChildTicks ), selfms = ( ms - childms ), avg = item->mTimer.avgms(), selfavg = average( selfms, item->mTimer.calls );
				if ( !item->GetParent() )
					fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\">%u</td><td class=\"number\">%0.4f (%3.0f%%)</td><td class=\"number\">%0.4f</td><td class=\"number\">%0.4f</td><td class=\"number\">%0.4f</td></tr>\n", 
						item->mName, 
						item->mTimer.calls,
						ms,
						globalpct,
						avg,
						selfms,
						selfavg
					);
				else
					fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\" style=\"background-color:%s\">%u</td><td class=\"number\" style=\"background-color:%s\">%0.4f (%3.0f%%)</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td></tr>\n", 
						item->mName, 
						maxStats.color( Max::Calls, item->mTimer.calls ),  item->mTimer.calls,
						maxStats.color( Max::Ms, ms ), ms,
						globalpct,
						maxStats.color( Max::Avg, avg ), avg,
						maxStats.color( Max::SelfMs, selfms ), selfms,
						maxStats.color( Max::SelfAvg, selfavg ), selfavg
					);
			}
			FILE *mFile;
			Buffer<const char *> &mPrefix;
		};

		struct FormatHtmlTop {
			FormatHtmlTop( FILE *f ) : mFile(f) {}
			void operator()( Caller *item, bool islast ) const {
				fprintf( mFile, "\t<tr %s><td><table class=\"tree\"><tr>", !item->GetParent() ? "style=\"" css_thread_style "\"": "class=\"h\"" );
				fprintf( mFile, "<td><img src=\"%s\" /></td>", islast ? "img/last-empty.gif" : "img/empty.gif" );
				u64 ticks = item->mTimer.ticks;
				f64 ms = Timer::ms( ticks ), globalpct = average( ticks * 100, mGlobalDuration ), avg = item->mTimer.avgms();
				if ( !item->GetParent() ) {
					fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\">%u</td><td class=\"number\">%0.4f (%.0f%%)</td><td class=\"number\">%0.4f</td></tr>\n", 
						item->mName, 
						item->mTimer.calls,
						ms,
						globalpct,
						avg
					);
				} else {
					fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\" style=\"background-color:%s\">%u</td><td class=\"number\" style=\"background-color:%s\">%0.4f (%.0f%%)</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td></tr>\n", 
						item->mName, 
						maxStats.color( Max::Calls, item->mTimer.calls ),  item->mTimer.calls,
						maxStats.color( Max::Ms, ms ), ms,
						globalpct,
						maxStats.color( Max::Avg, avg ), avg
					);
				}
			}
			FILE *mFile;
		};

		/* 
			Methods
		*/

		// we're guaranteed to be null because of calloc. ONLY create Callers with "new"!
		Caller( const char *name, Caller *parent = NULL ) { 
			mName = name;
			mParent = parent;
			Resize( 2 ); // mBuckets must always exist and mBucketCount >= 2!
		}
		
		~Caller() { 
			ForEach( foreach::Deleter() );
			free( mBuckets );
		}

		void CopyToListNonEmpty( Buffer<Caller *> &list ) {
			list.Clear();

			for ( u32 i = 0; i < mBucketCount; ++i )
				if ( mBuckets[ i ] && !mBuckets[ i ]->GetTimer().IsEmpty() )
					list.Push( mBuckets[ i ] );
		}

		inline Caller *FindOrCreate( const char *name ) {
			u32 index = ( GetBucket( name, mBucketCount ) ), mask = ( mBucketCount - 1 );
			for ( Caller *caller = mBuckets[index]; caller; caller = mBuckets[index & mask] ) {
				if ( caller->mName == name )
					return caller;
				
				index = ( index + 1 );
			}

			// didn't find the caller, lock this thread and mutate
			AcquirePerThreadLock();
			EnsureCapacity( ++mNumChildren );
			Caller *&slot = FindEmptyChildSlot( mBuckets, mBucketCount, name );
			slot = new Caller( name, this );
			ReleasePerThreadLock();
			return slot;
		}

		template< class Mapto >
		void ForEachByRef( Mapto &mapto ) {
			for ( u32 i = 0; i < mBucketCount; ++i )
				if ( mBuckets[ i ] )
					mapto( mBuckets[ i ] );
		}

		template< class Mapto >
		void ForEachByRefNonEmpty( Mapto &mapto ) {
			for ( u32 i = 0; i < mBucketCount; ++i )
				if ( mBuckets[ i ] && !mBuckets[ i ]->GetTimer().IsEmpty() )
					mapto( mBuckets[ i ] );
		}

		template< class Mapto > 
		void ForEach( Mapto mapto ) {
			ForEachByRef( mapto );
		}

		template< class Mapto > 
		void ForEachNonEmpty( Mapto mapto ) {
			ForEachByRefNonEmpty( mapto );
		}

		inline Caller *GetParent() { 
			return mParent; 
		}

		Timer &GetTimer() { 
			return mTimer;
		}

		const char *GetName() const {
			return mName;
		}

		bool IsActive() const {
			return mActive;
		}

		void Print( u32 indent = 0, bool islast = false ) {
			Buffer<Caller *> children( mNumChildren );
			CopyToListNonEmpty( children );

			mFormatter.EnsureCapacity( indent + 3 );
			char *fmt = ( &mFormatter[indent] );
			
			if ( indent ) {
				fmt[-2] = ( islast ) ? ' ' : '|';
				fmt[-1] = ( islast ) ? '\\' : ' ';
			}
			fmt[0] = ( children.Size() ) ? '+' : '-';
			fmt[1] = ( '-' );
			fmt[2] = ( 0 );
			
			Format(mFormatter.Data())( this, islast );

			if ( indent && islast )
				fmt[-2] = fmt[-1] = ' ';

			if ( children.Size() ) {
				children.Sort( compare::Ticks() );
				children.ForEach( foreach::Printer(indent+2) );
			}
		}

		void PrintHtml( FILE *f, u32 indent = 0, bool islast = false ) {
			Buffer<Caller *> children( mNumChildren );
			CopyToListNonEmpty( children );

			if ( !indent ) {
				mHTMLFormatter.Push( "img/root.gif" );
			} else if ( children.Size() ) {
				mHTMLFormatter.Push( ( ( islast ) || ( children.Size() == 1 ) ) ? "img/last-child-open.gif" : "img/open.gif" );
			} else {
				mHTMLFormatter.Push( ( islast ) ? "img/last-empty.gif" : "img/empty.gif" );
			}
			
			FormatHtml(f, mHTMLFormatter)( this );

			mHTMLFormatter[indent] = ( islast || !indent ) ? "img/blank.gif" : "img/vertical.gif";	

			if ( children.Size() ) {
				children.Sort( compare::Ticks() );
				children.ForEach( foreach::PrinterHtml(f,indent+1) );
			}

			mHTMLFormatter.Pop();		
		}

		void PrintTopStats( u32 nitems ) {
			nitems = ( nitems > mNumChildren ) ? mNumChildren : nitems;
			printf( "\ntop %u functions (self time)\n", (u32 )nitems );
			Buffer<Caller *> sorted( mNumChildren );
			CopyToListNonEmpty( sorted );
			sorted.Sort( compare::SelfTicks() );
			sorted.ForEach( Format(">"), nitems );
		}

		void Resize( u32 new_size ) {
			new_size = ( new_size < mBucketCount ) ? mBucketCount << 1 : nextpow2( new_size - 1 );
			Caller **new_buckets = (Caller **)calloc( new_size, sizeof( Caller* ) );
			ForEach( foreach::AddToNewBuckets( new_buckets, new_size ) );

			free( mBuckets );
			mBuckets = ( new_buckets );
			mBucketCount = ( new_size );
		}

		void Reset() {
			ForEach( foreach::Deleter() );
			zeroarray( mBuckets, mBucketCount );
			mNumChildren = ( 0 );
			mTimer.Reset();			
		}

		void SetActive( bool active ) {
			mActive = active;
		}

		void SetParent( Caller *parent ) {
			mParent = parent;
		}

		void SoftReset() {
			mTimer.SoftReset();
			ForEach( foreach::SoftReset() );
		}

		void Start() {
			mTimer.Start();
		}

		void Stop() {
			mTimer.Stop();
		}

		void *operator new ( size_t size ) {
			return calloc( size, 1 );
		}

		void operator delete ( void *p ) { 
			free( p );
		}


		/* Acquire the caller lock for this thread */

		inline static void AcquirePerThreadLock() {
#if defined(__PROFILER_SMP__)
			if ( thisThread.requireThreadLock )
				thisThread.threadLock.Acquire();
#endif
		}

		inline static void ReleasePerThreadLock() {
#if defined(__PROFILER_SMP__)
			if ( thisThread.requireThreadLock )
				thisThread.threadLock.Release();
#endif
		}

	protected:
		static inline Caller *&FindEmptyChildSlot( Caller **buckets, u32 bucket_count, const char *name ) {
			u32 index = ( GetBucket( name, bucket_count ) ), mask = ( bucket_count - 1 );
			Caller **caller = &buckets[index];
			for ( ; *caller; caller = &buckets[index & mask] )
				index = ( index + 1 );
			return *caller;
		}

		inline static u32 GetBucket( const char *name, u32 bucket_count ) {
			return u32( ( ( (size_t )name >> 5 ) /* * 2654435761 */ ) & ( bucket_count - 1 ) );
		}

		inline void EnsureCapacity( u32 capacity ) {
			if ( capacity < ( mBucketCount / 2 ) )
				return;
			Resize( capacity );
		}

	protected:
		const char *mName;
		Timer mTimer;
		u32 mBucketCount, mNumChildren;
		Caller **mBuckets, *mParent;

		bool mActive;
		u64 mChildTicks;

	public:
		// caller
		static Buffer<char> mFormatter;
		static Buffer<const char *> mHTMLFormatter;
		static ColorRamp mColors;

		// global
		static f64 mTimerOverhead, mRdtscOverhead;
		static u64 mGlobalDuration;
		static struct Max {
			enum f64Enum { SelfMs = 0, Ms, Avg, SelfAvg, f64Enums };
			enum u64Enum { Calls = 0, TotalCalls, u64Enums };

			void reset() {
				memset( this, 0, sizeof( *this ) );
			}

			void check( u64Enum e, u64 u ) { if ( u64fields[e] < u ) u64fields[e] = u; if ( e == Calls ) u64fields[TotalCalls] += u; }
			void check( f64Enum e, f64 f ) { if ( f64fields[e] < f ) f64fields[e] = f; }

			const char *color( u64Enum e, u64 u ) const { return mColors.value( f32(f64(u)/f64(u64fields[e])) ); }
			const char *color( f64Enum e, f64 f ) const { return mColors.value( f32(f/f64fields[e]) ); }

			const u64 &operator() ( u64Enum e ) const { return u64fields[e]; }
			const f64 &operator() ( f64Enum e ) const { return f64fields[e]; }

		protected:
			u64 u64fields[u64Enums];
			f64 f64fields[f64Enums];
		} maxStats;

		// per thread state
		struct ThreadState {
			CASLock threadLock;
			bool requireThreadLock;
			Caller *activeCaller;
		};
		
		static threadlocal ThreadState thisThread;
	};
	#pragma pack(pop)






#if defined(__PROFILER_ENABLED__)
	threadlocal Caller::ThreadState Caller::thisThread = { {0}, 0, 0 };
	f64 Caller::mTimerOverhead = 0, Caller::mRdtscOverhead = 0;
	u64 Caller::mGlobalDuration = 0;
	Caller::Max Caller::maxStats;
	Buffer<char> Caller::mFormatter( 64 );
	Buffer<const char *> Caller::mHTMLFormatter( 64 );
	ColorRamp Caller::mColors;
	char *programName = NULL, *commandLine = NULL;

	void detectByArgs( int argc, char **argv ) {
		const char *path = argv[0], *finalSlash = path, *iter = path;
		for ( ; *iter; ++iter )
			finalSlash = ( *iter == PATHSLASH() ) ? iter + 1 : finalSlash;
		if ( !*finalSlash )
			finalSlash = path;
		programName = copystring( finalSlash );
		
		size_t width = 0;
		for ( int i = 1; i < argc; i++ ) {
			size_t len = strlen( argv[i] );
			commandLine = (char *)realloc( commandLine, width + len + 1 );
			memcpy( commandLine + width, argv[i], len );
			commandLine[width + len] = ' ';
			width += len + 1;
		}
		if ( width )
			commandLine[width - 1] = '\x0';
	}

	/*void detectWinMain( const char *cmdLine ) {
#if defined(_MSC_VER)
		char path[1024], *finalSlash = path, *iter = path;
		GetModuleFileName( NULL, path, 1023 );
		for ( ; *iter; ++iter )
			finalSlash = ( *iter == PATHSLASH() ) ? iter + 1 : finalSlash;
		if ( !*finalSlash )
			finalSlash = path;
		programName = copystring( finalSlash );
		commandLine = copystring( cmdLine );
#else
		programName = copystring( "only_for_win32" );
		commandLine = copystring( "" );
#endif
	}*/

	/*
	============
	Root - Holds the root caller and the thread state for a thread
	============
	*/

	struct Root {
		Root( Caller *caller, Caller::ThreadState *ts ) : root(caller), threadState(ts) {}
		Caller *root;
		Caller::ThreadState *threadState;
	};

	struct GlobalThreadList {
		~GlobalThreadList() { 
			if ( list ) {
				Buffer<Root> &threadsref = *list;
				u32 cnt = threadsref.Size();
				for ( u32 i = 0; i < cnt; i++ )
					delete threadsref[i].root;
			}
			delete list; 
		}

		void AcquireGlobalLock() { 
			threadsLock.Acquire(); 
			if ( !list )
				list = new Buffer<Root>;
		}

		void ReleaseGlobalLock() {
			threadsLock.Release();
		}

		Buffer<Root> *list;
		CASLock threadsLock;
	};

	u64 globalStart = Timer::getticks();
	GlobalThreadList threads = { NULL, {0} };
	threadlocal Caller *root = NULL;
	

	/*
		Thread Dumping
	*/

	struct PrintfDumper {
		void Init() {
		}

		void GlobalInfo( u64 rawCycles ) {
			printf( "> Raw run time %.2f mcycles\n", Timer::ms( rawCycles ) );
		}

		void ThreadsInfo( u64 totalCalls, f64 timerOverhead, f64 rdtscOverhead ) {
			printf( "> Total calls " PRINTFU64() ", per call overhead %.0f cycles, rdtsc overhead %.0f cycles, estimated overhead %.2f mcycles\n\n",
				totalCalls, timerOverhead, rdtscOverhead, Timer::ms( timerOverhead * totalCalls ) );
		}

		void PrintThread( Caller *root ) {
			root->Print();
			printf( "\n\n" );
		}

		void PrintAccumulated( Caller *accumulated ) {
			accumulated->PrintTopStats( 50 );
		}

		void Finish() {
		}
	};

	struct HTMLDumper {
		void Init() {
			Caller::mColors.clear();
			Caller::mColors.push( ColorF( 255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f ), 0.00f );
			//Caller::mColors.push( ColorF( 255.0f/255.0f, 212.0f/255.0f, 129.0f/255.0f ), 0.50f );
			Caller::mColors.push( ColorF( 255.0f/255.0f,  203.0f/255.0f,  203.0f/255.0f ), 0.20f );
			Caller::mColors.push( ColorF( 255.0f/255.0f,  128.0f/255.0f,  128.0f/255.0f ), 1.00f );

			time_t now;
			time( &now );
			tm *now_tm = localtime( &now );
			strftime( timeFormat, 255, "%Y%m%d_%H%M%S", now_tm );
			snprintf( fileFormat, 255, "%s-profile-%s.html", programName ? programName : "no-info-given", timeFormat );
			strftime( timeFormat, 255, "%#c", now_tm );			
			f = fopen( fileFormat, "wb+" );

			Caller::mHTMLFormatter.Clear();
			fputs(
				"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\" class=\"\">\n"
				"<head>\n"
				"	<style type=\"text/css\">\n"
				"		body {font-family: arial;font-size: 11px;}\n"
				"		table {padding: 0px;margin: 0px;border-spacing: 0pt 0pt;}\n"
				"		table.tree td {padding: 0px; margin: 0px;}\n"
				"		tr {padding: 0px;margin: 0px;}\n"
				"		tr.h:hover {background-color: #EEEEEE; color:blue;}\n"
				"		tr.header {background-image: url(img/tile.gif); background-position: left bottom; background-repeat: repeat-x; height: 24px;}\n"
				"		tr.header td { padding-left: 8px; padding-right:8px; border-right:1px solid " css_outline_color "; border-top:1px solid " css_outline_color "; }\n"
				"		tr.header td.left { border-left:1px solid " css_outline_color "; }\n"
				"		tr.header td.right { border-right:1px solid " css_outline_color "; }\n"
				"		tr.spacer {height: 24px;}\n"
				"		td {padding: 0px;padding-left:3px;padding-right:3px;margin: 0px;}\n"
				"		td.text {text-align: left;}\n"
				"		td.number {text-align: right;}\n"
				"		div.overall { background-color: #F0F0F0; width: 98%; color: #A31212; font-size: 16px; padding: 5px; padding-left: 20px; margin-bottom: 15px; }\n"
				"		div.thread { margin-bottom: 15px; }\n"
				"		div.overall td.title { padding-left: 10px; font-weight: bold; }\n"
				"	</style>\n"
				"</head>\n"
				"<body>\n\n",			
				f
			);
		}

		void GlobalInfo( u64 rawCycles ) {
			fputs( "<div class=\"overall\"><table>", f );
			if ( programName ) {
				fprintf( f, "<tr><td class=\"title\">Command Line: </td><td>%s", programName );
				if ( commandLine )
					fprintf( f, " %s", commandLine );
				fputs( "</td></tr>", f );
			}
			fprintf( f, "<tr><td class=\"title\">Date: </td><td>%s</td></tr><tr><td class=\"title\">Raw run time: </td><td>%.2f mcycles</td></tr>\n", 
				timeFormat, Timer::ms( rawCycles ) );
		}

		void ThreadsInfo( u64 totalCalls, f64 timerOverhead, f64 rdtscOverhead ) {
			fprintf( f, "<tr><td class=\"title\">Total calls: </td><td>" PRINTFU64() "</td></tr>\n", totalCalls );
			fprintf( f, "<tr><td class=\"title\">rdtsc overhead: </td><td>%.0f cycles</td></tr>\n", rdtscOverhead );
			fprintf( f, "<tr><td class=\"title\">Per call overhead: </td><td>%.0f cycles</td></tr>\n", timerOverhead );
			fprintf( f, "<tr><td class=\"title\">Estimated overhead: </td><td>%.4f mcycles</td></tr>\n", Timer::ms( totalCalls * timerOverhead ) );
			fprintf( f, "</table></div>\n" );
		}

		void PrintThread( Caller *root ) {
			fputs( "<div class=\"thread\"><table>\n", f );
			fputs( css_title_row, f );
			root->PrintHtml( f );
			fputs( "</table></div>\n", f );
		}

		void PrintAccumulated( Caller *accumulated ) {
			fputs( "<div class=\"thread\"><table>\n", f );
			fputs( css_totals_row, f );
			fputs( "<tr style=\"" css_thread_style "\"><td><table><tr><td><img src=\"img/root.gif\" /></td><td>Functions sorted by self time</td></tr></table></td><td></td><td></td><td></td></tr>\n", f );
			Buffer<Caller *> sorted;
			accumulated->CopyToListNonEmpty( sorted );
			sorted.ForEach( Caller::foreach::UpdateTopMaxStats() );
			sorted.Sort( Caller::compare::SelfTicks() );		
			sorted.ForEach( Caller::FormatHtmlTop(f) );
			fputs( "</table></div>\n", f );
		}

		void Finish() {
			fputs( "</div>\n", f );
			fputs( "</body></html>", f );
			fclose( f );
		}

	protected:
		FILE *f;
		char timeFormat[256], fileFormat[256];
	};

	template< class Dumper >
	void dumpThreads( Dumper dumper ) {
		u64 rawDuration = ( Timer::getticks() - globalStart );

		Caller *accumulate = new Caller( "/Top Callers" ), *packer = new Caller( "/Thread Packer" );
		Buffer<Caller *> packedThreads;

		dumper.Init();
		dumper.GlobalInfo( rawDuration );

		threads.AcquireGlobalLock();	

		// crawl the list of theads and store their data in to packer
		Buffer<Root> &threadsref = *threads.list;
		for ( u32 i = 0; i < threadsref.Size(); i++ ) {
			Root &thread = threadsref[i];

			// if the thread is no longer active, the lock won't be valid
			bool active = ( thread.root->IsActive() );
			if ( active ) {
				thread.threadState->threadLock.Acquire();
				// disable requiring our local lock in case the caller is in our thread, accumulate will try to set it otherwise
				Caller::thisThread.requireThreadLock = false;
				for ( Caller *walk = thread.threadState->activeCaller; walk; walk = walk->GetParent() )
					walk->GetTimer().SoftStop();
			}

#if defined(__PROFILER_CONSOLIDATE_THREADS__)
			// merge the thread into the packer object, will result in 1 caller per thread name, not 1 caller per thread instance
			Caller::foreach::Merger( packer ).addFrom( thread.root );
			Caller *child = packer->FindOrCreate( thread.root->GetName() );

			// add the child to the list of threads to dump (use the active flag to indicate if it's been added)
			if ( !child->IsActive() ) {
				packedThreads.Push( child );
				child->SetActive( true );
			}
#else
			// create a dummy entry for each thread (fake a name with the address of the thread root)
			Caller *stub = packer->FindOrCreate( (const char *)thread.root );
			Caller::foreach::Merger( stub ).addFrom( thread.root );
			Caller *stubroot = stub->FindOrCreate( thread.root->GetName() );
			stubroot->SetParent( NULL ); // for proper crawling
			packedThreads.Push( stubroot );
#endif

			if ( active ) {
				Caller::thisThread.requireThreadLock = true;
				thread.threadState->threadLock.Release();
			}
		}

		// working on local data now, don't need the threads lock any more
		threads.ReleaseGlobalLock();	

		// do the pre-computations on the gathered threads
		Caller::ComputeChildTicks preprocessor( *accumulate );
		for ( u32 i = 0; i < packedThreads.Size(); i++ )
			preprocessor( packedThreads[i] );

		dumper.ThreadsInfo( Caller::maxStats( Caller::Max::TotalCalls ), Caller::mTimerOverhead, Caller::mRdtscOverhead );

		// print the gathered threads
		u64 sumTicks = 0;
		for ( u32 i = 0; i < packedThreads.Size(); i++ ) {
			Caller *root = packedThreads[i];
			u64 threadTicks = root->GetTimer().ticks;
			sumTicks += threadTicks;
			Caller::mGlobalDuration = threadTicks;
			dumper.PrintThread( root );
		}

		// print the totals, use the summed total of ticks to adjust percentages
		Caller::mGlobalDuration = sumTicks;
		dumper.PrintAccumulated( accumulate );		
		dumper.Finish();

		delete accumulate;
		delete packer;
	}

	void resetThreads() {
		globalStart = Timer::getticks();

#if defined(__PROFILER_SMP__)
		threads.AcquireGlobalLock();

		Buffer<Root> &threadsref = *threads.list;
		u32 cnt = threadsref.Size(), last = cnt - 1;
		for ( u32 i = 0; i < cnt; i++ ) {
			Root &thread = threadsref[i];
			if ( !thread.root->IsActive() ) {
				// thread isn't active, remove it
				delete thread.root;
				Root removed = threadsref.Pop();
				if ( i != last )
					thread = removed;
				last--;
				cnt--;
				i--;
			} else {
				thread.threadState->threadLock.Acquire();
				thread.root->SoftReset();
				Caller *iter = thread.threadState->activeCaller;
				for ( ; iter; iter = iter->GetParent() )
					iter->GetTimer().calls = 1;
				thread.threadState->threadLock.Release();
			}
		}

		threads.ReleaseGlobalLock();
#else
		if ( root )
			root->SoftReset();
#endif
	}

	void enterThread( const char *name ) {
		Caller *tmp = new Caller( name );

		threads.AcquireGlobalLock();
		threads.list->Push( Root( tmp, &Caller::thisThread ) );

		Caller::AcquirePerThreadLock();
		Caller::thisThread.activeCaller = tmp;
		tmp->Start();
		tmp->SetActive( true );
		root = tmp;
		Caller::ReleasePerThreadLock();

		threads.ReleaseGlobalLock();
	}

	void exitThread() {
		threads.AcquireGlobalLock();

		Caller::AcquirePerThreadLock();
		root->Stop();
		root->SetActive( false );
		Caller::thisThread.activeCaller = NULL;
		Caller::ReleasePerThreadLock();

		threads.ReleaseGlobalLock();
	}

	inline void fastcall enterCaller( const char *name ) {
		Caller *parent = Caller::thisThread.activeCaller;
		if ( !parent )
			return;
		
		Caller *active = parent->FindOrCreate( name );
		active->Start();
		Caller::thisThread.activeCaller = active;
	}

	inline void exitCaller() {
		Caller *active = Caller::thisThread.activeCaller;
		if ( !active )
			return;
		
		active->Stop();
		Caller::thisThread.activeCaller = active->GetParent();
	}

	inline void pauseCaller() {
		u64 curticks = Timer::getticks();
		Caller *iter = Caller::thisThread.activeCaller;
		for ( ; iter; iter = iter->GetParent() )
			iter->GetTimer().Pause( curticks );
	}

	inline void unpauseCaller() {
		u64 curticks = Timer::getticks();
		Caller *iter = Caller::thisThread.activeCaller;
		for ( ; iter; iter = iter->GetParent() )
			iter->GetTimer().Unpause( curticks );
	}

	// enter the main thread automatically
	struct MakeRoot {
		MakeRoot() {
			// get an idea of how long timer calls / rdtsc takes
			const u32 reps = 1000;
			Caller::mTimerOverhead = Caller::mRdtscOverhead = 1000000;			
			for ( u32 tries = 0; tries < 20; tries++ ) {
				Timer t, t2;
				t.Start();
				for ( u32 i = 0; i < reps; i++ ) {
					t2.Start();
					t2.Stop();
				}
				t.Stop();
				f64 avg = f64(t2.ticks)/f64(reps);
				if ( avg < Caller::mRdtscOverhead )
					Caller::mRdtscOverhead = avg;
				avg = f64(t.ticks)/f64(reps);
				if ( avg < Caller::mTimerOverhead )
					Caller::mTimerOverhead = avg;
			}

			enterThread( "/Main" );
		}

		~MakeRoot() {
			free( programName );
			free( commandLine );
		}
	} makeRoot;

	void detect( int argc, char **argv ) { detectByArgs( argc, argv ); }
	//void detect( const char *commandLine ) { detectWinMain( commandLine ); }
	void dump() { dumpThreads( PrintfDumper() ); }
	void dumphtml() { dumpThreads( HTMLDumper() ); }
	void fastcall enter( const char *name ) { enterCaller( name ); }
	void fastcall exit() { exitCaller(); }
	void fastcall pause() { pauseCaller(); }
	void fastcall unpause() { unpauseCaller(); }
	void threadenter( const char *name ) { enterThread( name ); }
	void threadexit() { exitThread(); }
	void reset() { resetThreads(); }
#else
	void detect( int argc, char **argv ) {}
	//void detect( const char *commandLine ) {}
	void dump() {}
	void dumphtml() {}
	void fastcall enter( const char *name ) {}
	void fastcall exit() {}
	void fastcall pause() {}
	void fastcall unpause() {}
	void threadenter( const char *name ) {}
	void threadexit() {}
	void reset() {}
#endif

}; // namespace Profiler
