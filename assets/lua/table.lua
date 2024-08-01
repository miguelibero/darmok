function table.contains(table, element)
    for _, value in pairs(table) do
        if value == element then
            return true
        end
    end
    return false
end

function table.copy_shallow(table)
    local u = {}
    for k, v in pairs(table) do
        u[k] = v
    end
    return setmetatable(u, getmetatable(t))
end

function table.tostring(val, name, skipnewlines, depth)
    skipnewlines = skipnewlines or false
    depth = depth or 0
    local indent = string.rep(" ", depth * 2)
    local tmp = indent
    if name then tmp = tmp .. name .. " = " end
    local t = type(val)
    if t == "table" then
        tmp = tmp .. "{" .. (not skipnewlines and "\n" or "")
        for k, v in pairs(val) do
            tmp =  tmp .. table.tostring(v, k, skipnewlines, depth + 1)
                .. "," .. (not skipnewlines and "\n" or "")
        end
        tmp = tmp .. indent .. "}"
    elseif t == "number" then
        tmp = tmp .. tostring(val)
    elseif t == "string" then
        tmp = tmp .. string.format("%q", val)
    elseif t == "boolean" then
        tmp = tmp .. tostring(val)
    else
        tmp = tmp .. "\"[type:" .. t .. "]\""
    end

    return tmp
end
