local p = premake

if not p.modules.build then
    p.modules.build = {}

    newaction {
        trigger = "build",
        description = "Builds the project using whatever toolset is chosen",
        onProject = function(prj)
            printf("Building project '%s'", prj.name)
            
            local res,msg,sig;
            
            if (os.host() == "windows") then
                res,msg,sig = os.execute("msbuild "..prj.location.."\\"..prj.name..".vcxproj -t:Build -verbosity:minimal -p:Configuration=".._OPTIONS["config"].." -p:Platform=x64")
            elseif (os.host() == "linux") then
                res,msg,sig = os.execute("make -j3 "..prj.name.." config=".._OPTIONS["config"])
            end
            
            if (not res and msg == "exit") then
                error("Build "..msg.." with code: "..sig, 0)
            end
        end
    }
end

return p.modules.build