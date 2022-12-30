local p = premake

if not p.modules.test then
    p.modules.test = {}

    newaction {
        trigger = "test",
        description = "Runs tests using the given config",
        execute = function()
            print("Testing")
            
            local res,msg,sig;
            
            if (os.host() == "windows") then
                res,msg,sig = os.execute("bin/".._OPTIONS["config"].."-windows-x86_64/Test.exe")
            else
                res,msg,sig = os.execute("bin/".._OPTIONS["config"].."-linux-x86_64/Test")
            end
            
            if (not res and msg == "exit") then
                error("Build "..msg.." with code: "..sig, 0)
            end
        end
    }
end

return p.modules.test