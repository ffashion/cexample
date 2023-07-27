local function thing1(x)
    print("thing1", x);
    -- return to main corouting, and main corouting will get 'ret_thing1'
    return coroutine.yield("ret_thing1");
end

local function thing2(x) 
    print("thing2", x);
    return coroutine.yield("ret_thing2");
end


local function thing3(x) 
    print("thing3", x);
    return coroutine.yield("ret_thing3");
end

local function corouting_handler(x)
    x = thing1(x);
    -- get parm x from yield return value
    x = thing2(x);
    x = thing3(x);
    return "end";
end


local co = coroutine.create(corouting_handler)

local i = 0;
while 1 do
    -- only the first parm will be pass to corouting
    print("main", coroutine.resume(co, i))
    i = i + 1;
    if (coroutine.status(co) == 'dead') then
        break
    end
    
end


