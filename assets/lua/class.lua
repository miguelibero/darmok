class = require("darmok/middleclass")

local function configureSubclasses(cls, func)
	func(cls)
	local oldSubclass = cls.static.subclass
	cls.static.subclass = function(self, name)
		local sub = oldSubclass(self, name)
		func(sub, cls)
		return sub
	end
end

local function overwriteMetatableIndex(mt, func)
	local old = mt.__index
	local oldFunc = old or rawget
	if type(old) == "table" then
		oldFunc = function(self, k) return old[k] end
	end
	mt.__index = function(self, k)
		return func(self, k, oldFunc)
	end
end

local function overwriteMetatableNewIndex(mt, func)
	local oldFunc = mt.__newindex or rawset
	mt.__newindex = function(self, k, v)
		func(self, k, v, oldFunc)
	end
end

-- ClassProperties
-- middleclass mixin to add getter-setter properties
-- prefix methods with `_get_` and `_set_` to access them as properties
ClassProperties = {}

function ClassProperties._configure(cls)
	local mt = cls.__instanceDict
	overwriteMetatableNewIndex(mt, function(self, k, v, old)
		local func = mt["_set_" .. k]
		if func ~= nil then
			assert(type(func) == 'function', "setter must be a function")
			return func(self, v)
		end
		return old(self, k, v)
	end)
	overwriteMetatableIndex(mt, function(self, k, old)
		local func = mt["_get_" .. k]
		if func ~= nil then
			assert(type(func) == 'function', "getter must be a function")
			return func(self)
		end
        return old(self, k)
	end)
end

function ClassProperties:included(cls)
	configureSubclasses(cls, ClassProperties._configure)
end


-- ClassStatic
-- middleclass mixin to add static methods where the first parameter
-- is the class that was called, these methods sould be added
-- to Class.static._class_{name}
ClassStatic = {}

function ClassStatic._configure(cls)
	local mt = getmetatable(cls.static)
	overwriteMetatableIndex(mt, function(self, k, old)
		local func = old(self, "_class_" .. k)
		if func ~= nil then
			assert(type(func) == 'function', "class static member must be a function")
			return function(...) return func(cls, ...) end
		end
		return old(self, k)
	end)
end

function ClassStatic:included(cls)
	configureSubclasses(cls, ClassStatic._configure)
end