///////////////////////////////////////////////////////////////////////////////
///  @file oolua.h
///  This is the header file to be include to use Object Oriented Lua.
///  It also defines the Script class and the push and pull methods to
///  to transfer data to and from Lua and C++.
///
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////



///  @struct lua_State
///  Lua's virtual machine.
///  All information about a state is kept in this structure.
struct lua_State;

#ifndef OOLUA_H_
#	define OOLUA_H_


#	include <string>

#	include "lua_includes.h"
#	include "lvd_types.h"
#	include "determin_qualifier.h"
#	include "param_traits.h"
#	include "export_func_to_lua.h"
#	include "cpp_member_func.h"
#	include "oolua_c_func.h"
#	include "lua_operator.h"
#	include "proxy_class.h"
#	include "class_public_member.h"
#	include "proxy_caller.h"
#	include "lua_function.h"
#	include "lua_table.h"
#	include "lua_ref.h"
#	include "class_from_stack.h"
#	include "lua_stack_dump.h"
#	include "oolua_typedefs.h"
#	include "member_func_helper.h"
#	include "oolua_exception.h"
#   include "oolua_registration.h"
#	include "oolua_push_pull.h"
#   include "oolua_error.h"
#	include "oolua_version.h"
#	include "cpp_constructor.h"
#	include "oolua_va_args.h"

namespace OOLUA
{

	///////////////////////////////////////////////////////////////////////////////
	///  @class Script
	///  Simple wrapper around a lua_State which enables file reading via a member
	///  function and Lua function calling via the public member Lua_function
	///  instance.
	///
	///  @remarks
	///  This class is not copy constructible or assignable.
	///  To accomplish this a counted reference to the lua_State would need to be
	///  maintained.
	///  The class is only a helper, anything you can do with this class can be
	///  done either done via using a Lua_function struct, interfacing with
	///  Lua_interface and Lua API code to read a file. The class gives access to
	///  the lua_State via a cast operator and a function, the whole of OOLUA works
	///  with a lua_State and not a Script instance.
	///////////////////////////////////////////////////////////////////////////////
	class Script
	{
	public:
		Lua_function call;
		Script();
		~Script();
		template<typename T>
		void register_class();
		template<typename T>
		void register_class(T* );
		template<typename T,typename K,typename V>
		void register_class_static(K const& k, V const& v);
		bool run_file(std::string const & filename/*,bool multi_return = true*/);
		bool load_file(std::string const & filename);
		int stack_count(){ return lua_gettop(m_lua); }
		operator lua_State * /*const*/() const {return m_lua;}
		lua_State * const & get_ptr() const {return m_lua;}
		bool load_chunk(std::string const& chunk);
		bool run_chunk(std::string const& chunk);
		void gc();
	protected:
		void close_down();
		Script(Script const& );
		Script& operator = (Script const& );
		lua_State* m_lua;

	};


	template<typename T>
	inline void Script::register_class()
	{
		OOLUA::register_class<T>(m_lua);
	}
	template<typename T>
	inline void Script::register_class(T* /*dummy*/)
	{
		register_class<T>();
	}

	template<typename T,typename K,typename V>
	inline void Script::register_class_static(K const& k, V const& v)
	{
			OOLUA::register_class_static<T>(m_lua,k,v);
	}
	//if you want to use oolua with a lua_State you already have active
	//or supplied by some third party then calling this function
	//adds the necessary tables and globals for it to work with oolua.
	void setup_user_lua_state(lua_State* l);
	
	template<typename T>
	bool set_global(lua_State* l, char const* name, T& instance)
	{
		bool result = OOLUA::push2lua(l, instance);
		if (!result) return false;
		lua_setglobal(l, name);
		return true;
	}
	inline void set_global_to_nil(lua_State*l, char const * name)
	{
		lua_pushnil(l);
		lua_setglobal(l, name);
	}
	
	template<typename T>
	bool get_global(lua_State* l, char const* name, T& instance)
	{
		lua_getglobal(l, name);
		return OOLUA::pull2cpp(l, instance);
	}
	
}//endof namepsace OOLUA




#endif //OOLUA_H_


