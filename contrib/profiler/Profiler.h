#ifndef __PROFILER_H__
#define __PROFILER_H__

#ifdef PIONEER_PROFILER
#define __PROFILER_ENABLED__
#endif

#define __PROFILER_FULL_TYPE_EXPANSION__

//#define USE_CHRONO
#if !defined(USE_CHRONO) && (defined(__arm__) || defined(__aarch64__) || defined(__PPC64__) || defined(_M_AMD64) || defined(_WIN64) || defined(_M_X64) || defined(__riscv))
// this isn't optional for __arm__ or x64 builds
#define USE_CHRONO
#endif

#include <chrono>
#include <ratio>

#if defined(_MSC_VER)
	#undef __PRETTY_FUNCTION__
	#define __PRETTY_FUNCTION__ __FUNCSIG__
	#define PROFILE_CONCAT( a, b ) a "/" b

	#define fastcall __fastcall
#else
	#define PROFILE_CONCAT( a, b ) b

	#define fastcall
#endif

#if defined(__PROFILER_FULL_TYPE_EXPANSION__)
	#define PROFILE_FUNCTION() __PRETTY_FUNCTION__
#else
	#define PROFILE_FUNCTION() __FUNCTION__
#endif

	// http://stackoverflow.com/questions/1597007/creating-c-macro-with-and-line-token-concatenation-with-positioning-macr
	#define _PROFILE_JOIN2(x,y) x##y
	#define _PROFILE_JOIN(x,y) _PROFILE_JOIN2(x,y)
	#define PROFILE_LINENAME(x) _PROFILE_JOIN(x,__LINE__)

#if defined(__PROFILER_ENABLED__)
	// thread
	#define PROFILE_THREAD_START_RAW( text )   Profiler::threadenter( text );
	#define PROFILE_THREAD_START()             PROFILE_THREAD_START_RAW( PROFILE_FUNCTION()  )
	#define PROFILE_THREAD_START_DESC( desc )  PROFILE_THREAD_START_RAW( PROFILE_CONCAT( PROFILE_FUNCTION(), desc ) )

	#define PROFILE_THREAD_SCOPED_RAW( text )  Profiler::ScopedThread PROFILE_LINENAME(profiler) ( text );
	#define PROFILE_THREAD_SCOPED()            PROFILE_THREAD_SCOPED_RAW( PROFILE_FUNCTION() )
	#define PROFILE_THREAD_SCOPED_DESC( desc ) PROFILE_THREAD_SCOPED_RAW( PROFILE_CONCAT( PROFILE_FUNCTION(), desc ) )

	#define PROFILE_THREAD_STOP()              Profiler::threadexit();

	// function
	#define PROFILE_PAUSE()             Profiler::pause();
	#define PROFILE_UNPAUSE()           Profiler::unpause();
	#define PROFILE_PAUSE_SCOPED()      Profiler::ScopedPause PROFILE_LINENAME(profilerpause);

	#define PROFILE_START_RAW( text )   Profiler::enter( text );
	#define PROFILE_START()             PROFILE_START_RAW( PROFILE_FUNCTION()  )
	#define PROFILE_START_DESC( desc )  PROFILE_START_RAW( PROFILE_CONCAT( PROFILE_FUNCTION(), desc ) )

	#define PROFILE_SCOPED_RAW( text )  Profiler::Scoped PROFILE_LINENAME(profiler) ( text );
	#define PROFILE_SCOPED()            PROFILE_SCOPED_RAW( PROFILE_FUNCTION() )
	#define PROFILE_SCOPED_DESC( desc ) PROFILE_SCOPED_RAW( PROFILE_CONCAT( PROFILE_FUNCTION(), desc ) )

	#define PROFILE_STOP()              Profiler::exit();
#else
	#define PROFILE_THREAD_START_RAW( text )
	#define PROFILE_THREAD_START()
	#define PROFILE_THREAD_START_DESC( desc )

	#define PROFILE_THREAD_SCOPED_RAW( text )
	#define PROFILE_THREAD_SCOPED()
	#define PROFILE_THREAD_SCOPED_DESC( desc )

	#define PROFILE_THREAD_STOP()

	#define PROFILE_PAUSE()
	#define PROFILE_UNPAUSE()
	#define PROFILE_PAUSE_SCOPED()

	#define PROFILE_START_RAW( text )
	#define PROFILE_START()
	#define PROFILE_START_DESC( desc )

	#define PROFILE_SCOPED_RAW( text )
	#define PROFILE_SCOPED()
	#define PROFILE_SCOPED_DESC( desc )

	#define PROFILE_STOP()
#endif

namespace Profiler {
	/*
	=============
	Types that won't conflict with the rest of the system
	=============
	*/
	typedef float f32;
	typedef double f64;
	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	#if defined(_MSC_VER)
		typedef unsigned __int64 u64;
	#else
		typedef unsigned long long u64;
	#endif

	template< class type1, class type2 >
	f64 average( type1 sum, type2 count ) {
		return ( count ) ? f64(sum)/f64(count) : 0;
	}

	/*
	=============
	Timer
	=============
	*/
	#pragma pack(push,1)
	struct Timer {
		Timer() { Reset(); }

		inline bool IsEmpty() const { return ticks == 0; }
		inline bool IsPaused() const { return paused; }
		inline void Unpause( u64 curticks ) { started = curticks; paused = false; }
		inline void Unpause() { Unpause( getticks() ); }
		inline void Pause( u64 curticks ) { ticks += ( curticks - started ); paused = true; }
		inline void Pause() { Pause( getticks() ); }
		inline void Start() { ++calls; started = getticks(); }
		inline void Stop() { ticks += ( getticks() - started ); }
		inline void Reset() { ticks = started = calls = 0; paused = false; }
		inline void SoftStop() { if ( !paused ) { u64 t = getticks(); ticks += ( t - started ); started = t; } }
		inline void SoftReset() { ticks = 0; calls = 0; started = getticks(); }

		template< class type > static f64 ms( const type &t ) { return f64( t ) / 1000000.0; }
		f64 millicycles() { return ms( ticks ); }
		f64 currentmillicycles() { return ms( ticks + ( getticks() - started ) ); }
		f64 avg() { return average( ticks, calls ); }
		f64 avgms() { return ms( average( ticks, calls ) ); }

		void operator+= ( const Timer &b ) {
			ticks += b.ticks;
			calls += b.calls;
		}

	#if !defined(USE_CHRONO)
		#if defined(__GNUC__)
			static inline u64 getticks() {
				u32 __a,__d;
				asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
				return ( u64(__a) | u64(__d) << 32 );
			}
		#elif defined(__ICC) || defined(__ICL)
			static inline u64 getticks() { return _rdtsc(); }
		#elif defined(_MSC_VER)
			static inline u64 getticks() { __asm { rdtsc }; }
		#endif
	#else
		static inline u64 getticks() { return std::chrono::high_resolution_clock::now().time_since_epoch().count(); }
	#endif

		u64 ticks, started;
		u32 calls;
		bool paused;
	};
	#pragma pack(pop)

	#pragma pack(push, 1)
	struct Clock {
		Clock() { Reset(); }

		inline bool IsEmpty() const { return ticks == 0; }
		inline bool IsPaused() const { return paused; }
		inline void Unpause(u64 curticks)
		{
			started = curticks;
			paused = false;
		}
		inline void Unpause() { Unpause(getticks()); }
		inline void Pause(u64 curticks)
		{
			ticks += (curticks - started);
			paused = true;
		}
		inline void Pause() { Pause(getticks()); }
		inline void Start()
		{
			++calls;
			started = getticks();
		}
		inline void Stop() { ticks += (getticks() - started); }
		inline void Reset()
		{
			ticks = started = calls = 0;
			paused = false;
		}
		inline void SoftStop()
		{
			if (!paused) {
				u64 t = getticks();
				ticks += (t - started);
				started = t;
			}
		}
		inline void SoftReset()
		{
			ticks = 0;
			calls = 0;
			started = getticks();
		}

		static f64 ms(const u64 t) {
			std::chrono::duration<f64, std::milli> dur = std::chrono::steady_clock::duration(t);
			return dur.count();
		}

		f64 seconds() {
			std::chrono::duration<f64> dur = std::chrono::steady_clock::duration(ticks);
			return dur.count();
		}

		f64 milliseconds() { return ms(ticks); }
		f64 currentmilliseconds() { return ms(ticks + (getticks() - started)); }
		f64 avg() { return average(ticks, calls); }
		f64 avgms() { return ms(average(ticks, calls)); }

		void operator+=(const Clock &b)
		{
			ticks += b.ticks;
			calls += b.calls;
		}

		static inline u64 getticks()
		{
			return std::chrono::steady_clock::now().time_since_epoch().count();
		}

		u64 ticks, started;
		u32 calls;
		bool paused;
	};
	#pragma pack(pop)

	/*
	=============
	Interface functions
	=============
	*/

	void detect( int argc, char **argv );
	//void detect( const char *commandLine );
	void dump(const char *dir = 0);
	void dumptrace(const char *dir = 0);
	void dumpzones(const char *dir = 0);
	void dumphtml(const char *dir = 0);
	void fastcall enter( const char *name );
	void fastcall exit();
	void fastcall pause();
	void fastcall unpause();
	void threadenter( const char *name );
	void threadexit();
	void reset();

	struct Scoped {
		Scoped( const char *name ) { PROFILE_START_RAW( name ) }
		~Scoped() { PROFILE_STOP() }
	};

	struct ScopedPause {
		ScopedPause() { PROFILE_PAUSE() }
		~ScopedPause() { PROFILE_UNPAUSE() }
	};

	struct ScopedThread {
		ScopedThread( const char *name ) { PROFILE_THREAD_START_RAW( name ); }
		~ScopedThread() { PROFILE_THREAD_STOP() }
	};
} // namespace Profiler

#endif // __PROFILER_H__
