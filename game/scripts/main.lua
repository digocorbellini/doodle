Engine.testFunct();

Test = require("test");

Test.TestCall();

local function testFunct2()
    local testStr = "test2";
    print(testStr .. " funct");
end

local function testFunct()
    local testStr = "test";
    print(testStr .. " funct");
    testFunct2();
end

Value = 0;
function Frame() 
    local test = 0;
    test = test + 5;
    Value = Value + 1;
    if Value > 100 then
        print("called from lua frame");
        Value = 0;
        testFunct();
        Test.TestCall();
    end 
end

print("Calling from Lua!!!!");

