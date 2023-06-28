#ifndef __API__H__
#define __API__H__

#include <assert.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stddef.h>
#include <stdio.h>
#include "config.h"

int add(lua_State *L);
int mul(lua_State *L);

extern luaL_Reg api_module[];
#endif  /*__API__H__*/
