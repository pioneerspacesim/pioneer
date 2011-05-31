#ifndef OOLUA_USERDATA_H_
#	define OOLUA_USERDATA_H_

#include "oolua_config.h"
#include <cstring>
namespace OOLUA
{
    namespace INTERNAL
    {
        struct Lua_ud
        {
			void* void_class_ptr;
			char* name;
			char* none_const_name;//none constant name of the class
			int name_size;//size of name
            bool gc;//should it be garbage collected
        };
		
		
		inline bool id_is_const(Lua_ud* ud)
		{
			return ud->name != ud->none_const_name;
		}

#if OOLUA_SAFE_ID_COMPARE == 1
		inline bool ids_equal(char* lhsName,int lhsSize,char* rhsName,int rhsSize)
		{
			if(lhsSize != rhsSize)return false;
			return memcmp(lhsName,rhsName,lhsSize) == 0;
		}
#else
		inline bool ids_equal(char* lhsName,int /*lhsSize*/,char* rhsName,int /*rhsSize*/)
		{
			return lhsName == rhsName;
		}		
#endif
		
    }
}
#endif
