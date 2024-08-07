Class = {}

function Class:new(super, name)
    local class = { name = name }
    local static = {}
    class.super = super
    class.static = static
    local static_metatable = {}
    class.static_metatable = static_metatable
    setmetatable(class, static_metatable)

    function class.static:subclass_of(otherClass)
        if self.super == otherClass then
            return true
        end
        if self.super then
            return self.super:subclass_of(otherClass)
        end
        return false
    end

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

    function static_metatable:__tostring()
        if static.tostring then
            return static.tostring(self)
        end
        return class.name or "Class"
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
        if super and super.metatable then
            return super.metatable.__newindex(self, key, value)
        end
        rawset(self, key, value)
    end

    function metatable:__tostring()
        if class.tostring then
            return class.tostring(self)
        end
        local name = static_metatable.__tostring(class)
        return name .. "(" .. table.tostring(self) .. ")"
    end

    function class:alloc()
        local data = {
            class = class
        }
        while data.class.class do
            data.class = data.class.class
        end
        return setmetatable(data, self.metatable)
    end

    function class:new(...)
        local obj = self:alloc()
        if obj then
            obj:init(...)
        end
        return obj
    end

    function class:init()
    end

    function class:instance_of(otherClass)
        return self.class:subclass_of(otherClass)
    end

    function class:extend(name)
        return Class:new(self, name)
    end

    return class
end