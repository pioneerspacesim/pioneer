#ifndef OOLUA_PUSH_PULL_H_
#	define OOLUA_PUSH_PULL_H_

#include <string>
#include "lua_includes.h"
#include "lua_table.h"
#include "lua_ref.h"
#include "lvd_types.h"
#include "param_traits.h"
#include "proxy_class.h"
#include "class_from_stack.h"
#include "push_pointer_internal.h"
#include "fwd_push_pull.h"
#include "oolua_storage.h"
#include "oolua_parameter_helper.h"

#include <cassert>

namespace OOLUA
{
	namespace INTERNAL
	{
		typedef char (&one)[1];
		typedef char (&two)[2];

		template <typename T,typename From>
		class can_convert
		{
			static one test(T );
			template <typename U>
			static two test(...);
		public:
			enum { value = sizeof( test( From() ) )  == 1 ? 1 : 0 };
		};

		template<typename T,int is_intergal>
		struct push_basic_type;



		template<typename T>
		struct push_basic_type<T,0>
		{

			static void push2lua(lua_State* const  s, T const&  value)
			{
				//enumeration type so a static cast must be allowed.
				//enums will be stronger in C++0x so this will need revisiting then
				typedef char dummy_can_convert [ can_convert<int,T>::value ? 1 : -1];
				lua_pushinteger(s, static_cast<lua_Integer>(value) );
			}
		};

		template<typename T>
		struct push_basic_type<T,1>
		{
			static void push2lua(lua_State* const  s, T const&  value)
			{
				lua_pushinteger(s, static_cast<lua_Integer>(value) );
			}
		};


		template<typename T,int is_it_const>
		struct ptr_push
		{
			static void push(lua_State* const s, OOLUA::lua_acquire_ptr<T>&  value)
			{
				INTERNAL::push_pointer<T>(s,value.m_ptr,Lua);
			}
		};

		template<typename T>
		struct ptr_push<T,1>
		{
			static void push(lua_State* const s, OOLUA::lua_acquire_ptr<T>&  value)
			{
				INTERNAL::push_const_pointer<typename LVD::remove_const<T>::type>(s,value.m_ptr,Lua);
			}
		};

		template<typename T,bool IsIntegral>
		struct push_ptr_2lua;

		template<typename T>
		struct push_ptr_2lua<T,false>
		{
			static void push2lua(lua_State* const l, T * const &  value,Owner owner)
			{
				assert(l && value);
				push(l,value,LVD::Int2type<LVD::is_const<T>::value>(),owner);
			}

			static void push2lua(lua_State* const l, T * const &  value)
			{
				assert(l && value);
				push(l,value,LVD::Int2type<LVD::is_const<T>::value>());
			}
		private:
			static void push(lua_State* const l, T* const & value, LVD::Int2type<0> /*is_const*/ )
			{
				INTERNAL::push_pointer<T>(l,value,No_change);
			}
			static void push(lua_State* const l, T* const & value, LVD::Int2type<1>   /*is_const*/)
			{
				INTERNAL::push_const_pointer<typename LVD::remove_const<T>::type >(l,value,No_change);
			}
			static void push(lua_State* const l, T* const & value, LVD::Int2type<0> /*is_const*/ ,Owner owner )
			{
				INTERNAL::push_pointer<T>(l,value,owner);
			}
			static void push(lua_State* const l, T* const & value, LVD::Int2type<1> /*is_const*/ ,Owner owner  )
			{
				INTERNAL::push_const_pointer<typename LVD::remove_const<T>::type>(l,value,owner);
			}
		};

		template<typename T>
		struct push_ptr_2lua<T,true>
		{
			//why is owner here?
			static void push2lua(lua_State* const l, T * const &  value,Owner/* owner*/)
			{
				assert(l && value);
				OOLUA::push2lua(l,*value);
			}
			static void push2lua(lua_State* const l, T * const &  value)
			{
				assert(l && value);
				OOLUA::push2lua(l,*value);
			}
		};

		///////////////////////////////////////////////////////////////////////////////
		///  Specialisation for C style strings
		///////////////////////////////////////////////////////////////////////////////
		template<>
		struct push_ptr_2lua<char,true>
		{
			static void push2lua(lua_State* const l, char * const &  value,Owner/* owner*/)
			{
				assert(l && value);
				lua_pushstring (l,value);
			}
			static void push2lua(lua_State* const l, char * const &  value)
			{
				assert(l && value);
				lua_pushstring (l,value);
			}
		};
		template<>
		struct push_ptr_2lua<char const,true>
		{
			static void push2lua(lua_State* const l, char const * const &  value,Owner/* owner*/)
			{
				assert(l && value);
				lua_pushstring (l,value);
			}
			static void push2lua(lua_State* const l, char const * const &  value)
			{
				assert(l && value);
				lua_pushstring (l,value);
			}
		};

	}

	void inline push2lua(lua_State* const s, bool const& value)
	{
		assert(s);
		lua_pushboolean(s, (value? 1 : 0) );
	}
	void inline push2lua(lua_State* const s, std::string const& value)
	{
		assert(s);
		lua_pushstring (s, value.c_str());
	}
	void inline push2lua(lua_State* const s, char const * const& value)
	{
		assert(s && value);
		lua_pushstring (s, value);
	}
	void inline push2lua(lua_State* const s, char * const& value)
	{
		assert(s && value);
		lua_pushstring (s, value);
	}
	void inline push2lua(lua_State* const s, double const& value)
	{
		assert(s);
		lua_pushnumber(s, value);
	}
	void inline push2lua(lua_State* const s, float const&  value)
	{
		assert(s);
		lua_pushnumber(s, value);
	}
	void inline push2lua(lua_State* const s, lua_CFunction const &  value)
	{
		assert(s );
		lua_pushcclosure(s,value,0);
	}
	void inline push2lua(lua_State* const s, Lua_table const &  value)
	{
		assert(s /*&& value.valid() */);
		//value.get_table();
		value.push_on_stack(s);
	}

	void inline push2lua(lua_State* const s, Lua_func_ref const &  value)
	{
		assert(s /*value.valid()*/ );
		value.push(s);
	}
	template<typename T>
	void inline push2lua(lua_State* const  s, T const&  value)
	{
		INTERNAL::push_basic_type<T,LVD::is_integral_type<T>::value >::push2lua(s,value);
	}

	//pushes a pointer onto the stack which Lua will then own and call delete on
	template<typename T>
	void push2lua(lua_State* const s, OOLUA::lua_acquire_ptr<T>&  value)
	{
		assert(s && value.m_ptr);
		INTERNAL::ptr_push<T,LVD::is_const<T>::value >::push(s,value);
	}

	template<typename T>
	inline void push2lua(lua_State* const s, T * const &  value,Owner owner)
	{
		INTERNAL::push_ptr_2lua<T,LVD::is_integral_type<typename LVD::remove_const<T>::type >::value>::push2lua(s,value,owner);
	}
	template<typename T>
	inline void push2lua(lua_State* const s, T * const &  value)
	{
		INTERNAL::push_ptr_2lua<T,LVD::is_integral_type<T>::value>::push2lua(s,value);
	}













	void inline pull2cpp(lua_State* const s, bool& value)
	{
		assert(s && lua_isboolean(s,-1) );
		value =  lua_toboolean( s, -1) ? true : false;
		lua_pop( s, 1);
	}
	void inline pull2cpp(lua_State* const s, std::string& value)
	{
		assert(s && lua_isstring(s,-1) );
		value = lua_tolstring(s,-1,0);
		lua_pop( s, 1);
	}
	void inline pull2cpp(lua_State* const s, double& value)
	{
		assert(s && lua_isnumber(s,-1) );
		value = static_cast<double>( lua_tonumber( s, -1) );
		lua_pop( s, 1);
	}
	void inline pull2cpp(lua_State* const s, float& value)
	{
		assert(s && lua_isnumber(s,-1) );
		value = static_cast<float>( lua_tonumber( s, -1) );
		lua_pop( s, 1);
	}
	void inline pull2cpp(lua_State* const s, lua_CFunction& value)
	{
		assert(s && lua_iscfunction(s,-1) );
		value = lua_tocfunction( s, -1);
		lua_pop( s, 1);
	}
	
	void inline pull2cpp(lua_State* const s, Lua_func_ref& value)
	{
		value.pull(s);
	}

	void inline pull2cpp(lua_State* const s, Lua_table&  value)
	{
		value.pull_from_stack(s);
	}

	void inline pull2cpp(lua_State* const s, Lua_table_ref& value)
	{
		value.pull(s);
	}

	namespace INTERNAL
	{
		template<typename T,int is_intergal>
		struct pull_basic_type;

		template<typename T>
		struct pull_basic_type<T,0>//enum
		{
			static void pull2cpp(lua_State* const  s, T &  value)
			{
				//enumeration type so a static cast should be allowed else this
				//is being called with the wrong type
				typedef char dummy_can_convert [ can_convert<int,T>::value ? 1 : -1];
				//value = static_cast<T>( lua_tonumber( s, -1) );
				value = static_cast<T>( lua_tointeger( s, -1) );
				lua_pop( s, 1);
			}
		};

		template<typename T>
		struct pull_basic_type<T,1>
		{
			static void pull2cpp(lua_State* const  s, T &  value)
			{
				assert(s && lua_isnumber(s,-1) );
				value = static_cast<T>( lua_tointeger( s, -1) );
				lua_pop( s, 1);
			}
		};
		
		
		///////////////////////////////////////////////////////////////////////////////
		///  @struct pull_ptr_2cpp
		///  Pulls a pointer to C++ depending on the second template parameter. If it
		///  is true then the type is an integral type and one of the normal overloaded
		///  OOLUA::pull2cpp functions are called. If on the other hand the type is not
		///  integral then OOLUA::Lua_interface is used to remove the pointer from the
		///  Lua stack.
		///////////////////////////////////////////////////////////////////////////////
		template<typename T,bool IsIntregal>
		struct pull_ptr_2cpp;

		inline void pull_class_type_error(lua_State* const s,char const* type)
		{
			luaL_error(s,"%s %s %s","tried to pull type"
				,type
				,"which is not the type or a base of the type on the stack");
		}

		template<typename Pull_type>
		inline void pull_class_type(lua_State *const s,int Is_const,Pull_type*& class_type)
		{
#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable : 4127)//conditional expression is constant
#endif
			if(Is_const) class_type = INTERNAL::class_from_stack_top< Pull_type >(s);
			else class_type = INTERNAL::none_const_class_from_stack_top<Pull_type>(s);
#ifdef _MSC_VER
#	pragma warning(pop)
#endif
		}


		template<typename T>
		struct pull_ptr_2cpp<T,false>
		{
			static void pull2cpp(lua_State* const s, T *&  value)
			{
				assert(s);
				typename OOLUA::param_type<T>::raw_type* class_ptr;
				pull_class_type<typename OOLUA::param_type<T>::raw_type>(s,OOLUA::param_type<T*>::is_constant,class_ptr);
				if(!class_ptr )
				{
#if 1
					pull_class_type_error(s,OOLUA::param_type<T*>::is_constant 
						? Proxy_class<typename OOLUA::param_type<T>::raw_type>::class_name_const 
						: Proxy_class<typename OOLUA::param_type<T>::raw_type>::class_name);
#elif defined OOLUA_EXCEPTIONS
					throw Type_error("tried to pull a type that is not the type or a base of the type on the stack");
#else
					value = 0;
#endif
					return;
				}
				assert(class_ptr);
				value = class_ptr;
				lua_pop(s,1);
			}
		};
		template<typename T>
		struct pull_ptr_2cpp<T,true>
		{
			static void pull2cpp(lua_State* const s, T *&  value)
			{
				///integral type, dereference and call the normal pull function
				///FIXME: we may well be dereferencing a null or uninitialised
				///pointer here!!!!!!!!
				assert(value);
				OOLUA::pull2cpp(s,*value);
			}
		};

	}
	template<typename T> 
	inline void pull2cpp(lua_State* const s, T& value)
	{
		INTERNAL::pull_basic_type<T,LVD::is_integral_type<T>::value>::pull2cpp(s,value);
	}

	//pulls a pointer from the stack which Cpp will then own and call delete on
	template<typename T>
	inline void pull2cpp(lua_State* const s, OOLUA::cpp_acquire_ptr<T>&  value)
	{
		assert(s);
		typename cpp_acquire_ptr<T>::raw* class_ptr;
		INTERNAL::pull_class_type<typename cpp_acquire_ptr<T>::raw>(s,OOLUA::param_type<T*>::is_constant,class_ptr);
		if(!class_ptr )
		{
#if 1 
			INTERNAL::pull_class_type_error(s,OOLUA::param_type<T*>::is_constant
					? Proxy_class<typename cpp_acquire_ptr<T>::raw>::class_name_const 
					: Proxy_class<typename cpp_acquire_ptr<T>::raw>::class_name);
#elif defined OOLUA_EXCEPTIONS
			throw Type_error("tried to pull a type that is not the type or a base of the type on the stack");
#else
			value.m_ptr = 0;
#endif
			return;
		}
		assert(class_ptr);
		value.m_ptr = class_ptr;
		INTERNAL::set_owner(s,value.m_ptr,OOLUA::Cpp);
		lua_pop( s, 1);
	}

	///////////////////////////////////////////////////////////////////////////////
	///  inline public overloaded  pull2cpp
	///  Checks if it is an integral type( LVD::is_intergal_type ) or that is a type
	///  that should be registered to the OOLUA::Lua_interface and call the correct
	///  function.
	///  @param [in]       s lua_State *const \copydoc lua_State
	///  @param [in, out]  value T *&
	///  This function doesn't return a value
	///  @remarks
	///  @copydoc pulling_cpp_values
	///  @see pulling_cpp_values
	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void pull2cpp(lua_State* const s, T *&  value)
	{
		INTERNAL::pull_ptr_2cpp<T,LVD::is_integral_type<T>::value>::pull2cpp(s,value);
	}

}

#endif







