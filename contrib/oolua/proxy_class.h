///////////////////////////////////////////////////////////////////////////////
///  @file proxy_class.h
///  Defines the class, its bases in the hierarchical tree. The classes name
///  an array used to hold the functions its make available to the script and
///  C++ special member functions
///
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////

#ifndef CPP_PROXY_CLASS
#	define CPP_PROXY_CLASS

#include "type_list.h"

namespace OOLUA
{
	///////////////////////////////////////////////////////////////////////////////
	///  @class OOLUA::Proxy_class
	///  A template wrapper for class objects of type T used by the script binding.
	///  @remarks \see Proxy_class_macros for the macros which are used to define
	///  a proxy class.
	///////////////////////////////////////////////////////////////////////////////
	template<typename T>class Proxy_class;

	namespace INTERNAL
	{
		template<typename TL,typename T>
		struct AddBase;

		template<typename TL,typename T>
		struct AddBase
		{
			typedef typename TYPELIST::Append<TL,T>::Result Result;
		};

		template<typename TL>
		struct AddBase<TL,TYPE::Null_type>
		{
			typedef TL Result;
		};

		template<int Index,typename AddToTL,typename BasesTL,typename BaseType>
		struct ForEachBaseAddAllBases
		{
			typedef typename AddBase<AddToTL,typename OOLUA::Proxy_class<BaseType>::AllBases>::Result InternalResult;
			typedef typename ForEachBaseAddAllBases<
								Index+1
								,InternalResult//adding to this type list
								,BasesTL//each of the entries in this types list
									,typename TYPELIST::At_default<BasesTL
									,Index+1
									,TYPE::Null_type>::Result//next type in the base type list
				>::Result Result;
		};

		template<int Index,typename AddToTL,typename BasesTL>
		struct ForEachBaseAddAllBases<Index,AddToTL,BasesTL,TYPE::Null_type>
		{
			typedef AddToTL Result;
		};

		template<typename T>
		struct FindAllBases
		{
			typedef typename ForEachBaseAddAllBases<
								0
								,typename OOLUA::Proxy_class<T>::Bases//adding to this type list
								,typename OOLUA::Proxy_class<T>::Bases//each of the entry in this types list
									,typename TYPELIST::At_default<typename OOLUA::Proxy_class<T>::Bases
									,0
									,TYPE::Null_type>::Result//starting with this type
				>::Result Result;
		};

		template<typename AddTo,typename BaseType,typename NullIfRoot>
		struct AddIfRoot
		{
			typedef AddTo Result;
		};

		template<typename AddTo,typename BaseType>
		struct AddIfRoot<AddTo,BaseType,TYPE::Null_type>
		{
			typedef typename AddBase<AddTo,BaseType>::Result Result;
		};

		template<int Index,typename AddTo, typename AllTypesBases, typename T>
		struct ForEachOfAllBasesAddRoots
		{
			typedef typename AddIfRoot<AddTo,T,typename OOLUA::Proxy_class<T>::Bases>::Result InternalResult;
			typedef typename ForEachOfAllBasesAddRoots<
								Index+1//look at this index
								,InternalResult//add to this type list
								,AllTypesBases//looking at each entry in this list
									,typename TYPELIST::At_default<AllTypesBases
									,Index+1
									,TYPE::Null_type>::Result//next entry to check
				>::Result Result;
		};

		template<int Index,typename AddTo, typename AllTypesBases>
		struct ForEachOfAllBasesAddRoots<Index,AddTo,AllTypesBases,TYPE::Null_type>
		{
			typedef AddTo Result;
		};

		template<typename T>
		struct FindRootBases
		{
			typedef Type_list<>::type DummyTL;
			typedef typename ForEachOfAllBasesAddRoots<
								0//starting at the beginning
								,DummyTL//add to this type list
								,typename OOLUA::Proxy_class<T>::AllBases//looking at each entry in this list
									,typename TYPELIST::At_default<typename OOLUA::Proxy_class<T>::AllBases
									,0
									,TYPE::Null_type>::Result//starting with this entry
				>::Result Result;
		};

		/////////////////////////////////////////////////////////////////////////////////
		/////  @struct OOLUA::Set_this_ptr
		/////  For each base set the this pointer
		/////////////////////////////////////////////////////////////////////////////////
		template<typename T,typename TypeList, int TypeIndex,typename BaseType>
		struct Set_this_ptr
		{
			void operator()(T* proxy_this, typename T::class_* this_)
			{
				//base is valid add the this pointer to the base
				OOLUA::Proxy_class<BaseType>* base(proxy_this);
				//and set the m_this pointer
				base->m_this = this_;
				//move to the next class in the base list
				Set_this_ptr<
						T
						,TypeList
						,TypeIndex + 1,typename TYPELIST::At_default< TypeList, TypeIndex + 1, TYPE::Null_type >::Result
					> nextBase;
				nextBase(proxy_this,this_);
			}
		};
		template<typename T,typename TL, int TypeIndex>
		struct  Set_this_ptr<T,TL,TypeIndex, TYPE::Null_type>
		{
			//no valid base class at the index
			void operator()(T* /*proxy_this*/,typename T::class_* /*this_*/)const{}//no op
		};

		template<typename T>struct Proxy_type;

		template<typename T>
		struct Proxy_type<OOLUA::Proxy_class<T> >
		{
			typedef T Type;
		};
	}
}


///  \addtogroup Proxy_class_macros
///  @{
///  \def OOLUA_FORWARD_DECLARE
///  forward declare in the OOLUA namespace
///	 @note
///	 This must be called at the global namespace and not be nested.
#define OOLUA_FORWARD_DECLARE(name)\
namespace OOLUA\
{\
class OOLUA::Proxy_class<name>;\
}

namespace OOLUA
{
	template<typename NoneProxyType>
	class Proxy_class
	{ 
		typedef NoneProxyType OoluaNoneProxy;
	};
}
/// \def OOLUA_CLASS_END
///end the class and the namespace
#define OOLUA_CLASS_END };}


///	 \def OOLUA_CLASS
///	Class which may have base classes
#define OOLUA_CLASS(name___)\
	namespace OOLUA\
	{\
	template<>\
	class Proxy_class<name___>


///  \def  OOLUA_BASIC
///  C++ special member functions and lua binding details
#define OOLUA_BASIC \
{\
public:\
	typedef INTERNAL::Proxy_type<Proxy_class>::Type class_;\
	typedef Proxy_class<class_> this_type;\
	typedef int (Proxy_class::*mfp)(lua_State *  const  ); \
	typedef int (Proxy_class::*mfp_const)(lua_State *  const  )const; \
	struct Reg_type{ const char *name; mfp mfunc; }; \
	struct Reg_type_const{ const char *name; mfp_const mfunc; }; \
	static char const class_name[]; \
	static char const class_name_const[]; \
	static int const name_size; \
	static Reg_type class_methods[]; \
	static Reg_type_const class_methods_const[]; \
	class_ * m_this;\
	Proxy_class():m_this(0){}\
	Proxy_class(class_* p):m_this(p)\
	{\
		set_this_ptr();\
	}\
	Proxy_class& operator = (Proxy_class const & );\
	Proxy_class(Proxy_class const &);\
	virtual ~Proxy_class(){}\
private:\
	void set_this_ptr()\
	{\
		INTERNAL::Set_this_ptr< \
							this_type \
							,AllBases \
							,0 \
							,TYPELIST::At_default< AllBases, 0, TYPE::Null_type >::Result \
			> ptr;\
		ptr(this,m_this);\
	}\
public:

#define OOLUA_ALLBASES\
	typedef INTERNAL::FindAllBases<class_>::Result AllBases;

///  \def OOLUA_NO_BASES
///  define the class to have no base classes
#define OOLUA_NO_BASES\
	typedef TYPE::Null_type Bases;\
	OOLUA_ALLBASES

///  \def OOLUA_BASES_START
///  define the class to have numerical amount (equal to NUM) of base classes
#define OOLUA_BASES_START\
	OOLUA_TYPEDEFS

///  \def OOLUA_BASES_END
///  end of base class declaring
#define OOLUA_BASES_END\
	>::type Bases;\
	OOLUA_ALLBASES


///	\def  OOLUA_CLASS_NO_BASES
///	uses OOLUA_CLASS, OOLUA_BASIC and OOLUA_NO_BASES
/// Defines a Proxy_class which has no base which you want to
/// inform the script binding about.
/// \note The real class may have base classes.
#define OOLUA_CLASS_NO_BASES(name___)\
	OOLUA_CLASS(name___)\
	OOLUA_BASIC\
	OOLUA_NO_BASES


///	\def  OOLUA_NO_TYPEDEFS
///	the classes has no typedefs
///	\see oolua_typedefs.h
#define OOLUA_NO_TYPEDEFS\
	typedef TYPE::Null_type Typedef;


///	\def  OOLUA_TYPEDEFS
///	Starts the typedef list which has NUM of types
///	\see oolua_typedefs.h
#define OOLUA_TYPEDEFS\
	typedef Type_list<

///	\def  OOLUA_END_TYPES
///	Closes the typedef list
///	\see oolua_typedefs.h
#define OOLUA_END_TYPES\
	>::type Typedef;
///  @}


#endif //CPP_PROXY_CLASS_H_
