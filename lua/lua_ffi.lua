local ffi = require("ffi")

ffi.cdef[[ int printf(const char *fmt, ...); ]]


ffi.C.printf("Hello lua_ffi\n")

