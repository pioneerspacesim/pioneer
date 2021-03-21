/*
 * 2D, 3D and 4D Perlin noise, classic and simplex, in a GLSL fragment shader.
 *
 * Classic noise is implemented by the functions:
 * float noise(vec2 P)
 * float noise(vec3 P)
 * float noise(vec4 P)
 *
 * Simplex noise is implemented by the functions:
 * float snoise(vec2 P)
 * float snoise(vec3 P)
 * float snoise(vec4 P)
 *
 * Author: Stefan Gustavson ITN-LiTH (stegu@itn.liu.se) 2004-12-05
 * Simplex indexing functions by Bill Licea-Kane, ATI
 */

/*
This code was irrevocably released into the public domain
by its original author, Stefan Gustavson, in January 2011.
Please feel free to use it for whatever you want.
Credit is appreciated where appropriate, and I also
appreciate being told where this code finds any use,
but you may do as you like. Alternatively, if you want
to have a familiar OSI-approved license, you may use
This code under the terms of the MIT license:

Copyright (C) 2004 by Stefan Gustavson. All rights reserved.
This code is licensed to you under the terms of the MIT license:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
 * The value of classic 4D noise goes above 1.0 and below -1.0 at some
 * points. Not much and only very sparsely, but it happens. This is a
 * bug from the original software implementation, so I left it untouched.
 */


/*
 * "permTexture" is a 256x256 texture that is used for both the permutations
 * and the 2D and 3D gradient lookup. For details, see the main C program.
 * "gradTexture" is a 256x256 texture with 4D gradients, similar to
 * "permTexture" but with the permutation index in the alpha component
 * replaced by the w component of the 4D gradient.
 * 2D classic noise uses only permTexture.
 * 2D simplex noise uses only permTexture.
 * 3D classic noise uses only permTexture.
 * 3D simplex noise uses only permTexture.
 * 4D classic noise uses permTexture and gradTexture.
 * 4D simplex noise uses permTexture and gradTexture.
 */
uniform sampler2D permTexture;
uniform sampler2D gradTexture;

/*
 * Both 2D and 3D texture coordinates are defined, for testing purposes.
 */
#ifdef FRAGMENT_SHADER
in vec2 v_texCoord2D;
in vec3 v_texCoord3D;
in vec4 v_color;
#endif // FRAGMENT_SHADER

/*
 * To create offsets of one texel and one half texel in the
 * texture lookup, we need to know the texture image size.
 */
#define ONE 0.00390625
#define ONEHALF 0.001953125
// The numbers above are 1/256 and 0.5/256, change accordingly
// if you change the code to use another perm/grad texture size.


/*
 * The 5th degree smooth interpolation function for Perlin "improved noise".
 */
float fade(const in float t) {
  // return t*t*(3.0-2.0*t); // Old fade, yields discontinuous second derivative
  return t*t*t*(t*(t*6.0-15.0)+10.0); // Improved fade, yields C2-continuous noise
}

/*
 * Efficient simplex indexing functions by Bill Licea-Kane, ATI. Thanks!
 * (This was originally implemented as a texture lookup. Nice to avoid that.)
 */
void simplex( const in vec3 P, out vec3 offset1, out vec3 offset2 )
{
  vec3 offset0;

  vec2 isX = step( P.yz, P.xx );         // P.x >= P.y ? 1.0 : 0.0;  P.x >= P.z ? 1.0 : 0.0;
  offset0.x  = dot( isX, vec2( 1.0 ) );  // Accumulate all P.x >= other channels in offset.x
  offset0.yz = 1.0 - isX;                // Accumulate all P.x <  other channels in offset.yz

  float isY = step( P.z, P.y );          // P.y >= P.z ? 1.0 : 0.0;
  offset0.y += isY;                      // Accumulate P.y >= P.z in offset.y
  offset0.z += 1.0 - isY;                // Accumulate P.y <  P.z in offset.z

  // offset0 now contains the unique values 0,1,2 in each channel
  // 2 for the channel greater than other channels
  // 1 for the channel that is less than one but greater than another
  // 0 for the channel less than other channels
  // Equality ties are broken in favor of first x, then y
  // (z always loses ties)

  offset2 = clamp(   offset0, 0.0, 1.0 );
  // offset2 contains 1 in each channel that was 1 or 2
  offset1 = clamp( --offset0, 0.0, 1.0 );
  // offset1 contains 1 in the single channel that was 1
}

void simplex( const in vec4 P, out vec4 offset1, out vec4 offset2, out vec4 offset3 )
{
  vec4 offset0;

  vec3 isX = step( P.yzw, P.xxx );        // See comments in 3D simplex function
  offset0.x = dot( isX, vec3( 1.0 ) );
  offset0.yzw = 1.0 - isX;

  vec2 isY = step( P.zw, P.yy );
  offset0.y += dot( isY, vec2( 1.0 ) );
  offset0.zw += 1.0 - isY;

  float isZ = step( P.w, P.z );
  offset0.z += isZ;
  offset0.w += 1.0 - isZ;

  // offset0 now contains the unique values 0,1,2,3 in each channel

  offset3 = clamp(   offset0, 0.0, 1.0 );
  offset2 = clamp( --offset0, 0.0, 1.0 );
  offset1 = clamp( --offset0, 0.0, 1.0 );
}


/*
 * 2D classic Perlin noise. Fast, but less useful than 3D noise.
 */
float noise(const in vec2 P)
{
  vec2 Pi = ONE*floor(P)+ONEHALF; // Integer part, scaled and offset for texture lookup
  vec2 Pf = fract(P);             // Fractional part for interpolation

  // Noise contribution from lower left corner
  vec2 grad00 = texture(permTexture, Pi).rg * 4.0 - 1.0;
  float n00 = dot(grad00, Pf);

  // Noise contribution from lower right corner
  vec2 grad10 = texture(permTexture, Pi + vec2(ONE, 0.0)).rg * 4.0 - 1.0;
  float n10 = dot(grad10, Pf - vec2(1.0, 0.0));

  // Noise contribution from upper left corner
  vec2 grad01 = texture(permTexture, Pi + vec2(0.0, ONE)).rg * 4.0 - 1.0;
  float n01 = dot(grad01, Pf - vec2(0.0, 1.0));

  // Noise contribution from upper right corner
  vec2 grad11 = texture(permTexture, Pi + vec2(ONE, ONE)).rg * 4.0 - 1.0;
  float n11 = dot(grad11, Pf - vec2(1.0, 1.0));

  // Blend contributions along x
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade(Pf.x));

  // Blend contributions along y
  float n_xy = mix(n_x.x, n_x.y, fade(Pf.y));

  // We're done, return the final noise value.
  return n_xy;
}


/*
 * 3D classic noise. Slower, but a lot more useful than 2D noise.
 */
float noise(const in vec3 P)
{
  vec3 Pi = ONE*floor(P)+ONEHALF; // Integer part, scaled so +1 moves one texel
                                  // and offset 1/2 texel to sample texel centers
  vec3 Pf = fract(P);     // Fractional part for interpolation

  // Noise contributions from (x=0, y=0), z=0 and z=1
  float perm00 = texture(permTexture, Pi.xy).a ;
  vec3  grad000 = texture(permTexture, vec2(perm00, Pi.z)).rgb * 4.0 - 1.0;
  float n000 = dot(grad000, Pf);
  vec3  grad001 = texture(permTexture, vec2(perm00, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n001 = dot(grad001, Pf - vec3(0.0, 0.0, 1.0));

  // Noise contributions from (x=0, y=1), z=0 and z=1
  float perm01 = texture(permTexture, Pi.xy + vec2(0.0, ONE)).a ;
  vec3  grad010 = texture(permTexture, vec2(perm01, Pi.z)).rgb * 4.0 - 1.0;
  float n010 = dot(grad010, Pf - vec3(0.0, 1.0, 0.0));
  vec3  grad011 = texture(permTexture, vec2(perm01, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n011 = dot(grad011, Pf - vec3(0.0, 1.0, 1.0));

  // Noise contributions from (x=1, y=0), z=0 and z=1
  float perm10 = texture(permTexture, Pi.xy + vec2(ONE, 0.0)).a ;
  vec3  grad100 = texture(permTexture, vec2(perm10, Pi.z)).rgb * 4.0 - 1.0;
  float n100 = dot(grad100, Pf - vec3(1.0, 0.0, 0.0));
  vec3  grad101 = texture(permTexture, vec2(perm10, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n101 = dot(grad101, Pf - vec3(1.0, 0.0, 1.0));

  // Noise contributions from (x=1, y=1), z=0 and z=1
  float perm11 = texture(permTexture, Pi.xy + vec2(ONE, ONE)).a ;
  vec3  grad110 = texture(permTexture, vec2(perm11, Pi.z)).rgb * 4.0 - 1.0;
  float n110 = dot(grad110, Pf - vec3(1.0, 1.0, 0.0));
  vec3  grad111 = texture(permTexture, vec2(perm11, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n111 = dot(grad111, Pf - vec3(1.0, 1.0, 1.0));

  // Blend contributions along x
  vec4 n_x = mix(vec4(n000, n001, n010, n011),
                 vec4(n100, n101, n110, n111), fade(Pf.x));

  // Blend contributions along y
  vec2 n_xy = mix(n_x.xy, n_x.zw, fade(Pf.y));

  // Blend contributions along z
  float n_xyz = mix(n_xy.x, n_xy.y, fade(Pf.z));

  // We're done, return the final noise value.
  return n_xyz;
}


/*
 * 4D classic noise. Slow, but very useful. 4D simplex noise is a lot faster.
 *
 * This function performs 8 texture lookups and 16 dependent texture lookups,
 * 16 dot products, 4 mix operations and a lot of additions and multiplications.
 * Needless to say, it's not super fast. But it's not dead slow either.
 */
float noise(const in vec4 P)
{
  vec4 Pi = ONE*floor(P)+ONEHALF; // Integer part, scaled so +1 moves one texel
                                  // and offset 1/2 texel to sample texel centers
  vec4 Pf = fract(P);      // Fractional part for interpolation

  // "n0000" is the noise contribution from (x=0, y=0, z=0, w=0), and so on
  float perm00xy = texture(permTexture, Pi.xy).a ;
  float perm00zw = texture(permTexture, Pi.zw).a ;
  vec4 grad0000 = texture(gradTexture, vec2(perm00xy, perm00zw)).rgba * 4.0 -1.0;
  float n0000 = dot(grad0000, Pf);

  float perm01zw = texture(permTexture, Pi.zw  + vec2(0.0, ONE)).a ;
  vec4  grad0001 = texture(gradTexture, vec2(perm00xy, perm01zw)).rgba * 4.0 - 1.0;
  float n0001 = dot(grad0001, Pf - vec4(0.0, 0.0, 0.0, 1.0));

  float perm10zw = texture(permTexture, Pi.zw  + vec2(ONE, 0.0)).a ;
  vec4  grad0010 = texture(gradTexture, vec2(perm00xy, perm10zw)).rgba * 4.0 - 1.0;
  float n0010 = dot(grad0010, Pf - vec4(0.0, 0.0, 1.0, 0.0));

  float perm11zw = texture(permTexture, Pi.zw  + vec2(ONE, ONE)).a ;
  vec4  grad0011 = texture(gradTexture, vec2(perm00xy, perm11zw)).rgba * 4.0 - 1.0;
  float n0011 = dot(grad0011, Pf - vec4(0.0, 0.0, 1.0, 1.0));

  float perm01xy = texture(permTexture, Pi.xy + vec2(0.0, ONE)).a ;
  vec4  grad0100 = texture(gradTexture, vec2(perm01xy, perm00zw)).rgba * 4.0 - 1.0;
  float n0100 = dot(grad0100, Pf - vec4(0.0, 1.0, 0.0, 0.0));

  vec4  grad0101 = texture(gradTexture, vec2(perm01xy, perm01zw)).rgba * 4.0 - 1.0;
  float n0101 = dot(grad0101, Pf - vec4(0.0, 1.0, 0.0, 1.0));

  vec4  grad0110 = texture(gradTexture, vec2(perm01xy, perm10zw)).rgba * 4.0 - 1.0;
  float n0110 = dot(grad0110, Pf - vec4(0.0, 1.0, 1.0, 0.0));

  vec4  grad0111 = texture(gradTexture, vec2(perm01xy, perm11zw)).rgba * 4.0 - 1.0;
  float n0111 = dot(grad0111, Pf - vec4(0.0, 1.0, 1.0, 1.0));

  float perm10xy = texture(permTexture, Pi.xy + vec2(ONE, 0.0)).a ;
  vec4  grad1000 = texture(gradTexture, vec2(perm10xy, perm00zw)).rgba * 4.0 - 1.0;
  float n1000 = dot(grad1000, Pf - vec4(1.0, 0.0, 0.0, 0.0));

  vec4  grad1001 = texture(gradTexture, vec2(perm10xy, perm01zw)).rgba * 4.0 - 1.0;
  float n1001 = dot(grad1001, Pf - vec4(1.0, 0.0, 0.0, 1.0));

  vec4  grad1010 = texture(gradTexture, vec2(perm10xy, perm10zw)).rgba * 4.0 - 1.0;
  float n1010 = dot(grad1010, Pf - vec4(1.0, 0.0, 1.0, 0.0));

  vec4  grad1011 = texture(gradTexture, vec2(perm10xy, perm11zw)).rgba * 4.0 - 1.0;
  float n1011 = dot(grad1011, Pf - vec4(1.0, 0.0, 1.0, 1.0));

  float perm11xy = texture(permTexture, Pi.xy + vec2(ONE, ONE)).a ;
  vec4  grad1100 = texture(gradTexture, vec2(perm11xy, perm00zw)).rgba * 4.0 - 1.0;
  float n1100 = dot(grad1100, Pf - vec4(1.0, 1.0, 0.0, 0.0));

  vec4  grad1101 = texture(gradTexture, vec2(perm11xy, perm01zw)).rgba * 4.0 - 1.0;
  float n1101 = dot(grad1101, Pf - vec4(1.0, 1.0, 0.0, 1.0));

  vec4  grad1110 = texture(gradTexture, vec2(perm11xy, perm10zw)).rgba * 4.0 - 1.0;
  float n1110 = dot(grad1110, Pf - vec4(1.0, 1.0, 1.0, 0.0));

  vec4  grad1111 = texture(gradTexture, vec2(perm11xy, perm11zw)).rgba * 4.0 - 1.0;
  float n1111 = dot(grad1111, Pf - vec4(1.0, 1.0, 1.0, 1.0));

  // Blend contributions along x
  float fadex = fade(Pf.x);
  vec4 n_x0 = mix(vec4(n0000, n0001, n0010, n0011),
                  vec4(n1000, n1001, n1010, n1011), fadex);
  vec4 n_x1 = mix(vec4(n0100, n0101, n0110, n0111),
                  vec4(n1100, n1101, n1110, n1111), fadex);

  // Blend contributions along y
  vec4 n_xy = mix(n_x0, n_x1, fade(Pf.y));

  // Blend contributions along z
  vec2 n_xyz = mix(n_xy.xy, n_xy.zw, fade(Pf.z));

  // Blend contributions along w
  float n_xyzw = mix(n_xyz.x, n_xyz.y, fade(Pf.w));

  // We're done, return the final noise value.
  return n_xyzw;
}


/*
 * 2D simplex noise. Somewhat slower but much better looking than classic noise.
 */
float snoise(const in vec2 P) {

// Skew and unskew factors are a bit hairy for 2D, so define them as constants
// This is (sqrt(3.0)-1.0)/2.0
#define F2 0.366025403784
// This is (3.0-sqrt(3.0))/6.0
#define G2 0.211324865405

  // Skew the (x,y) space to determine which cell of 2 simplices we're in
 	float s = (P.x + P.y) * F2;   // Hairy factor for 2D skewing
  vec2 Pi = floor(P + s);
  float t = (Pi.x + Pi.y) * G2; // Hairy factor for unskewing
  vec2 P0 = Pi - t; // Unskew the cell origin back to (x,y) space
  Pi = Pi * ONE + ONEHALF; // Integer part, scaled and offset for texture lookup

  vec2 Pf0 = P - P0;  // The x,y distances from the cell origin

  // For the 2D case, the simplex shape is an equilateral triangle.
  // Find out whether we are above or below the x=y diagonal to
  // determine which of the two triangles we're in.
  vec2 o1;
  if(Pf0.x > Pf0.y) o1 = vec2(1.0, 0.0);  // +x, +y traversal order
  else o1 = vec2(0.0, 1.0);               // +y, +x traversal order

  // Noise contribution from simplex origin
  vec2 grad0 = texture(permTexture, Pi).rg * 4.0 - 1.0;
  float t0 = 0.5 - dot(Pf0, Pf0);
  float n0;
  if (t0 < 0.0) n0 = 0.0;
  else {
    t0 *= t0;
    n0 = t0 * t0 * dot(grad0, Pf0);
  }

  // Noise contribution from middle corner
  vec2 Pf1 = Pf0 - o1 + G2;
  vec2 grad1 = texture(permTexture, Pi + o1*ONE).rg * 4.0 - 1.0;
  float t1 = 0.5 - dot(Pf1, Pf1);
  float n1;
  if (t1 < 0.0) n1 = 0.0;
  else {
    t1 *= t1;
    n1 = t1 * t1 * dot(grad1, Pf1);
  }

  // Noise contribution from last corner
  vec2 Pf2 = Pf0 - vec2(1.0-2.0*G2);
  vec2 grad2 = texture(permTexture, Pi + vec2(ONE, ONE)).rg * 4.0 - 1.0;
  float t2 = 0.5 - dot(Pf2, Pf2);
  float n2;
  if(t2 < 0.0) n2 = 0.0;
  else {
    t2 *= t2;
    n2 = t2 * t2 * dot(grad2, Pf2);
  }

  // Sum up and scale the result to cover the range [-1,1]
  return 70.0 * (n0 + n1 + n2);
}


/*
 * 3D simplex noise. Comparable in speed to classic noise, better looking.
 */
float snoise(const in vec3 P) {

// The skewing and unskewing factors are much simpler for the 3D case
#define F3 0.333333333333
#define G3 0.166666666667

  // Skew the (x,y,z) space to determine which cell of 6 simplices we're in
 	float s = (P.x + P.y + P.z) * F3; // Factor for 3D skewing
  vec3 Pi = floor(P + s);
  float t = (Pi.x + Pi.y + Pi.z) * G3;
  vec3 P0 = Pi - t; // Unskew the cell origin back to (x,y,z) space
  Pi = Pi * ONE + ONEHALF; // Integer part, scaled and offset for texture lookup

  vec3 Pf0 = P - P0;  // The x,y distances from the cell origin

  // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
  // To find out which of the six possible tetrahedra we're in, we need to
  // determine the magnitude ordering of x, y and z components of Pf0.
  vec3 o1;
  vec3 o2;
  simplex(Pf0, o1, o2);

  // Noise contribution from simplex origin
  float perm0 = texture(permTexture, Pi.xy).a;
  vec3  grad0 = texture(permTexture, vec2(perm0, Pi.z)).rgb * 4.0 - 1.0;
  float t0 = 0.6 - dot(Pf0, Pf0);
  float n0;
  if (t0 < 0.0) n0 = 0.0;
  else {
    t0 *= t0;
    n0 = t0 * t0 * dot(grad0, Pf0);
  }

  // Noise contribution from second corner
  vec3 Pf1 = Pf0 - o1 + G3;
  float perm1 = texture(permTexture, Pi.xy + o1.xy*ONE).a;
  vec3  grad1 = texture(permTexture, vec2(perm1, Pi.z + o1.z*ONE)).rgb * 4.0 - 1.0;
  float t1 = 0.6 - dot(Pf1, Pf1);
  float n1;
  if (t1 < 0.0) n1 = 0.0;
  else {
    t1 *= t1;
    n1 = t1 * t1 * dot(grad1, Pf1);
  }

  // Noise contribution from third corner
  vec3 Pf2 = Pf0 - o2 + 2.0 * G3;
  float perm2 = texture(permTexture, Pi.xy + o2.xy*ONE).a;
  vec3  grad2 = texture(permTexture, vec2(perm2, Pi.z + o2.z*ONE)).rgb * 4.0 - 1.0;
  float t2 = 0.6 - dot(Pf2, Pf2);
  float n2;
  if (t2 < 0.0) n2 = 0.0;
  else {
    t2 *= t2;
    n2 = t2 * t2 * dot(grad2, Pf2);
  }

  // Noise contribution from last corner
  vec3 Pf3 = Pf0 - vec3(1.0-3.0*G3);
  float perm3 = texture(permTexture, Pi.xy + vec2(ONE, ONE)).a;
  vec3  grad3 = texture(permTexture, vec2(perm3, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float t3 = 0.6 - dot(Pf3, Pf3);
  float n3;
  if(t3 < 0.0) n3 = 0.0;
  else {
    t3 *= t3;
    n3 = t3 * t3 * dot(grad3, Pf3);
  }

  // Sum up and scale the result to cover the range [-1,1]
  return 32.0 * (n0 + n1 + n2 + n3);
}


/*
 * 4D simplex noise. A lot faster than classic 4D noise, and better looking.
 */

float snoise(const in vec4 P) {

// The skewing and unskewing factors are hairy again for the 4D case
// This is (sqrt(5.0)-1.0)/4.0
#define F4 0.309016994375
// This is (5.0-sqrt(5.0))/20.0
#define G4 0.138196601125

  // Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
 	float s = (P.x + P.y + P.z + P.w) * F4; // Factor for 4D skewing
  vec4 Pi = floor(P + s);
  float t = (Pi.x + Pi.y + Pi.z + Pi.w) * G4;
  vec4 P0 = Pi - t; // Unskew the cell origin back to (x,y,z,w) space
  Pi = Pi * ONE + ONEHALF; // Integer part, scaled and offset for texture lookup

  vec4 Pf0 = P - P0;  // The x,y distances from the cell origin

  // For the 4D case, the simplex is a 4D shape I won't even try to describe.
  // To find out which of the 24 possible simplices we're in, we need to
  // determine the magnitude ordering of x, y, z and w components of Pf0.
  vec4 o1;
  vec4 o2;
  vec4 o3;
  simplex(Pf0, o1, o2, o3);

  // Noise contribution from simplex origin
  float perm0xy = texture(permTexture, Pi.xy).a;
  float perm0zw = texture(permTexture, Pi.zw).a;
  vec4  grad0 = texture(gradTexture, vec2(perm0xy, perm0zw)).rgba * 4.0 - 1.0;
  float t0 = 0.6 - dot(Pf0, Pf0);
  float n0;
  if (t0 < 0.0) n0 = 0.0;
  else {
    t0 *= t0;
    n0 = t0 * t0 * dot(grad0, Pf0);
  }

  // Noise contribution from second corner
  vec4 Pf1 = Pf0 - o1 + G4;
  o1 = o1 * ONE;
  float perm1xy = texture(permTexture, Pi.xy + o1.xy).a;
  float perm1zw = texture(permTexture, Pi.zw + o1.zw).a;
  vec4  grad1 = texture(gradTexture, vec2(perm1xy, perm1zw)).rgba * 4.0 - 1.0;
  float t1 = 0.6 - dot(Pf1, Pf1);
  float n1;
  if (t1 < 0.0) n1 = 0.0;
  else {
    t1 *= t1;
    n1 = t1 * t1 * dot(grad1, Pf1);
  }

  // Noise contribution from third corner
  vec4 Pf2 = Pf0 - o2 + 2.0 * G4;
  o2 = o2 * ONE;
  float perm2xy = texture(permTexture, Pi.xy + o2.xy).a;
  float perm2zw = texture(permTexture, Pi.zw + o2.zw).a;
  vec4  grad2 = texture(gradTexture, vec2(perm2xy, perm2zw)).rgba * 4.0 - 1.0;
  float t2 = 0.6 - dot(Pf2, Pf2);
  float n2;
  if (t2 < 0.0) n2 = 0.0;
  else {
    t2 *= t2;
    n2 = t2 * t2 * dot(grad2, Pf2);
  }

  // Noise contribution from fourth corner
  vec4 Pf3 = Pf0 - o3 + 3.0 * G4;
  o3 = o3 * ONE;
  float perm3xy = texture(permTexture, Pi.xy + o3.xy).a;
  float perm3zw = texture(permTexture, Pi.zw + o3.zw).a;
  vec4  grad3 = texture(gradTexture, vec2(perm3xy, perm3zw)).rgba * 4.0 - 1.0;
  float t3 = 0.6 - dot(Pf3, Pf3);
  float n3;
  if (t3 < 0.0) n3 = 0.0;
  else {
    t3 *= t3;
    n3 = t3 * t3 * dot(grad3, Pf3);
  }

  // Noise contribution from last corner
  vec4 Pf4 = Pf0 - vec4(1.0-4.0*G4);
  float perm4xy = texture(permTexture, Pi.xy + vec2(ONE, ONE)).a;
  float perm4zw = texture(permTexture, Pi.zw + vec2(ONE, ONE)).a;
  vec4  grad4 = texture(gradTexture, vec2(perm4xy, perm4zw)).rgba * 4.0 - 1.0;
  float t4 = 0.6 - dot(Pf4, Pf4);
  float n4;
  if(t4 < 0.0) n4 = 0.0;
  else {
    t4 *= t4;
    n4 = t4 * t4 * dot(grad4, Pf4);
  }

  // Sum up and scale the result to cover the range [-1,1]
  return 27.0 * (n0 + n1 + n2 + n3 + n4);
}

float fbm(vec3 position, int octaves, float frequency, float persistence, float time) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		total += snoise(vec4(position * frequency, time)) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
	}
	return total / maxAmplitude;
}

float absNoise(vec3 position, int octaves, float frequency, float persistence, float time) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		total += abs(snoise(vec4(position * frequency, time))) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
	}
	return total / maxAmplitude;
}

float ridgedNoise(vec3 position, int octaves, float frequency, float persistence, float time) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		total += ((1.0 - abs(snoise(vec4(position * frequency, time)))) * 2.0 - 1.0) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
	}
	return total / maxAmplitude;
}

float squaredNoise(vec3 position, int octaves, float frequency, float persistence, float time) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		float tmp = snoise(vec4(position * frequency, time));
		total += tmp * tmp * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
	}
	return total / maxAmplitude;
}

float cubedNoise(vec3 position, int octaves, float frequency, float persistence, float time) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		float tmp = snoise(vec4(position * frequency, time));
		total += tmp * tmp * tmp * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
	}
	return total / maxAmplitude;
}

//void main( void )
//{
//
  /* These lines test, in order, 2D classic noise, 2D simplex noise,
   * 3D classic noise, 3D simplex noise, 4D classic noise, and finally
   * 4D simplex noise.
   * Everything but the 4D simplex noise will make some uniform
   * variables remain unused and be optimized away by the compiler,
   * so OpenGL will fail to bind them. It's safe to ignore these
   * warnings from the C program. The program is designed to work anyway.
   */
  //float n = noise(v_texCoord2D * 32.0 + 240.0);
  //float n = noise(vec3(4.0 * v_texCoord3D.xyz * (2.0 + sin(0.5 * time))));
  //float n = snoise(vec3(2.0 * v_texCoord3D.xyz * (2.0 + sin(0.5 * time))));
  //float n = noise(vec4(8.0 * v_texCoord3D.xyz, 0.5 * time));
  //float n = snoise(vec4(4.0 * v_texCoord3D.xyz, 0.5 * time));
  //float n = fbm(v_texCoord3D.xyz, 8, 8.0, 0.5, time);
//
  //gl_FragColor = v_color * vec4(0.5 + 0.5 * vec3(n, n, n), 1.0);
//
//}
