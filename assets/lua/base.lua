
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
AppComponent = Class:new()

function AppComponent.static.add_app_component(class, app, ...)
	local comp = class:new(...)
    app:add_lua_component(comp)
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


-- SceneComponent
SceneComponent = Class:new()

function SceneComponent.static.add_scene_component(class, scene, ...)
	local comp = class:new(...)
    scene:add_lua_component(comp)
    return comp
end

function SceneComponent.static.get_scene_component(class, scene)
    return scene:get_lua_component(class)
end


-- EntityComponent
EntityComponent = Class:new()

function EntityComponent.static.add_entity_component(class, entity, ...)
	local comp = class:alloc({
		entity = entity
	}, ...)
    entity:add_lua_component(comp)
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


-- Camera
function Camera:add_renderer(type, ...)
	return type.add_renderer(self, ...)
end


-- ForwardRenderer
function ForwardRenderer:add_component(type, ...)
	return type.add_render_component(self, ...)
end