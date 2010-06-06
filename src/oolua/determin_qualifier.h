///////////////////////////////////////////////////////////////////////////////
///  @file determin_qualifier.h
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  @licence 
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#ifndef DETERMIN_QUALIFIER_H_
#	define DETERMIN_QUALIFIER_H_

#include "lvd_types.h"
#include "type_list.h"

namespace LVD
{
    struct is_true{};
	struct is_false{};
	
//	template< bool B, typename Then = void, typename Else = void >
//	struct if_else 
//	{
//		typedef Then type;
//	};
//
//	template< typename Then, typename Else >
//	struct if_else< false, Then, Else > 
//	{
//		typedef Else type;
//	};
	
	template<typename T,typename T1>
	struct is_same
	{
	    enum {value = 0};
    };
	template<typename T>
	struct is_same<T,T>
	{
	    enum {value = 1};	 	   
 	};
 	


template<typename T>struct is_const
{
    enum {value = 0};
};
template<typename T>struct is_const<T const>
{
    enum {value = 1};
};
template<typename T>struct is_const<T const&>
{
    enum {value = 1};
};
template<typename T>struct is_const<T const*>
{
    enum {value = 1};
};
template<typename T>struct is_const<T * const>
{
	enum {value = 0};
};
template<typename T>struct is_const<T const * const>
{
	enum {value = 1};
};
template<typename T>struct is_const<T const * const&>
{
	enum {value = 1};
};
template<typename T>struct is_const<T * const&>
{
	enum {value = 0};
};
template<typename T>struct is_const<T const *&>
{
	enum {value = 1};
};
template<typename T>struct is_const<T * const *&>
{
	enum {value = 0};
};
template<typename T>struct is_const<T const * const *&>
{
	enum {value = 1};
};

template<typename T>struct is_void
{
    enum {value = 0};
};
template<>struct is_void<void>
{
    enum {value = 1};
};

template<typename T>struct is_ref
{
    enum {value = 0};
};
template<typename T>struct is_ref<T&>
{
    enum {value = 1};
};

template<typename T>struct is_ref_to_const
{
    enum {value = 0};
};
template<typename T>struct is_ref_to_const<T const&>
{
    enum {value = 1};
};

template<typename T>struct is_ref_to_pointer
{
    enum {value = 0};
};

template<typename T>struct is_ref_to_pointer<T*&>
{
    enum {value = 1};
};

template<typename T>struct is_ptr
{
    enum {value = 0};
};

template<typename T>struct is_ptr<T*>
{
    enum {value = 1};
};

template<typename T>struct is_ptr_to_const
{
    enum {value = 0};
};

template<typename T>struct is_ptr_to_const<T const*>
{
    enum {value = 1};
};
template<typename T>struct is_const_ptr_to
{
    enum {value = 0};
};

template<typename T>struct is_const_ptr_to<T *const>
{
    enum {value = 1};
};

template<typename T>struct is_const_ptr_to_const
{
    enum {value = 0};
};

template<typename T>struct is_const_ptr_to_const<T const * const>
{
    enum {value = 1};
};

template<typename T>struct has_qualifiers
{
    enum { value = (
		   is_ref<T>::value 
    	   + is_ref_to_const<T>::value 
    	   + is_ref_to_pointer<T>::value
    	   + is_ptr<T>::value
    	   + is_ptr_to_const<T>::value
    	   + is_const_ptr_to<T>::value
    	   + is_const_ptr_to_const<T>::value
    	   + is_const<T>::value 
		   ) ? 1 : 0 };
	   
};

template<typename T>struct by_reference
{
	enum { value = (
		is_ref<T>::value 
		+ is_ref_to_const<T>::value 
		+ is_ref_to_pointer<T>::value
		+ is_ptr<T>::value
		+ is_ptr_to_const<T>::value
		+ is_const_ptr_to<T>::value
		+ is_const_ptr_to_const<T>::value
		) ? 1 : 0 };

};

template<typename T>struct has_qualifiers1
{
    enum { value = (
		   if_or<is_ref<T>::value, 
    	   if_or<is_ref_to_const<T>::value ,
    	   if_or<is_ref_to_pointer<T>::value,
    	   if_or<is_ptr<T>::value,
    	   if_or<is_ptr_to_const<T>::value,
    	   if_or<is_const_ptr_to<T>::value,
    	   if_or<is_const_ptr_to_const<T>::value,
    	   is_const<T>::value 
		   >::value >::value >::value >::value 
		   >::value >::value >::value
		   ) ? 1 : 0 };
	   
};
template<typename T>struct constant_remover
{
     typedef T  type;
};
template<typename T>struct constant_remover<T const>
{
     typedef T  type;
};
template<typename T>struct constant_remover<T const&>
{
     typedef T&  type;
};
template<typename T>struct constant_remover<T const*>
{
     typedef T*  type;
};
template<typename T>struct constant_remover<T const * const>
{
	typedef T* const  type;
};
template<typename T>struct constant_remover<T* const>
{
	typedef T* const  type;
};

template<typename T>struct remove_const
{
     typedef typename constant_remover<T>::type type;
};

/////////////////////////////////////////////////////////////////////////////////////


template<typename T>struct all_constant_remover
{
	typedef T  type;
};
template<typename T>struct all_constant_remover<T const>
{
	typedef T  type;
};
template<typename T>struct all_constant_remover<T const&>
{
	typedef T&  type;
};
template<typename T>struct all_constant_remover<T const*>
{
	typedef T*  type;
};

template<typename T>struct all_constant_remover<T const * const>
{
	typedef T*  type;
};
template<typename T>struct all_constant_remover<T* const>
{
	typedef T*  type;
};

template<typename T>struct remove_all_const
{
	typedef typename all_constant_remover<T>::type type;
};



template<typename T>struct remove_ref
{
   typedef T type;  
};
template<typename T>struct remove_ref<T&>
{
   typedef T type;  
};

template<typename T>struct ptr_remover
{
   typedef T type;  
};
template<typename T>struct ptr_remover<T*>
{
   typedef T type;  
};


template<typename T>struct remove_ptr
{
     typedef typename ptr_remover<T>::type type;
};

namespace
{
 		 template<typename Input,template <typename> class Do,
				  template <typename> class Test,int result>
		  struct do_while;


		  template<typename Input,template <typename> class Do,
				  template <typename> class Test,int result>
		  struct do_while
		  {
   		   		 typedef typename do_while<typename Do<Input>::type,
	  	   		 		 Do,Test,
			 	 		 Test<typename Do<Input>::type>::value >::type type;  
		  };

		  template<typename Input,template <typename>class Do,
				  template <typename> class Test >
		  struct do_while<Input,Do,Test,0>
		  {
   		   		 typedef Input type;  
          };

}

template<typename T>struct remove_all_ptrs
{
						/// input,   do,     test,     result
     typedef typename do_while<T,remove_ptr,is_ptr,is_ptr<T>::value >::type type;
};
}//endof namespace LVD
#endif //DETERMIN_QUALIFIER_H_
