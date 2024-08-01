
-- App
function App:add_component(type, ...)
	return type.add_app_component(self, ...)
end
function App:get_shared_component(type)
	return type.get_shared_component(self)
end

-- Component
Component = Class:new()
function Component.static.add_entity_component(class, entity, ...)
    local comp = class:new(...)
    entity:add_lua_component(comp)
    return comp
end

function Component.static.get_entity_component(class, entity)
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
	if comp ~= nil then
		return comp
	end
	return self:add_component(type, ...)
end

-- Scene
function Scene:get_entity(comp)
	return comp:get_entity(self)
end
function Scene:add_component(type, ...)
	return type.add_scene_component(self, ...)
end

-- Camera
function Camera:add_renderer(type, ...)
	return type.add_renderer(self, ...)
end

-- ForwardRenderer
function ForwardRenderer:add_component(type, ...)
	return type.add_render_component(self, ...)
end