
local program = app.assets:load_standard_program(StandardProgramType.ForwardPhong)
local meshCreator = MeshCreator.new(program.vertex_layout)

local cubeMesh = meshCreator:create_cube()
local greenTex = app.assets:load_color_texture(Color.green)
cubeMesh.material = Material.new(greenTex)

local camEntity = app.scene:create_entity()
local camTrans = camEntity:add_component(ComponentType.Transform)
camTrans.position = { 0, 2, -2 }
camTrans:look_at({ 0, 0, 0 })
local cam = camEntity:add_component(ComponentType.Camera)
cam:set_projection(60, app.window.size, { 0.3, 1000 })
cam:set_forward_renderer(program)
cam:add_component(CameraComponentType.PhongLighting)

local lightEntity = app.scene:create_entity()
local lightTrans = lightEntity:add_component(ComponentType.Transform)
lightTrans.position = { 1, 1, -2 }
lightEntity:add_component(ComponentType.PointLight)

local meshEntity = app.scene:create_entity()
local meshTrans = meshEntity:add_component(ComponentType.Transform)
meshEntity:add_component(ComponentType.Mesh).mesh = cubeMesh

local speed = 0.1

function move_mesh(dir)
    meshTrans.position = meshTrans.position + (dir * speed)
end

function move_left()
    move_mesh(Vec3.left)
end

function move_right()
    move_mesh(Vec3.right)
end

function move_forward()
    move_mesh(Vec3.forward)
end

function move_backward()
    move_mesh(Vec3.backward)
end

local groundPlane = Plane.new(Vec3.up);

function move_mouse()
    local ray = cam:screen_point_to_ray(app.input.mouse.position)
    local dist = ray:intersect(groundPlane)
    if dist ~= nil then
        meshTrans.position = ray * dist;
    end
end

app.input:add_bindings("test", {
    KeyLeft = move_left,
    KeyA = move_left,
    KeyRight = move_right,
    KeyD = move_right,
    KeyUp = move_forward,
    KeyW = move_forward,
    KeyDown = move_backward,
    KeyS = move_backward,
    MouseLeft = move_mouse,
})

