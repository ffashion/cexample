-- module ("t", package)
-- package.path = ./

local cm = require("cmodule")

-- lua_module.lua 返回的结果交给luam 
local luam = require("lua_module")

-- module 
local age = 10
Name = "ffashion"
print("Hello, World!")

print(cm.add(1, 2))

print (add(3, 4))


print("lua module result is" .. " " .. luam.age)

-- t.a = 2;

-- print("t.a is" .. " " .. t.a)

-- print("t._NAME is " .. t._NAME)