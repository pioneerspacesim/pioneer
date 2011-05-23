#ifndef LUA_FUNCTION_H_
#	define LUA_FUNCTION_H_

///////////////////////////////////////////////////////////////////////////////
///  @file lua_function.h
///  @remarks Warning this file was generated, edits to it will not persist if 
///  the file is regenerated.
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence 
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#	include "lua_includes.h"
#	include "fwd_push_pull.h"
#	include "lua_ref.h"
namespace OOLUA
{
///////////////////////////////////////////////////////////////////////////////
///  Lua_function
///  Struct which is used to call a lua function.
///  @remarks
///  The Lua function can either be called by name(std::string) or with the
///  use a Lua reference which is stored in a Lua_func.
///////////////////////////////////////////////////////////////////////////////
struct Lua_function
{
	template<typename FUNC>
	bool operator()(FUNC const&  func_name)
	{
		set_function(func_name);

		return call(0);
	}
	template<typename FUNC,typename P1>
	bool operator()(FUNC const&  func_name,P1 p1)
	{
		set_function(func_name);
		push2lua(m_lua,p1); 
		return call(1);
	}
	template<typename FUNC,typename P1,typename P2>
	bool operator()(FUNC const&  func_name,P1 p1,P2 p2)
	{
		set_function(func_name);
		push2lua(m_lua,p1); push2lua(m_lua,p2); 
		return call(2);
	}
	template<typename FUNC,typename P1,typename P2,typename P3>
	bool operator()(FUNC const&  func_name,P1 p1,P2 p2,P3 p3)
	{
		set_function(func_name);
		push2lua(m_lua,p1); push2lua(m_lua,p2); push2lua(m_lua,p3); 
		return call(3);
	}
	template<typename FUNC,typename P1,typename P2,typename P3,typename P4>
	bool operator()(FUNC const&  func_name,P1 p1,P2 p2,P3 p3,P4 p4)
	{
		set_function(func_name);
		push2lua(m_lua,p1); push2lua(m_lua,p2); push2lua(m_lua,p3); push2lua(m_lua,p4); 
		return call(4);
	}
	template<typename FUNC,typename P1,typename P2,typename P3,typename P4,typename P5>
	bool operator()(FUNC const&  func_name,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
	{
		set_function(func_name);
		push2lua(m_lua,p1); push2lua(m_lua,p2); push2lua(m_lua,p3); push2lua(m_lua,p4); push2lua(m_lua,p5); 
		return call(5);
	}
	template<typename FUNC,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
	bool operator()(FUNC const&  func_name,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
	{
		set_function(func_name);
		push2lua(m_lua,p1); push2lua(m_lua,p2); push2lua(m_lua,p3); push2lua(m_lua,p4); push2lua(m_lua,p5); push2lua(m_lua,p6); 
		return call(6);
	}
	template<typename FUNC,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
	bool operator()(FUNC const&  func_name,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
	{
		set_function(func_name);
		push2lua(m_lua,p1); push2lua(m_lua,p2); push2lua(m_lua,p3); push2lua(m_lua,p4); push2lua(m_lua,p5); push2lua(m_lua,p6); push2lua(m_lua,p7); 
		return call(7);
	}
	template<typename FUNC,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8>
	bool operator()(FUNC const&  func_name,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8)
	{
		set_function(func_name);
		push2lua(m_lua,p1); push2lua(m_lua,p2); push2lua(m_lua,p3); push2lua(m_lua,p4); push2lua(m_lua,p5); push2lua(m_lua,p6); push2lua(m_lua,p7); push2lua(m_lua,p8); 
		return call(8);
	}
	template<typename FUNC,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8,typename P9>
	bool operator()(FUNC const&  func_name,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8,P9 p9)
	{
		set_function(func_name);
		push2lua(m_lua,p1); push2lua(m_lua,p2); push2lua(m_lua,p3); push2lua(m_lua,p4); push2lua(m_lua,p5); push2lua(m_lua,p6); push2lua(m_lua,p7); push2lua(m_lua,p8); push2lua(m_lua,p9); 
		return call(9);
	}
	template<typename FUNC,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8,typename P9,typename P10>
	bool operator()(FUNC const&  func_name,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8,P9 p9,P10 p10)
	{
		set_function(func_name);
		push2lua(m_lua,p1); push2lua(m_lua,p2); push2lua(m_lua,p3); push2lua(m_lua,p4); push2lua(m_lua,p5); push2lua(m_lua,p6); push2lua(m_lua,p7); push2lua(m_lua,p8); push2lua(m_lua,p9); push2lua(m_lua,p10); 
		return call(10);
	}
	void bind_script(lua_State* const lua);
private:
	bool call(int const& count);
	void set_function(std::string const& func);
	void set_function(Lua_func_ref const& func);
	lua_State* m_lua;
	int m_error_func_index;

};
}


#endif 
