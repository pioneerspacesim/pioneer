/* note(jpab): This class was pulled out of the pcg_extras.hpp header, because
 *             its use of the __DATE__ and __TIME__ macros disables ccache.
 *
 * PCG Random Number Generation for C++
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *     http://www.pcg-random.org
 */

#ifndef PCG_STATIC_ARBITRARY_SEED_HPP_INCLUDED
#define PCG_STATIC_ARBITRARY_SEED_HPP_INCLUDED 1

/*
 * Sometimes you might want a distinct seed based on when the program
 * was compiled.  That way, a particular instance of the program will
 * behave the same way, but when recompiled it'll produce a different
 * value.
 */

template <typename IntType>
struct static_arbitrary_seed {
private:
    static constexpr IntType fnv(IntType hash, const char* pos) {
        return *pos == '\0'
             ? hash
             : fnv((hash * IntType(16777619U)) ^ *pos, (pos+1));
    }

public:
    static constexpr IntType value = fnv(IntType(2166136261U ^ sizeof(IntType)),
                        __DATE__ __TIME__ __FILE__);
};

} // namespace pcg_extras

#endif // PCG_STATIC_ARBITRARY_SEED_HPP_INCLUDED
