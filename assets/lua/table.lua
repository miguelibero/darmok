function table.contains(table, element)
    for _, value in pairs(table) do
        if value == element then
            return true
        end
    end
    return false
end

function table.copy(t)
    local u = { }
    for k, v in pairs(t) do u[k] = v end
    return setmetatable(u, getmetatable(t))
end