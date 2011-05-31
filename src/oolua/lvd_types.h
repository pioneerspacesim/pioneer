///////////////////////////////////////////////////////////////////////////////
///  @file lvd_types.h
///  Header for the types used.
///
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////

#ifndef BASIC_TYPES_H_
#	define BASIC_TYPES_H_

#	include "platform_check.h"
#	include "type_list.h"

#	if ( defined (WINDOWS_BUILD) )
#		ifdef __GNUC__
    #		include <stdint.h>
namespace LVD
{
            typedef int8_t				int8;
            typedef uint8_t				uint8;
            typedef int16_t				int16;
            typedef uint16_t			uint16;
            typedef int32_t				int32;
            typedef uint32_t			uint32;
//#       else
//            typedef signed	__int8		int8;
//            typedef unsigned __int8		uint8;
//            typedef signed	__int16		int16;
//            typedef unsigned __int16	uint16;
//            typedef signed	__int32		int32;
//            typedef unsigned __int32	uint32;
//#       endif
//#		ifdef __GNUC__
			//gcc does define these types yet you need to use the
			// -Wno-long-long flag
			__extension__ typedef long long             int64;
			__extension__ typedef unsigned long long    uint64;
}
#		elif defined _MSC_VER
namespace LVD
{
            typedef signed	__int8		int8;
            typedef unsigned __int8		uint8;
            typedef signed	__int16		int16;
            typedef unsigned __int16	uint16;
            typedef signed	__int32		int32;
            typedef unsigned __int32	uint32;
			typedef	signed	__int64		int64;
			typedef unsigned __int64	uint64;
}
#		else
#			error please inform me what compiler are you using and set the correct types
#		endif
#	elif (defined(UNIX_BUILD) || defined(MAC_BUILD))
#		include <stdint.h>
namespace LVD
{
		typedef int8_t				int8;
		typedef uint8_t				uint8;
		typedef int16_t				int16;
		typedef uint16_t			uint16;
		typedef int32_t				int32;
		typedef uint32_t			uint32;
#		ifdef __CYGWIN__
			//gcc does define these types yet you need to use the
			// the extension keyword to prevent warnings when using a high warning level
			//The -Wno-long-long flag could supress this but lets not interfere
			//with what settings a user wants.
			__extension__ typedef	long long			int64;
			__extension__ typedef unsigned long long	uint64;
#		else
			typedef int64_t				int64;
			typedef uint64_t			uint64;
#		endif
}

#	endif

namespace LVD
{
	//typedef uint8 byte;
	typedef unsigned char byte;
	typedef uint32 size_t;

	template<typename T>
 	struct is_integral_type
 	{
        typedef Type_list<
        		bool,
				char,unsigned char, signed char,
				short,unsigned short, signed short,
				int,unsigned int, signed int,
				long, unsigned long, signed long, int64, uint64,
				float,
				double, long double>::type Intergal;
		enum {value = TYPELIST::IndexOf<Intergal,T>::value == -1 ? 0 : 1};
	};


	template< bool Test>
	struct if_
	{
		enum {RESULT = 1};
	};

	template< >
	struct if_<false>
	{
		enum {RESULT = 0};
	};

	template<int T,int T1>
	struct if_or
	{
	 	   enum {value =1};
 	};
	template<>
	struct if_or<0,0>
	{
	 	   enum {value =0};
 	};

	template< bool B, typename Then = void, typename Else = void >
	struct if_else
	{
		typedef Then type;
	};

	template< typename Then, typename Else >
	struct if_else< false, Then, Else >
	{
		typedef Else type;
	};

	struct Pointer_size
	{
		enum Size{size_value = sizeof(void*)};
		typedef  if_else< sizeof( uint8 ) == size_value, uint8,
				 if_else< sizeof( uint16 ) == size_value, uint16,
			 	 if_else< sizeof( uint32 ) == size_value, uint32
#if defined LVD_WINDOWS_BUILD && defined __GNUC__
				  ,
			 	 if_else< sizeof( uint64 ) == size_value, uint64>::type
#endif
                 >::type >::type >::type type;
	};
//
//	///unsigned int type for pointer manipulation
//	//it is here because it is not part of the C++ standard :(
//	typedef Pointer_size::type uintptr_t;
	typedef Pointer_size::type uintptr_t_lvd;

	///  Int2type
	///  Converts a int to a constant.
	template<int T>
	struct Int2type
	{
		enum {value = T};
	};

	///  Type2type
	///  typedefs a type.
	///  @remarks Useful for compile time choices.
	template<typename T>
	struct Type2type
	{
		typedef T type;
	};

	///  Type2bytes
	///  Convert a integral type into an array of bytes.
	template<typename T>
	struct Type2bytes
	{
		Type2bytes(T const &value):whole(value){}
		Type2bytes():whole(T()/*0*/){}
		union
		{
			T whole;
			byte bytes[sizeof(T)];
		};
	};

}//end of TYPE


#endif//BASIC_TYPES_H_
