#include <assert.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stddef.h>
#include <stdio.h>

#include "config.h"
#include "api.h"
#define CEXAMPLE_OK 0
#define CEXAMPLE_ERR -1

enum {
    CE_LUA_READER_BUFSIZE = 4096,
};


typedef struct {
    FILE       *f;
    char        buff[CE_LUA_READER_BUFSIZE];

} ce_lua_clfactory_file_ctx_t;

int ce_lua_panic_handler(lua_State *L) {

#ifdef CE_LUA_ABORT_AT_PANIC
    abort();
#else

    const char                  *s = NULL;
    size_t                len = 0;

    if (lua_type(L, -1) == LUA_TSTRING) {
        s = lua_tolstring(L, -1, &len);
    }

    if (s == NULL) {
        s = "unknown reason";
        len = sizeof("unknown reason") - 1;
    }

    fprintf(stderr, "lua panic: Lua VM crashed %*s", (int)len , s);
    return 0;
#endif
}

int ce_report_lua_error(lua_State  *L) {
    const char *err;
    if ((err = luaL_checkstring(L, -1)) == NULL) {
        return CEXAMPLE_ERR;
    }

    printf("lua error: %s\n", err);
    return CEXAMPLE_OK;
}


int ce_register_module(lua_State *L, const char *module_name, const luaL_Reg *l) {
    //just in lua5.1
    luaL_register(L, module_name, l);
    return CEXAMPLE_OK;
}

int ce_register_function(lua_State *L, const char *fun_name, lua_CFunction fun) {
    lua_register(L, fun_name, fun);
    return CEXAMPLE_OK;
}

static const char *
ce_lua_clfactory_get_file(lua_State *L, void *ud, size_t *size) {
    ce_lua_clfactory_file_ctx_t *lf;
    size_t n;

    lf = (ce_lua_clfactory_file_ctx_t *)ud;

    n = fread(lf->buff, 1, sizeof(lf->buff), lf->f);

    if (n == 0) {
        *size = 0;
        return NULL;
    }

    *size = n;
    return lf->buff;
}


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

int ce_init_lua_vm(lua_State **vm) {
    lua_State *L;
    L = ce_new_lua_stat();

    if (L == NULL) {
        return CEXAMPLE_ERR;
    }

    lua_atpanic(L, ce_lua_panic_handler);

#if HAVE_LUAJIT
    luaopen_ffi(L);
#endif

    if (ce_register_module(L, "cmodule", api_module) != CEXAMPLE_OK) {
        return CEXAMPLE_ERR;
    }

    if (ce_register_function(L, "add", add) != CEXAMPLE_OK) {
        return CEXAMPLE_ERR;
    }
    *vm = L;
    return CEXAMPLE_OK;
}

int ce_load_lua_script(lua_State *vm, const char *filename) {
    ce_lua_clfactory_file_ctx_t lf;
    int rc;

    lf.f = fopen(filename, "r");
    if (lf.f == NULL) {
        return CEXAMPLE_ERR;
    }

    rc = lua_load(vm, ce_lua_clfactory_get_file, &lf, filename);
    fclose(lf.f);
    if (rc != 0) {
        return CEXAMPLE_ERR;
    }

    return CEXAMPLE_OK;
}

int ce_call_lua_code(lua_State *vm) {
    assert(lua_isfunction(vm, -1));
    
    if (lua_resume(vm, 0) != LUA_OK) {
        return CEXAMPLE_ERR;
    }

    return CEXAMPLE_OK;
}


int ce_read_lua_result(lua_State *vm) {
    int age;
    const char *name;
    //can't get this var. because age is local var.
    lua_getglobal(vm, "age");
    age = lua_tointeger(vm, -1);

    lua_getglobal(vm, "Name");
    name = lua_tostring(vm, -1);
    
    printf("name %s age is %d\n", name, age);
    return CEXAMPLE_OK;
}

int ce_process_lua(const char *file) {
    lua_State   *L;

    if (ce_init_lua_vm(&L) != CEXAMPLE_OK) {
        ce_report_lua_error(L);
        return CEXAMPLE_ERR;
    }

    if (ce_load_lua_script(L, "main.lua") != CEXAMPLE_OK) {
        ce_report_lua_error(L);
        return CEXAMPLE_ERR;
    }

    if (ce_call_lua_code(L) != CEXAMPLE_OK) {
        ce_report_lua_error(L);
        return CEXAMPLE_ERR;
    }

    if (ce_read_lua_result(L) != CEXAMPLE_OK) {
        ce_report_lua_error(L);
        return CEXAMPLE_ERR;
    }

    return CEXAMPLE_OK;
}

int	main(int argc, char **argv) {
    return ce_process_lua("main.lua");
}
