#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


#define CEXAMPLE_OK 0
#define CEXAMPLE_ERR -1

lua_State *ce_new_lua_stat() {
    lua_State *L;

    L = luaL_newstate();
    if (L == NULL) {
        return NULL;
    }

    luaL_openlibs(L);

    lua_getglobal(L, "package");

    if (!lua_istable(L, -1)) {

        return NULL;
    }

    return L;
}

int ce_init_lua_vm(lua_State **new_vm) {
    lua_State *L;
    L = ce_new_lua_stat();

    if (L == NULL) {
        return CEXAMPLE_ERR;
    }


    return CEXAMPLE_OK;
}
int	main(int argc, char **argv) {
    lua_State                       *L;
    
    if (ce_init_lua_vm(&L) != CEXAMPLE_OK) {
        return CEXAMPLE_ERR;
    }


    return 0;
}
