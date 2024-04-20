
local program = app.assets:load_standard_program("ForwardPhong")
local cubeMesh = Mesh.create_cube(program.vertex_layout)
local greenTex = app.assets:load_color_texture(Colors.green)
cubeMesh.material = Material.new(greenTex)

local camEntity = app.scene:create_entity()
local camTrans = camEntity:get_transform()
camTrans.position = vec3.new(0, 2, -2)
camTrans.rotation = vec3.new(45, 0, 0)
local cam = camEntity:get_camera()
cam:set_projection(60, vec2.new(0.3, 1000))
cam:set_forward_phong_renderer(program)

local lightEntity = app.scene:create_entity()
local lightTrans = lightEntity:get_transform()
lightTrans.position = vec3.new(1, 1, -2)
lightEntity:get_point_light()

local meshEntity = app.scene:create_entity()
local meshTrans = meshEntity:get_transform()
meshTrans.position = vec3.new(0, 0, 0)
meshEntity:add_mesh(cubeMesh)

local speed = 0.1

function move_mesh(dir)
    meshTrans.position = meshTrans.position + (dir * speed)
end

function move_left()
    move_mesh(vec3.left)
end

function move_right()
    move_mesh(vec3.right)
end

function move_forward()
    move_mesh(vec3.forward)
end

function move_backward()
    move_mesh(vec3.move_backward)
end

local groundPlane = Plane.new(vec3.up);

function move_mouse()
    local ray = cam:screen_point_to_ray(app.input.mouse.position)
    local dist = ray:intersect(groundPlane)
    if dist ~= nil then
        meshTrans.position = ray * dist;
    end
end

app.input:add_bindings("test", {
    Keyleft = move_left,
    KeyA = move_left,
    Keyright = move_right,
    KeyD = move_right,
    Keyup = move_forward,
    KeyW = move_forward,
    Keydown = move_backward,
    KeyS = move_backward,
    MouseLeft = move_mouse,
})

