#ifndef _LOOKUP3_H
#define _LOOKUP3_H

#include <stdint.h>
#include <stdlib.h>

uint32_t lookup3_hashword(
	const uint32_t *k,       /* the key, an array of uint32_t values */
	size_t          length,  /* the length of the key, in uint32_ts */
	uint32_t        initval);

void lookup3_hashword2 (
	const uint32_t *k,      /* the key, an array of uint32_t values */
	size_t          length, /* the length of the key, in uint32_ts */
	uint32_t       *pc,     /* IN: seed OUT: primary hash value */
	uint32_t       *pb);    /* IN: more seed OUT: secondary hash value */

uint32_t lookup3_hashlittle(const void *key, size_t length, uint32_t initval);

void lookup3_hashlittle2(
  const void *key,    /* the key to hash */
  size_t      length, /* length of the key */
  uint32_t   *pc,     /* IN: primary initval, OUT: primary hash */
  uint32_t   *pb);    /* IN: secondary initval, OUT: secondary hash */

#endif
