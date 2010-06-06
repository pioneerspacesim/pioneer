///////////////////////////////////////////////////////////////////////////////
///  @file member_func_helper.h
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#ifndef MEMBER_FUNC_HELPER_H_
#	define MEMBER_FUNC_HELPER_H_

#	include "lua_includes.h"
#	include "oolua_push_pull.h"
#	include "oolua_error.h"

namespace OOLUA
{

	namespace
	{
		template<typename Raw,typename T, int is_by_value, int is_constant>
		struct Internal_push
		{
			static void push(Owner owner,lua_State* const s, T& value)
			{
				push(owner,s,value, LVD::Int2type<is_by_value>());
			}
		private:
			
			//its by ref
			static void push(Owner owner,lua_State* const s, T& value,LVD::Int2type<0> /*t*/)
			{
				OOLUA::INTERNAL::push_pointer<Raw>(s,&value,owner);
			}
			
			//by value
			static void push(Owner/* owner*/,lua_State* const s, T& value,LVD::Int2type<1> /*t*/)
			{
				//this needs an allocation and push onto the stack
				Raw* ptr = new Raw(value);
				OOLUA::INTERNAL::push_pointer<Raw>(s,ptr,Lua);
			}
		};
		template<typename Raw,typename T,int is_by_value >
		struct Internal_push<Raw,T,is_by_value , 1>
		{
			static void push(Owner owner,lua_State* const s, T& value)
			{
				push(owner,s,value, LVD::Int2type<is_by_value>());
			}
		private:
			//its constant is it by ref
			static void push(Owner owner,lua_State* const s, T& value,LVD::Int2type<0> /*t*/)
			{
				OOLUA::INTERNAL::push_const_pointer<Raw>(s,&value,owner);
			}
			//must be by value
			static void push(Owner/* owner*/,lua_State* const s, T& value,LVD::Int2type<1> /*t*/)
			{
				//this needs an allocation and push onto the stack
				Raw* ptr = new Raw(value);
				OOLUA::INTERNAL::push_const_pointer<Raw>(s,ptr,Lua);
			}
		};

		template<typename T,typename WT,int is_integral = 1>
		struct Is_intergal_pushpull
		{
			static void pull(lua_State* const s, T& value)
			{
				OOLUA::INTERNAL::LUA_CALLED::pull2cpp(s,value);
			}
			static void push(lua_State* const s, T& value)
			{
				OOLUA::push2lua(s,value);
			}
		};

		//user defined type
		template<typename T,typename WT>
		struct Is_intergal_pushpull<T,WT,0>
		{
			static void pull(lua_State* const s, T& value)
			{
				T* t= &value;
				OOLUA::INTERNAL::LUA_CALLED::pull2cpp(s,t);
			}
			static void pull(lua_State* const s, T*& value)
			{
				OOLUA::INTERNAL::LUA_CALLED::pull2cpp(s,value);
			}
			static void push(lua_State* const s, T& value)
			{
				Internal_push<typename WT::raw_type,T,WT::is_by_value, WT::is_constant>::push((Owner)WT::owner,s,value);
			}
		};
	}


	template<typename Type, int owner>struct Member_func_helper;
	template<typename Type>
	struct Member_func_helper<Type,No_change>
	{
		template<typename T>
		static void pull2cpp(lua_State* const s, T*& value)
		{
			Is_intergal_pushpull<T,Type,Type::is_integral>::pull(s,value);
		}
		template<typename T>
		static void pull2cpp(lua_State* const s, T& value)
		{
			Is_intergal_pushpull<T,Type,Type::is_integral>::pull(s,value);
		}
		template<typename T>
		static void push2lua(lua_State* const s, T& value)
		{
			Is_intergal_pushpull<T,Type,Type::is_integral>::push(s,value);
		}
		template<typename T>
		static void push2lua(lua_State* const s, T*& value)
		{
			OOLUA::push2lua(s,value,No_change);
		}
		///special case "T* const" and "T const * const"
		template<typename T>
		static void push2lua(lua_State* const s, T*const& value)
		{
			OOLUA::push2lua(s,value,No_change);
		}

	};
	template<typename Type>
	struct Member_func_helper<Type,Cpp>
	{
		template<typename T>
		static void pull2cpp(lua_State* const s, T*& value)
		{
			OOLUA::cpp_acquire_ptr<T> p;
			OOLUA::INTERNAL::LUA_CALLED::pull2cpp(s,p);
			value = p.m_ptr;
		}
		template<typename T>
		static void push2lua(lua_State* const /*s*/, T*& /*value*/)
		{
			assert(0 && "this function should never be called");
		}//noop
	};
	template<typename Type>
	struct Member_func_helper<Type,Lua>
	{
		template<typename T>
		static void push2lua(lua_State* const s, T*& value)
		{
			OOLUA::lua_acquire_ptr<T> p(value);
			OOLUA::push2lua(s,p);
		}
		template<typename T>
		static void pull2cpp(lua_State* const /*s*/, T*& /*value*/)//noop
		{
			//this function should never be called
			assert(0 && "this function should never be called");
		}
	};


}
 #endif

