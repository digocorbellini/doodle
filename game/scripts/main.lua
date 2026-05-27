Engine.testFunct();

local test = require("test");

test.testCall();

function testFunct()
    print("test funct");
end

local value = 0;
function Frame() 
    local test = 0;
    test = test + 5;
    value = value + 1;
    if value > 100000 then
        print("called from lua frame");
        value = 0;
        testFunct();
    end 
end

print("Calling from Lua!!!!");

