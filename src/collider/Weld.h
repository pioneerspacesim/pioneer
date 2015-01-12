// This code is in the public domain -- castanyo@yahoo.es

#ifndef NV_MESH_WELD_H
#define NV_MESH_WELD_H

#include "libs.h"
#include "Containers.h"

// Weld function to remove array duplicates in linear time using hashing.

namespace nv
{

template <class T> struct Equal
{
	bool operator()(const T & a, const T & b) const
	{
		return a == b;
	}
};

/// Null index. @@ Move this somewhere else... This could have collisions with other definitions!
#define NIL Uint32(~0)


/// Generic welding routine. This function welds the elements of the array p
/// and returns the cross references in the xrefs array. To compare the elements
/// it uses the given hash and equal functors.
/// 
/// This code is based on the ideas of Ville Miettinen and Pierre Terdiman.
template <class T, class H=hash<T>, class E=Equal<T> >
struct Weld
{
	// xrefs maps old elements to new elements
	Uint32 operator()(Array<T> & p, Array<Uint32> & xrefs)
	{
		const Uint32 N = p.size();							// # of input vertices.
		Uint32 outputCount = 0;								// # of output vertices
		Uint32 hashSize = nextPowerOfTwo(N);					// size of the hash table
		Uint32 * hashTable = new Uint32[hashSize + N];			// hash table + linked list
		Uint32 * next = hashTable + hashSize;					// use bottom part as linked list

		xrefs.resize(N);
		memset( hashTable, NIL, hashSize*sizeof(Uint32) );	// init hash table (NIL = 0xFFFFFFFF so memset works)

		H hash;
		E equal;
		for (Uint32 i = 0; i < N; i++)
		{
			const T & e = p[i];
			Uint32 hashValue = hash(e) & (hashSize-1);
			Uint32 offset = hashTable[hashValue];

			// traverse linked list
			while( offset != NIL && !equal(p[offset], e) )
			{
				offset = next[offset];
			}

			xrefs[i] = offset;

			// no match found - copy vertex & add to hash
			if( offset == NIL )
			{
				// save xref
				xrefs[i] = outputCount;

				// copy element
				p[outputCount] = e;

				// link to hash table
				next[outputCount] = hashTable[hashValue];

				// update hash heads and increase output counter
				hashTable[hashValue] = outputCount++;
			}
		}

		// cleanup
		delete [] hashTable;

		p.resize(outputCount);
		
		// number of output vertices
		return outputCount;
	}
};


/// Reorder the given array accoding to the indices given in xrefs.
template <class T>
void reorderArray(Array<T> & array, const Array<Uint32> & xrefs)
{
	const Uint32 count = xrefs.count();
	Array<T> new_array(count);

	for(Uint32 i = 0; i < count; i++) {
		new_array[i] = array[xrefs[i]];
	}

	swap(array, new_array);
}

/// Reverse the given array so that new indices point to old indices.
inline void reverseXRefs(Array<Uint32> & xrefs, Uint32 count)
{
	Array<Uint32> new_xrefs(count);
	
	for(Uint32 i = 0; i < xrefs.count(); i++) {
		new_xrefs[xrefs[i]] = i;
	}
	
	swap(xrefs, new_xrefs);
}

} // nv namespace

#endif // NV_MESH_WELD_H
