// This code is in the public domain -- castanyo@yahoo.es

#ifndef NV_MESH_WELD_H
#define NV_MESH_WELD_H

#include <SDL_stdinc.h>
#include <vector>
#include <cassert>

// Weld function to remove array duplicates in linear time using hashing.

namespace nv {

	template <class T>
	struct Equal {
		bool operator()(const T &a, const T &b) const
		{
			return a == b;
		}
	};

/// Null index. @@ Move this somewhere else... This could have collisions with other definitions!
#define NIL Uint32(~0)

	template <typename Key>
	struct hash {
		inline Uint32 sdbm_hash(const void *data_in, Uint32 size, Uint32 h = 5381)
		{
			const Uint8 *data = static_cast<const Uint8 *>(data_in);
			Uint32 i = 0;
			while (i < size) {
				h = (h << 16) + (h << 6) - h + static_cast<Uint32>(data[i++]);
			}
			return h;
		}

		Uint32 operator()(const Key &k)
		{
			return sdbm_hash(&k, sizeof(Key));
		}
	};
	template <>
	struct hash<int> {
		Uint32 operator()(int x) const { return x; }
	};
	template <>
	struct hash<Uint32> {
		Uint32 operator()(Uint32 x) const { return x; }
	};

	/** Return the next power of two.
* @see http://graphics.stanford.edu/~seander/bithacks.html
* @warning Behaviour for 0 is undefined.
* @note isPowerOfTwo(x) == true -> nextPowerOfTwo(x) == x
* @note nextPowerOfTwo(x) = 2 << log2(x-1)
*/
	inline Uint32 nextPowerOfTwo(Uint32 x)
	{
		assert(x != 0);
#if 1 // On modern CPUs this is supposed to be as fast as using the bsr instruction.
		x--;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		return x + 1;
#else
		Uint32 p = 1;
		while (x > p) {
			p += p;
		}
		return p;
#endif
	}

	/// Return true if @a n is a power of two.
	inline bool isPowerOfTwo(Uint32 n)
	{
		return (n & (n - 1)) == 0;
	}

	/// Generic welding routine. This function welds the elements of the array p
	/// and returns the cross references in the xrefs array. To compare the elements
	/// it uses the given hash and equal functors.
	///
	/// This code is based on the ideas of Ville Miettinen and Pierre Terdiman.
	template <class T, class H = hash<T>, class E = Equal<T>>
	struct Weld {
		// xrefs maps old elements to new elements
		Uint32 operator()(std::vector<T> &p, std::vector<Uint32> &xrefs)
		{
			const Uint32 N = p.size(); // # of input vertices.
			Uint32 outputCount = 0; // # of output vertices
			Uint32 hashSize = nextPowerOfTwo(N); // size of the hash table
			Uint32 *hashTable = new Uint32[hashSize + N]; // hash table + linked list
			Uint32 *next = hashTable + hashSize; // use bottom part as linked list

			xrefs.resize(N);
			memset(hashTable, NIL, (hashSize + N) * sizeof(Uint32)); // init hash table (NIL = 0xFFFFFFFF so memset works)

			H hash;
			E equal;
			for (Uint32 i = 0; i < N; i++) {
				const T &e = p[i];
				const Uint32 hashValue = hash(e) & (hashSize - 1);
				//const Uint32 hashValue = CodeSupHash(e) & (hashSize-1);
				Uint32 offset = hashTable[hashValue];

				// traverse linked list
				while (offset != NIL && !equal(p[offset], e)) {
					offset = next[offset];
				}

				xrefs[i] = offset;

				// no match found - copy vertex & add to hash
				if (offset == NIL) {
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
			delete[] hashTable;

			p.resize(outputCount);

			// number of output vertices
			return outputCount;
		}
	};

} // namespace nv

#endif // NV_MESH_WELD_H
