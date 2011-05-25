///////////////////////////////////////////////////////////////////////////////
///  @file lua_operator.h
///  Defines operators which are available in scripts in respect to Proxy_class
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////

#ifndef LUA_OPERATOR_H_
#	define LUA_OPERATOR_H_

/*
Extract from the Lua programming Reference 5.1
2.8 - Metatables

Every value in Lua may have a metatable. This metatable is an ordinary Lua
table that defines the behavior of the original value under certain special
operations. You can change several aspects of the behavior of operations
over a value by setting specific fields in its metatable. For instance,
when a non-numeric value is the operand of an addition, Lua checks for a
function in the field "__add" in its metatable. If it finds one, Lua calls
this function to perform the addition.

We call the keys in a metatable events and the values metamethods. In the
previous example, the event is "add" and the metamethod is the function that
performs the addition.

You can query the metatable of any value through the getmetatable function.

You can replace the metatable of tables through the setmetatable function.
You cannot change the metatable of other types from Lua (except using the
debug library); you must use the C API for that.

Tables and full userdata have individual metatables (although multiple
tables and userdata can share their metatables); values of all other types
share one single metatable per type. So, there is one single metatable for
all numbers, one for all strings, etc.

A metatable may control how an object behaves in arithmetic operations,
order comparisons, concatenation, length operation, and indexing. A metatable
can also define a function to be called when a userdata is garbage collected.
For each of these operations Lua associates a specific key called an event.
When Lua performs one of these operations over a value, it checks whether
this value has a metatable with the corresponding event. If so, the value
associated with that key (the metamethod) controls how Lua will perform the
operation.

Metatables control the operations listed next. Each operation is identified
by its corresponding name. The key for each operation is a string with its
name prefixed by two underscores, '__'; for instance, the key for operation
"add" is the string "__add". The semantics of these operations is better
explained by a Lua function describing how the interpreter executes the operation.
*/

/*!
\page page3 Lua Operators
\section lua_extract_1_sec Extract from the Lua programming Reference 5.1
2.8 - Metatables
...\n
The code shown here in Lua is only illustrative; the real behavior is hard \n
coded in the interpreter and it is much more efficient than this simulation. \n
All functions used in these descriptions (rawget, tonumber, etc.) are described \n
in 5.1. In particular, to retrieve the metamethod of a given object, we use the \n
expression\n
\n
     metatable(obj)[event]\n
\n
This should be read as\n
\n
     rawget(getmetatable(obj) or {}, event)\n
\n
That is, the access to a metamethod does not invoke other metamethods, and the\n
access to objects with no metatables does not fail (it simply results in nil).\n
\n
    * "add": the + operation.\n
\n
      The function getbinhandler below defines how Lua chooses a handler for a\n
		binary operation. First, Lua tries the first operand. If its type does not \n
		define a handler for the operation, then Lua tries the second operand.\n
\n
           function getbinhandler (op1, op2, event)\n
             return metatable(op1)[event] or metatable(op2)[event]\n
           end\n
\n
      By using this function, the behavior of the op1 + op2 is\n
\n
\code
           function add_event (op1, op2)
             local o1, o2 = tonumber(op1), tonumber(op2)
             if o1 and o2 then  -- both operands are numeric?
               return o1 + o2   -- '+' here is the primitive 'add'
             else  -- at least one of the operands is not numeric
               local h = getbinhandler(op1, op2, "__add")
               if h then
                 -- call the handler with both operands
                 return (h(op1, op2))
               else  -- no handler available: default behavior
                 error()
               end\
             end\
           end\
\endcode
\n
    * "sub": the - operation. Behavior similar to the "add" operation.\n
    * "mul": the * operation. Behavior similar to the "add" operation.\n
    * "div": the / operation. Behavior similar to the "add" operation.\n
    * "mod": the % operation. Behavior similar to the "add" operation, with the\n
				operation o1 - floor(o1/o2)*o2 as the primitive operation.\n
    * "pow": the ^ (exponentiation) operation. Behavior similar to the "add" \n
				operation, with the function pow (from the C math library) as the primitive operation.\n
    * "unm": the unary - operation.\n
\code
           function unm_event (op)
             local o = tonumber(op)
             if o then  -- operand is numeric?
               return -o  -- '-' here is the primitive 'unm'
             else  -- the operand is not numeric.
               -- Try to get a handler from the operand
               local h = metatable(op).__unm
               if h then
                 -- call the handler with the operand
                 return (h(op))
               else  -- no handler available: default behavior
                 error()
               end
             end
           end
\endcode
    * "concat": the .. (concatenation) operation.
\code
           function concat_event (op1, op2)
             if (type(op1) == "string" or type(op1) == "number") and
                (type(op2) == "string" or type(op2) == "number") then
               return op1 .. op2  -- primitive string concatenation
             else
               local h = getbinhandler(op1, op2, "__concat")
               if h then
                 return (h(op1, op2))
               else
                 error()
               end
             end
           end
\endcode
    * "len": the # operation.
\code
           function len_event (op)
             if type(op) == "string" then
               return strlen(op)         -- primitive string length
             elseif type(op) == "table" then
               return #op                -- primitive table length
             else
               local h = metatable(op).__len
               if h then
                 -- call the handler with the operand
                 return (h(op))
               else  -- no handler available: default behavior
                 error()
               end
             end
           end
\endcode
      See 2.5.5 for a description of the length of a table.
    * "eq": the == operation. The function getcomphandler defines how Lua chooses
			metamethod for comparison operators. A metamethod only is selected when
			both objects being compared have the same type and the same metamethod
			for the selected operation.
\code
           function getcomphandler (op1, op2, event)
             if type(op1) ~= type(op2) then return nil end
             local mm1 = metatable(op1)[event]
             local mm2 = metatable(op2)[event]
             if mm1 == mm2 then return mm1 else return nil end
           end
\endcode
      The "eq" event is defined as follows:
\code
           function eq_event (op1, op2)
             if type(op1) ~= type(op2) then  -- different types?
               return false   -- different objects
             end
             if op1 == op2 then   -- primitive equal?
               return true   -- objects are equal
             end
             -- try metamethod
             local h = getcomphandler(op1, op2, "__eq")
             if h then
               return (h(op1, op2))
             else
               return false
             end
           end
\endcode
      a ~= b is equivalent to not (a == b).
    * "lt": the < operation.
\code
           function lt_event (op1, op2)
             if type(op1) == "number" and type(op2) == "number" then
               return op1 < op2   -- numeric comparison
             elseif type(op1) == "string" and type(op2) == "string" then
               return op1 < op2   -- lexicographic comparison
             else
               local h = getcomphandler(op1, op2, "__lt")
               if h then
                 return (h(op1, op2))
               else
                 error();
               end
             end
           end
\endcode
      a > b is equivalent to b < a.
    * "le": the <= operation.
\code
           function le_event (op1, op2)
             if type(op1) == "number" and type(op2) == "number" then
               return op1 <= op2   -- numeric comparison
             elseif type(op1) == "string" and type(op2) == "string" then
               return op1 <= op2   -- lexicographic comparison
             else
               local h = getcomphandler(op1, op2, "__le")
               if h then
                 return (h(op1, op2))
               else
                 h = getcomphandler(op1, op2, "__lt")
                 if h then
                   return not h(op2, op1)
                 else
                   error();
                 end
               end
             end
           end
\endcode
      a >= b is equivalent to b <= a. Note that, in the absence of a "le"
		metamethod, Lua tries the "lt", assuming that a <= b is equivalent to not (b < a).
    * "index": The indexing access table[key].
\code
           function gettable_event (table, key)
             local h
             if type(table) == "table" then
               local v = rawget(table, key)
               if v ~= nil then return v end
               h = metatable(table).__index
               if h == nil then return nil end
             else
               h = metatable(table).__index
               if h == nil then
                 error();
               end
             end
             if type(h) == "function" then
               return (h(table, key))     -- call the handler
             else return h[key]           -- or repeat operation on it
             end
           end
\endcode
    * "newindex": The indexing assignment table[key] = value.
\code
           function settable_event (table, key, value)
             local h
             if type(table) == "table" then
               local v = rawget(table, key)
               if v ~= nil then rawset(table, key, value); return end
               h = metatable(table).__newindex
               if h == nil then rawset(table, key, value); return end
             else
               h = metatable(table).__newindex
               if h == nil then
                 error();
               end
             end
             if type(h) == "function" then
               h(table, key,value)           -- call the handler
             else h[key] = value             -- or repeat operation on it
             end
           end
\endcode
    * "call": called when Lua calls a value.
\code
           function function_event (func, ...)
             if type(func) == "function" then
               return func(...)   -- primitive call
             else
               local h = metatable(func).__call
               if h then
                 return h(func, ...)
               else
                 error()
               end
             end
           end
\endcode
*/
#	include "lua_includes.h"
#	include "fwd_push_pull.h"
#	include "param_traits.h"
#   include "push_pointer_internal.h"
#   include "type_list.h"
#	include "oolua_userdata.h"
#	include "oolua_storage.h"

namespace OOLUA
{
	//rhs is top of stack lhs is below (lhs op rhs)
	template<typename T>
	int lua_equal(lua_State*  const l)
	{
		T const* lhs(0);
 		T const* rhs(0);
		INTERNAL::LUA_CALLED::pull2cpp(l,rhs);
    	INTERNAL::LUA_CALLED::pull2cpp(l,lhs);
		bool result (*lhs == *rhs);
		push2lua(l,result);
		return 1;
	}

 	template<typename T>
	int lua_less_than(lua_State*  const l)
	{
		T const* lhs(0);
		T const* rhs(0);
		INTERNAL::LUA_CALLED::pull2cpp(l,rhs);
		INTERNAL::LUA_CALLED::pull2cpp(l,lhs);
		bool result (*lhs < *rhs);
		push2lua(l,result);
		return 1;
	}

 	template<typename T>
	int lua_less_than_or_equal(lua_State*  const l)
	{
		T const* lhs(0);
		T const* rhs(0);
		INTERNAL::LUA_CALLED::pull2cpp(l,rhs);
		INTERNAL::LUA_CALLED::pull2cpp(l,lhs);
		bool result (*lhs <= *rhs);
		push2lua(l,result);
		return 1;
	}

	//these following operator functions return a type that they are working on
 	template<typename T>
	int lua_add(lua_State*  const l)
	{
		T const* lhs(0);
		T const* rhs(0);
		INTERNAL::LUA_CALLED::pull2cpp(l,rhs);
		INTERNAL::LUA_CALLED::pull2cpp(l,lhs);
    	T* result ( new T( *lhs + *rhs ) );
		OOLUA::INTERNAL::Lua_ud* ud = INTERNAL::add_ptr<T>(l,result,false);
		ud->gc = true;
    	return 1;
	}


	template<typename T>
	int lua_sub(lua_State*  const l)
	{
		T const* lhs(0);
		T const* rhs(0);
		INTERNAL::LUA_CALLED::pull2cpp(l,rhs);
		INTERNAL::LUA_CALLED::pull2cpp(l,lhs);
		T* result ( new T( *lhs - *rhs ) );
		OOLUA::INTERNAL::Lua_ud* ud = INTERNAL::add_ptr<T>(l,result,false);
		ud->gc = true;
		return 1;
	}

	template<typename T>
	int lua_mul(lua_State*  const l)
	{
		T const* lhs(0);
		T const* rhs(0);
		INTERNAL::LUA_CALLED::pull2cpp(l,rhs);
		INTERNAL::LUA_CALLED::pull2cpp(l,lhs);
		T* result ( new T( *lhs * *rhs ) );
		OOLUA::INTERNAL::Lua_ud* ud = INTERNAL::add_ptr<T>(l,result,false);
		ud->gc = true;
		return 1;
	}

	template<typename T>
	int lua_div(lua_State*  const l)
	{
		T const* lhs(0);
		T const* rhs(0);
		INTERNAL::LUA_CALLED::pull2cpp(l,rhs);
		INTERNAL::LUA_CALLED::pull2cpp(l,lhs);
		T* result ( new T( *lhs / *rhs ) );
		OOLUA::INTERNAL::Lua_ud* ud = INTERNAL::add_ptr<T>(l,result,false);
		ud->gc = true;
		return 1;
	}

#define DEFINE_OOLUA_OPERATOR_FUNCTION_(operation,lua_string_op)\
template<typename T, bool hasOperator >\
struct set_ ## operation ## _function\
{\
 	   static void set(lua_State*  const /*l*/,int /*const_metatable*/,int /*none_const_metatable*/){}\
};\
template<typename T>\
struct set_ ## operation ## _function<T, true> \
{\
 	   static void set(lua_State*  const l,int const_metatable,int none_const_metatable)\
 	   {\
			  lua_pushcfunction(l, lua_## operation<T>);\
			  int func = lua_gettop(l);\
			  lua_pushliteral(l, lua_string_op);\
			  int op_string = lua_gettop(l);\
			  lua_pushvalue(l,op_string);\
			  lua_pushvalue(l,func);\
			  lua_settable(l, const_metatable);\
			  lua_pushvalue(l,op_string);\
			  lua_pushvalue(l,func);\
			  lua_settable(l, none_const_metatable);\
			  lua_remove(l,op_string);\
			  lua_remove(l,func);\
       }\
};

DEFINE_OOLUA_OPERATOR_FUNCTION_(equal,"__eq")
DEFINE_OOLUA_OPERATOR_FUNCTION_(less_than,"__lt")
DEFINE_OOLUA_OPERATOR_FUNCTION_(less_than_or_equal,"__le")
DEFINE_OOLUA_OPERATOR_FUNCTION_(add,"__add")
DEFINE_OOLUA_OPERATOR_FUNCTION_(sub,"__sub")
DEFINE_OOLUA_OPERATOR_FUNCTION_(mul,"__mul")
DEFINE_OOLUA_OPERATOR_FUNCTION_(div,"__div")

#undef DEFINE_OOLUA_OPERATOR_FUNCTION_

template<typename T,typename TyDef>
struct has_typedef
{
	enum {Result = TYPELIST::IndexOf<typename T::Typedef,TyDef>::value == -1 ? 0 : 1};
};

}

#endif //LUA_OPERATOR_H_
