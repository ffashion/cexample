#include "api.h"
#include "lua.h"


luaL_Reg api_module[] = {
    {
    .name = "add",
    .func = add,
    },
    {
    .name = "mul",
    .func = mul,
    },
    {
    .name = NULL,
    .func = NULL,
    },
};

int add(lua_State *L) {
    int a, b;
    a = lua_tointeger(L, -1);
    lua_pop(L, 1);

    b = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_pushinteger(L, a + b);
    return 1;
}

int mul(lua_State *L) {
    int a, b;
    a = lua_tointeger(L, -1);
    lua_pop(L, 1);
    b = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_pushinteger(L, a * b);
    return 1;
}

