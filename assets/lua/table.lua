function table.contains(tab, element)
    for _, v in pairs(tab) do
        if v == element then
            return true
        end
    end
    return false
end

function table.shallowcopy(tab)
    local copy = {}
    for k, v in pairs(tab) do
        copy[k] = v
    end
    return setmetatable(copy, getmetatable(tab))
end

function table.deepcopy(val)
    local copy
    if type(val) == 'table' then
        copy = {}
        for k, v in pairs(tab) do
            copy[table.deepcopy(k)] = table.deepcopy(v)
        end
        setmetatable(copy, getmetatable(val))
    else
        copy = val
    end
    return copy
end

function table.merge(tab1, tab2)
    for k, v in pairs(tab2) do
        if type(v) == "table" then
            if type(tab1[k] or false) == "table" then
                table.merge(tab1[k] or {}, tab2[k] or {})
            else
                tab1[k] = v
            end
        else
            tab1[k] = v
        end
    end
    return tab1
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
