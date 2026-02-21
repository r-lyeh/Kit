local hurdy = require("hurdyl")

-- modify hurdy.path if you want to look for hurdy files in different places
-- (see the documentation for love.filesystem.setRequirePath)
--hurdy.path = "?.hurdy;?/init.hurdy"

local searchpath = function(name, path)
    local paths = {""}
    name = name:gsub("%.", "/")
    for p in path:gmatch("[^;]+") do
        local fPath = p:gsub("?", name)
        return fPath --if love.filesystem.getInfo(fPath, "file") then return fPath end
        --paths[#paths + 1] = string.format("\tno file '%s'", fPath)
    end
    return nil, table.concat(paths, "\n")
end

local hurdySearcher = function(name)
    local filename, err = searchpath(name, hurdy.path)
    if filename then
        local data = assert(io.open(filename, "r")):read("*a") --love.filesystem.read(filename)
        return hurdy.loadstring(data, "@" .. filename)
    else
        return err
    end
end


local searchers = package.loaders or package.searchers
table.insert(searchers, 1, hurdySearcher)

x = require('test')
