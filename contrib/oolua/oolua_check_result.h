#ifndef OOLUA_CHECK_RESULT_H_
#   define OOLUA_CHECK_RESULT_H_

struct lua_State;

namespace OOLUA
{
    namespace INTERNAL
    {
        bool protected_call_check_result(lua_State* l,int pcall_result);
        bool load_buffer_check_result(lua_State* l,int result);
    }
}

#endif
