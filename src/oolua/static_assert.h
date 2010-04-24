#ifndef LVD_STATIC_ASSERT_H_
#define LVD_ASSERT_H_

#define STATIC_ASSERT(condition) \
        extern char dummy_assert_array[(condition)?1:-1] 

template<bool T>struct Static_assert;

template<>
struct Static_assert<true>{};

#define LVD_STATIC_ASSERT(X)\
	Static_assert<(X) != 0>()



#endif//LVD_STATIC_ASSERT_H_
