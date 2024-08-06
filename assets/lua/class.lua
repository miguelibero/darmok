Class = {}

function Class:new(super)
    local class = {}
    local static = {}
    class.static = static
    local static_metatable = {}
    class.static_metatable = static_metatable
    setmetatable(class, static_metatable)

    function static_metatable:__index(key)
        local val = static[key]
        if val then
            return function(...)
                return val(self, ...)
            end
        end
        if super and super.static_metatable then
            return super.static_metatable.__index(self, key)
        end
    end

    local props = {}
    local metatable = {}
    class.properties = props
    class.metatable = metatable

    function metatable:__index(key)
        local prop = props[key]
        if prop then
            return prop.get(self)
        end
        local val = class[key]
        if val ~= nil then
            return val
        end
        if super and super.metatable then
            return super.metatable.__index(self, key)
        end
    end

    function metatable:__newindex(key, value)
        local prop = props[key]
        if prop then
            return prop.set(self, value)
        end
        if super then
            return super.metatable.__newindex(self, key, value)
        end
        rawset(self, key, value)
    end

    function class:alloc(data, ...)
        if type(data) ~= "table" then
            data = {}
        end
        if not data.class then
            data.class = class
        end
        local obj = setmetatable(data, self.metatable)
        if obj.__new then
            obj:__new(...)
        end
        return obj
    end

    function class:new(...)
        return self:alloc({}, ...)
    end

    function class:extend()
        return Class:new(self)
    end

    return class
end