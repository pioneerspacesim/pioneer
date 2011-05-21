#ifndef PROXY_CALLER_H_
#	define PROXY_CALLER_H_

///////////////////////////////////////////////////////////////////////////////
///  @file proxy_caller.h
///  @remarks Warning this file was generated, edits to it will not persist if 
///  the file is regenerated.
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence 
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#	include "member_func_helper.h"
#	include "param_traits.h"

#define OOLUA_CONVERTER(NUM)\
	Converter<typename P##NUM::pull_type, typename P##NUM::type> p##NUM##_(p##NUM);

#define OOLUA_CONVERTER1\
	OOLUA_CONVERTER(1)

#define OOLUA_CONVERTER2\
	OOLUA_CONVERTER1\
	OOLUA_CONVERTER(2)

#define OOLUA_CONVERTER3\
	OOLUA_CONVERTER2\
	OOLUA_CONVERTER(3)

#define OOLUA_CONVERTER4\
	OOLUA_CONVERTER3\
	OOLUA_CONVERTER(4)

#define OOLUA_CONVERTER5\
	OOLUA_CONVERTER4\
	OOLUA_CONVERTER(5)

#define OOLUA_CONVERTER6\
	OOLUA_CONVERTER5\
	OOLUA_CONVERTER(6)

#define OOLUA_CONVERTER7\
	OOLUA_CONVERTER6\
	OOLUA_CONVERTER(7)

#define OOLUA_CONVERTER8\
	OOLUA_CONVERTER7\
	OOLUA_CONVERTER(8)

namespace OOLUA
{
template <typename Return, typename Class, int ReturnIsVoid>struct Proxy_caller;

template <typename R, typename C>
struct Proxy_caller<R,C, 0 >
{
	template<typename FuncType>
	static void call(lua_State*  const l,C* m_this, FuncType ptr2mem )
	{
		typename R::type r( (m_this->*ptr2mem)() );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename FuncType>
	static void call(lua_State* const  l,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1)
	{
		OOLUA_CONVERTER1
		typename R::type r( (m_this->*ptr2mem) (p1_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename FuncType>
	static void call(lua_State* const  l,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2)
	{
		OOLUA_CONVERTER2
		typename R::type r( (m_this->*ptr2mem) (p1_,p2_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename FuncType>
	static void call(lua_State* const  l,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3)
	{
		OOLUA_CONVERTER3
		typename R::type r( (m_this->*ptr2mem) (p1_,p2_,p3_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename FuncType>
	static void call(lua_State* const  l,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4)
	{
		OOLUA_CONVERTER4
		typename R::type r( (m_this->*ptr2mem) (p1_,p2_,p3_,p4_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename FuncType>
	static void call(lua_State* const  l,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5)
	{
		OOLUA_CONVERTER5
		typename R::type r( (m_this->*ptr2mem) (p1_,p2_,p3_,p4_,p5_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename FuncType>
	static void call(lua_State* const  l,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6)
	{
		OOLUA_CONVERTER6
		typename R::type r( (m_this->*ptr2mem) (p1_,p2_,p3_,p4_,p5_,p6_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename FuncType>
	static void call(lua_State* const  l,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6,typename P7::pull_type&  p7)
	{
		OOLUA_CONVERTER7
		typename R::type r( (m_this->*ptr2mem) (p1_,p2_,p3_,p4_,p5_,p6_,p7_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8,typename FuncType>
	static void call(lua_State* const  l,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6,typename P7::pull_type&  p7,typename P8::pull_type&  p8)
	{
		OOLUA_CONVERTER8
		typename R::type r( (m_this->*ptr2mem) (p1_,p2_,p3_,p4_,p5_,p6_,p7_,p8_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}

};

template <typename R, typename C>
struct Proxy_caller<R,C, 1 >
{
	template<typename FuncType>
	static void call(lua_State*  const /*l*/,C* m_this, FuncType ptr2mem )
	{
		(m_this->*ptr2mem)();
	}
	template<typename P1,typename FuncType>
	static void call(lua_State* const  /*l*/,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1)
	{
		OOLUA_CONVERTER1
		(m_this->*ptr2mem)(p1_);
	}
	template<typename P1,typename P2,typename FuncType>
	static void call(lua_State* const  /*l*/,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2)
	{
		OOLUA_CONVERTER2
		(m_this->*ptr2mem)(p1_,p2_);
	}
	template<typename P1,typename P2,typename P3,typename FuncType>
	static void call(lua_State* const  /*l*/,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3)
	{
		OOLUA_CONVERTER3
		(m_this->*ptr2mem)(p1_,p2_,p3_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename FuncType>
	static void call(lua_State* const  /*l*/,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4)
	{
		OOLUA_CONVERTER4
		(m_this->*ptr2mem)(p1_,p2_,p3_,p4_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename FuncType>
	static void call(lua_State* const  /*l*/,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5)
	{
		OOLUA_CONVERTER5
		(m_this->*ptr2mem)(p1_,p2_,p3_,p4_,p5_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename FuncType>
	static void call(lua_State* const  /*l*/,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6)
	{
		OOLUA_CONVERTER6
		(m_this->*ptr2mem)(p1_,p2_,p3_,p4_,p5_,p6_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename FuncType>
	static void call(lua_State* const  /*l*/,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6,typename P7::pull_type&  p7)
	{
		OOLUA_CONVERTER7
		(m_this->*ptr2mem)(p1_,p2_,p3_,p4_,p5_,p6_,p7_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8,typename FuncType>
	static void call(lua_State* const  /*l*/,C* m_this,FuncType ptr2mem,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6,typename P7::pull_type&  p7,typename P8::pull_type&  p8)
	{
		OOLUA_CONVERTER8
		(m_this->*ptr2mem)(p1_,p2_,p3_,p4_,p5_,p6_,p7_,p8_);
	}

};
template <typename Return, int ReturnIsVoid>struct Proxy_none_member_caller;

template <typename R> 
struct Proxy_none_member_caller<R,0 > 
{
	template<typename FuncType> 
	static void call(lua_State*  const l,FuncType ptr2func ) 
	{
		typename R::type r( (*ptr2func)() );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename FuncType>
	static void call(lua_State* const  l,FuncType ptr2func,typename P1::pull_type&  p1)
	{
		OOLUA_CONVERTER1
		typename R::type r( (*ptr2func) (p1_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename FuncType>
	static void call(lua_State* const  l,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2)
	{
		OOLUA_CONVERTER2
		typename R::type r( (*ptr2func) (p1_,p2_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename FuncType>
	static void call(lua_State* const  l,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3)
	{
		OOLUA_CONVERTER3
		typename R::type r( (*ptr2func) (p1_,p2_,p3_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename FuncType>
	static void call(lua_State* const  l,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4)
	{
		OOLUA_CONVERTER4
		typename R::type r( (*ptr2func) (p1_,p2_,p3_,p4_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename FuncType>
	static void call(lua_State* const  l,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5)
	{
		OOLUA_CONVERTER5
		typename R::type r( (*ptr2func) (p1_,p2_,p3_,p4_,p5_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename FuncType>
	static void call(lua_State* const  l,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6)
	{
		OOLUA_CONVERTER6
		typename R::type r( (*ptr2func) (p1_,p2_,p3_,p4_,p5_,p6_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename FuncType>
	static void call(lua_State* const  l,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6,typename P7::pull_type&  p7)
	{
		OOLUA_CONVERTER7
		typename R::type r( (*ptr2func) (p1_,p2_,p3_,p4_,p5_,p6_,p7_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8,typename FuncType>
	static void call(lua_State* const  l,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6,typename P7::pull_type&  p7,typename P8::pull_type&  p8)
	{
		OOLUA_CONVERTER8
		typename R::type r( (*ptr2func) (p1_,p2_,p3_,p4_,p5_,p6_,p7_,p8_) );
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
	}
};

template <typename R >
struct Proxy_none_member_caller<R, 1 >
{
	template<typename FuncType>
	static void call(lua_State*  const /*l*/, FuncType ptr2func )
	{
		(*ptr2func)();
	}
	template<typename P1,typename FuncType>
	static void call(lua_State* const  /*l*/,FuncType ptr2func,typename P1::pull_type&  p1)
	{
		OOLUA_CONVERTER1
		(*ptr2func)(p1_);
	}
	template<typename P1,typename P2,typename FuncType>
	static void call(lua_State* const  /*l*/,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2)
	{
		OOLUA_CONVERTER2
		(*ptr2func)(p1_,p2_);
	}
	template<typename P1,typename P2,typename P3,typename FuncType>
	static void call(lua_State* const  /*l*/,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3)
	{
		OOLUA_CONVERTER3
		(*ptr2func)(p1_,p2_,p3_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename FuncType>
	static void call(lua_State* const  /*l*/,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4)
	{
		OOLUA_CONVERTER4
		(*ptr2func)(p1_,p2_,p3_,p4_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename FuncType>
	static void call(lua_State* const  /*l*/,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5)
	{
		OOLUA_CONVERTER5
		(*ptr2func)(p1_,p2_,p3_,p4_,p5_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename FuncType>
	static void call(lua_State* const  /*l*/,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6)
	{
		OOLUA_CONVERTER6
		(*ptr2func)(p1_,p2_,p3_,p4_,p5_,p6_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename FuncType>
	static void call(lua_State* const  /*l*/,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6,typename P7::pull_type&  p7)
	{
		OOLUA_CONVERTER7
		(*ptr2func)(p1_,p2_,p3_,p4_,p5_,p6_,p7_);
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8,typename FuncType>
	static void call(lua_State* const  /*l*/,FuncType ptr2func,typename P1::pull_type&  p1,typename P2::pull_type&  p2,typename P3::pull_type&  p3,typename P4::pull_type&  p4,typename P5::pull_type&  p5,typename P6::pull_type&  p6,typename P7::pull_type&  p7,typename P8::pull_type&  p8)
	{
		OOLUA_CONVERTER8
		(*ptr2func)(p1_,p2_,p3_,p4_,p5_,p6_,p7_,p8_);
	}

};

}
#undef OOLUA_CONVERTER
#undef OOLUA_CONVERTER1
#undef OOLUA_CONVERTER2
#undef OOLUA_CONVERTER3
#undef OOLUA_CONVERTER4
#undef OOLUA_CONVERTER5
#undef OOLUA_CONVERTER6
#undef OOLUA_CONVERTER7
#undef OOLUA_CONVERTER8


#endif 
