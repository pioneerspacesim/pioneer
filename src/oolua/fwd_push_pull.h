///////////////////////////////////////////////////////////////////////////////
///  @file fwd_push_pull.h
///  Foward declarations of the push and pull methods to and from the script.
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details. \n 

///////////////////////////////////////////////////////////////////////////////


#ifndef FORWARD_DECLARE_PUSH_PULL_H_

#	define FORWARD_DECLARE_PUSH_PULL_H_



#include <string>

#include "param_traits.h" //required for OOLUA::Owner

namespace OOLUA

{

    template<int ID>struct Lua_ref;

    class Lua_table;

	template<typename T>struct lua_acquire_ptr;

	template<typename T>struct cpp_acquire_ptr;

    

	void push2lua(lua_State* const s, bool const& value);

	void push2lua(lua_State* const s, std::string const& value);

	void push2lua(lua_State* const s, char const * const& value);

	void push2lua(lua_State* const s, double const& value);

	void push2lua(lua_State* const s, float const&  value);

	void push2lua(lua_State* const s, lua_CFunction const &  value);
	
	void push2lua(lua_State* const s, Lua_table const &  value);
	void push2lua(lua_State* const s, Lua_func_ref const &  value);
	

	template<typename T>void push2lua(lua_State* const s, T * const &  value);

	template<typename T>void push2lua(lua_State* const s, T * const &  value,OOLUA::Owner);

	template<typename T>void push2lua(lua_State* const s, lua_acquire_ptr<T>&  value);

	template<typename T>void push2lua(lua_State* const s, T const &  value);



	void pull2cpp(lua_State* const s, bool& value);

	void pull2cpp(lua_State* const s, std::string& value);

	void pull2cpp(lua_State* const s, double& value);

	void pull2cpp(lua_State* const s, float& value);

	void pull2cpp(lua_State* const s, lua_CFunction& value);

	void pull2cpp(lua_State* const s, Lua_table&  value);

	void pull2cpp(lua_State* const s, Lua_ref<1>&  value);

	template<typename T>void pull2cpp(lua_State* const s, T *&  value);

	template<typename T>void pull2cpp(lua_State* const s, T&  value);

	template<typename T>void pull2cpp(lua_State* const s, cpp_acquire_ptr<T>&  value);	

}



#endif //FORWARD_DECLARE_PUSH_PULL_H_

