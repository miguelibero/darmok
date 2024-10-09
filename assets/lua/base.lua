
class = require("darmok/middleclass")

-- ClassProperties
-- middleclass mixin to add getter-setter properties
-- prefix methods with `_get_` and `_set_` to access them as properties
ClassProperties = {}

function ClassProperties:__index(k)
    local getter = self.class.__instanceDict["_get_" .. k]
    if getter ~= nil then
        return getter(self)
    end
end

function ClassProperties:__newindex(k, v)
    local setter = self.class.__instanceDict["_set_" .. k]
    if setter ~= nil then
        setter(self, v)
    else
        rawset(self, k, v)
    end
end

-- ClassStatic
-- middleclass mixin to add static methods where the first parameter
-- is the class that was called, these methods sould be added
-- to Class.static._class_{name}
ClassStatic = {}

function ClassStatic:included(aClass)
	local function configure(cls)
		local mt = getmetatable(cls)
		local oldIndex = mt.__index
		mt.__index = function(self, k)
		    local func = cls.static["_class_" .. k]
		    if func ~= nil then
				assert(type(func) == 'function', "class static member must be a function")
				return function(...) return func(cls, ...) end
			end
			return oldIndex[k]
		end
	end

	configure(aClass)
	local oldSubclass = aClass.static.subclass
	aClass.static.subclass = function(self, name)
		local cls = oldSubclass(self, name)
		configure(cls)
		return cls
	end
end


-- App
function App:add_component(cls, ...)
	return cls.add_app_component(self, ...)
end

function App:get_component(cls)
	return cls.get_app_component(self)
end

function App:get_or_add_component(cls, ...)
	local comp = self:get_component(cls)
	if comp then
		return comp
	end
	return self:add_component(cls, ...)
end


-- AppComponent
AppComponent = class("AppComponent"):include(ClassStatic)

function AppComponent.static:_class_add_app_component(app, ...)
	local comp = self:allocate()
	if comp then
		app:add_lua_component(comp)
		comp:initialize(...)
	end
    return comp
end

function AppComponent.static:_class_get_app_component(app)
    return app:get_lua_component(self)
end


-- Scene
function Scene:get_entity(comp)
	return comp:get_entity(self)
end

function Scene:add_component(cls, ...)
	return cls.add_scene_component(self, ...)
end

function Scene:get_component(cls)
	return cls.get_scene_component(self)
end

function Scene:get_or_add_component(cls, ...)
	local comp = self:get_component(cls)
	if comp then
		return comp
	end
	return self:add_component(cls, ...)
end

function Scene:find_entity_component(cls)
	local comp = nil
	self:for_each_entity(function(entity)
		comp = entity:get_component(cls)
		return comp
	end)
	return comp
end

function Scene:find_or_add_entity_component(cls, ...)
	local comp = self:find_entity_component(cls)
	if comp then
		return comp
	end
	local entity = self:create_entity()
	return entity:add_component(cls, ...)
end

function Scene:find_entity_components(cls)
	local comps = {}
	self:for_each_entity(function(entity)
		local comp = entity:get_component(cls)
		if comp then
			table.insert(comps, comp)
		end
	end)
	return comps
end


-- SceneComponent
SceneComponent = class("SceneComponent"):include(ClassStatic)

function SceneComponent.static:_class_add_scene_component(scene, ...)
	local comp = self:allocate()
	comp.scene = scene
	if comp then
    	scene:add_lua_component(comp)
		comp:initialize(...)
	end
    return comp
end

function SceneComponent.static:_class_get_scene_component(scene)
    return scene:get_lua_component(self)
end


-- EntityComponent
EntityComponent = class("EntityComponent"):include(ClassStatic)

function EntityComponent.static:_class_add_entity_component(entity, ...)
	local comp = self:allocate()
	comp.entity = entity
	if comp then
    	entity:add_lua_component(comp)
		comp:initialize(...)
	end
    return comp
end

function EntityComponent.static:_class_get_entity_component(entity)
    return entity:get_lua_component(self)
end


-- Entity
function Entity:add_component(cls, ...)
	return cls.add_entity_component(self, ...)
end

function Entity:get_component(cls)
	return cls.get_entity_component(self)
end

function Entity:get_or_add_component(cls, ...)
	local comp = self:get_component(cls)
	if comp then
		return comp
	end
	return self:add_component(cls, ...)
end

function Entity:get_component_in_children(cls)
	local comp = nil
	self:for_each_child(function(entity)
		comp = entity:get_component(cls)
		return comp
	end)
	return comp
end

function Entity:get_components_in_children(cls)
	local comps = {}
	self:for_each_child(function(entity)
		local comp = entity:get_component(cls)
		if comp then
			table.insert(comps, comp)
		end
	end)
	return comps
end

function Entity:get_component_in_parent(cls)
	local comp = nil
	self:for_each_parent(function(entity)
		comp = entity:get_component(cls)
		return comp
	end)
	return comp
end

function Entity:is_child(entity)
	if not entity then
		return false
	end
	return self:for_each_parent(function(parent)
		return parent == entity
	end)
end

function Entity:is_parent(entity)
	if not entity then
		return false
	end
	return entity:is_child(self)
end

function Entity:is_child_or_self(entity)
	if self == entity then
		return true
	end
	return self:is_child(entity)
end


function Entity:is_parent_or_self(entity)
	if not entity then
		return false
	end
	return entity:is_child_or_self(self)
end

-- Camera
function Camera:add_component(cls, ...)
	return cls.add_camera_component(self, ...)
end

function Camera:get_component(cls)
	return cls.get_camera_component(self)
end

function Camera:get_or_add_component(cls, ...)
	local comp = self:get_component(cls)
	if comp then
		return comp
	end
	return self:add_component(cls, ...)
end

-- RenderChain
function RenderChain:add_step(cls, ...)
	return cls.add_chain_step(self, ...)
end