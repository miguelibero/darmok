
-- App
function App:add_component(type, ...)
	return type.add_app_component(self, ...)
end

function App:get_component(type)
	return type.get_app_component(self)
end

function App:get_or_add_component(type, ...)
	local comp = self:get_component(type)
	if comp then
		return comp
	end
	return self:add_component(type, ...)
end


-- AppComponent
AppComponent = Class:new("AppComponent")

function AppComponent.static.add_app_component(class, app, ...)
	local comp = class:alloc()
	if comp then
		app:add_lua_component(comp)
		comp:init(...)
	end
    return comp
end

function AppComponent.static.get_app_component(class, app)
    return app:get_lua_component(class)
end


-- Scene
function Scene:get_entity(comp)
	return comp:get_entity(self)
end

function Scene:add_component(type, ...)
	return type.add_scene_component(self, ...)
end

function Scene:get_component(type)
	return type.get_scene_component(self)
end

function Scene:get_or_add_component(type, ...)
	local comp = self:get_component(type)
	if comp then
		return comp
	end
	return self:add_component(type, ...)
end

function Scene:find_entity_component(type)
	local comp = nil
	self:for_each_entity(function(entity)
		comp = entity:get_component(type)
		return comp
	end)
	return comp
end

function Scene:find_or_add_entity_component(type, ...)
	local comp = self:find_entity_component(type)
	if comp then
		return comp
	end
	local entity = self:create_entity()
	return entity:add_component(type, ...)
end

function Scene:find_entity_components(type)
	local comps = {}
	self:for_each_entity(function(entity)
		local comp = entity:get_component(type)
		if comp then
			table.insert(comps, comp)
		end
	end)
	return comps
end

-- SceneComponent
SceneComponent = Class:new("SceneComponent")

function SceneComponent.static.add_scene_component(class, scene, ...)
	local comp = class:alloc()
	comp.scene = scene
	if comp then
    	scene:add_lua_component(comp)
		comp:init(...)
	end
    return comp
end

function SceneComponent.static.get_scene_component(class, scene)
    return scene:get_lua_component(class)
end


-- EntityComponent
EntityComponent = Class:new("EntityComponent")

function EntityComponent.static.add_entity_component(class, entity, ...)
	local comp = class:alloc()
	comp.entity = entity
	if comp then
    	entity:add_lua_component(comp)
		comp:init(...)
	end
    return comp
end

function EntityComponent.static.get_entity_component(class, entity)
    return entity:get_lua_component(class)
end


-- Entity
function Entity:add_component(type, ...)
	return type.add_entity_component(self, ...)
end

function Entity:get_component(type)
	return type.get_entity_component(self)
end

function Entity:get_or_add_component(type, ...)
	local comp = self:get_component(type)
	if comp then
		return comp
	end
	return self:add_component(type, ...)
end

function Entity:get_component_in_children(type)
	local comp = nil
	self:for_each_child(function(entity)
		comp = entity:get_component(type)
		return comp
	end)
	return comp
end

function Entity:get_components_in_children(type)
	local comps = {}
	self:for_each_child(function(entity)
		local comp = entity:get_component(type)
		if comp then
			table.insert(comps, comp)
		end
	end)
	return comps
end

function Entity:get_component_in_parent(type)
	local comp = nil
	self:for_each_parent(function(entity)
		comp = entity:get_component(type)
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
function Camera:add_component(type, ...)
	return type.add_camera_component(self, ...)
end

function Camera:get_component(type)
	return type.get_camera_component(self)
end

function Camera:get_or_add_component(type, ...)
	local comp = self:get_component(type)
	if comp then
		return comp
	end
	return self:add_component(type, ...)
end

-- RenderChain
function RenderChain:add_step(type, ...)
	return type.add_chain_step(self, ...)
end

