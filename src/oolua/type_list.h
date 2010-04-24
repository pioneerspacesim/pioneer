////////////////////////////////////////////////////////////////////////////////
/// @file type_list.h
/// 
/// @licence
/// The Loki Library
/// Copyright (c) 2001 by Andrei Alexandrescu
/// This code accompanies the book:
/// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
/// Patterns Applied". Copyright (c) 2001. Addison-Wesley.
/// Permission to use, copy, modify, distribute and sell this software for any 
/// purpose is hereby granted without fee, provided that the above copyright 
/// notice appear in all copies and that both that copyright notice and this 
/// permission notice appear in supporting documentation.
/// The author or Addison-Wesley Longman make no representations about the 
/// suitability of this software for any purpose. It is provided "as is" 
/// without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////


#ifndef TYPE_LIST_H_
#	define TYPE_LIST_H_

#include "typelist_structs.h"


namespace TYPELIST
{
	///typelist algorithms

	///length////////////////////////////////////////////////////////////
	template <typename Type_list> struct Length;//forward declare
	
	///end of list stop recursive search
	template <> struct Length<TYPE::Null_type>
	{
		enum { value = 0 };
	};

	///recursively search the list counting until we reach null 
	template <typename This, typename Next>
	struct Length< Type_node<This, Next> >//node in the list
	{
		enum { value = 1 + Length<Next>::value };
	};
	///length////////////////////////////////////////////////////////////


	///at////////////////////////////////////////////////////////////////
	///forward declare
	template <typename Type_list, unsigned int index> struct At;

	///specialise the index zero
	///when called the This type is the type of interest
	template <typename This, typename Next>
	struct At<Type_node<This, Next>, 0>
	{
		typedef This Result;
	};
	///recursive at finds the type at the index or throws a compile error
	///because it has gone out of bounds
	template <typename This, typename Next, unsigned int i>
	struct At<Type_node<This, Next>, i>
	{
		typedef typename At<Next, i - 1>::Result Result;
	};
	///at////////////////////////////////////////////////////////////////

	///forgiving at//////////////////////////////////////////////////////
	///at returns a compile time error when out of bounds where as this
	///just set the type to the default if out of bounds
	
	//end of list, so prevent out of bounds and use default
	template <typename Type_list, unsigned int index,typename Default = TYPE::Null_type>
	struct At_default
	{
		typedef Default Result;
	};

	///at the index of interest typedef the result
	template <typename This, typename Next, typename Default>
	struct At_default<Type_node<This, Next>, 0, Default>
	{
		typedef This Result;
	};

	///recursivly search the tree until we get the index of interest,
	//this maybe out of bounds and the default will be stored
	template <typename This, typename Next, unsigned int i, typename Default>
	struct At_default<Type_node<This, Next>, i, Default>
	{
		typedef typename At_default<Next, i - 1, Default>::Result Result;
	};
	///forgiving at//////////////////////////////////////////////////////

	///index/////////////////////////////////////////////////////////////

	///first of search.???

	template <typename Type_list, typename Type> struct IndexOf;

	///not found in list
	template <typename Type>
	struct IndexOf<TYPE::Null_type, Type>
	{
		enum { value = -1 };
	};

	///This is the type we are looking for
	template <typename T, typename Next>
	struct IndexOf<Type_node<T, Next>, T>
	{
		enum { value = 0 };
	};

	template <typename This, typename Next, typename Type>
	struct IndexOf<Type_node<This, Next>, Type>
	{
	private:
		enum { temp = IndexOf<Next, Type>::value };
	public:
		enum { value = temp == -1 ? -1 : 1 + temp };
	};

	///index/////////////////////////////////////////////////////////////



	///append////////////////////////////////////////////////////////////
	template <typename Type_list, typename T> struct Append;

	///list is empty and type is empty
	template <> struct Append<TYPE::Null_type, TYPE::Null_type>
	{
		typedef TYPE::Null_type Result;
	};

	///if the list is empty then this is its only entry
	template <typename T> struct Append<TYPE::Null_type, T>
	{
		//typedef typename type1<T>::type Result;
		typedef Type_node<T, TYPE::Null_type> Result;
	};

	///tree is empty but what we are appending to it is a node
	template <typename This, typename Next>
	struct Append<TYPE::Null_type, Type_node<This, Next> >
	{
		typedef Type_node<This, Next> Result;
	};


	template <typename Head, typename Tail, typename T>
	struct Append<Type_node<Head, Tail>, T>
	{
		typedef Type_node<Head, typename Append<Tail, T>::Result> Result;
	};

	///append////////////////////////////////////////////////////////////


	///erase/////////////////////////////////////////////////////////////
	///erase the first instance of type Type
	template <typename Type_list, typename Type> struct Erase;

	template <typename Type>                         /// Specialization 1
	struct Erase<TYPE::Null_type, Type>
	{
		typedef TYPE::Null_type Result;
	};

	template <typename Type, typename Next>             /// Specialization 2
	struct Erase<Type_node<Type, Next>, Type>
	{
		typedef Next Result;
	};

	template <typename This, typename Next, typename Type> /// Specialization 3
	struct Erase<Type_node<This, Next>, Type>
	{
		typedef Type_node<This, typename Erase<Next, Type>::Result> Result;
	};

	///erase/////////////////////////////////////////////////////////////


	///erase all/////////////////////////////////////////////////////////
	///erase all instances of Type
	template <typename Type_list, typename Type> struct EraseAll;

	template <typename Type>
	struct EraseAll<TYPE::Null_type, Type>
	{
		typedef TYPE::Null_type Result;
	};

	template <typename Type, typename Next>
	struct EraseAll<Type_node<Type, Next>, Type>
	{
		/// Go all the way down the list removing the type
		typedef typename EraseAll<Next, Type>::Result Result;
	};

	template <typename This, typename Next, typename Type>
	struct EraseAll<Type_node<This, Next>, Type>
	{
		/// Go all the way down the list removing the type
		typedef Type_node<This, typename EraseAll<Next, Type>::Result> Result;
	};
	///erase all/////////////////////////////////////////////////////////

}///endof namespace TYPELIST


#endif//TYPE_LIST_H_

