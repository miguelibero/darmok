
local program = app.assets:load_standard_program("ForwardPhong")
local layout = program.vertex_layout
local cubeMesh = Mesh.new_cube(program.vertex_layout)
local greenTex = app.assets:load_color_texture(colors.green)
cubeMesh.material = Material.new(greenTex)

local camEntity = app.scene:create_entity()
local camTrans = camEntity:add_transform_component()
camTrans.position = vec3.new(0, 2, -2)
camTrans.rotation = vec3.new(45, 0, 0)
local cam = camEntity:add_camera_component()
cam:set_projection(60, app.window.size, 0.3, 1000)
cam:set_forward_phong_renderer(program)

local lightEntity = app.scene:create_entity()
local lightTrans = lightEntity:add_transform_component()
lightTrans.position = vec3.new(1, 1, -2)
lightEntity:add_point_light_component()

local meshEntity = app.scene:create_entity()
local meshTrans = meshEntity:add_transform_component()
meshTrans.position = vec3.new(0, 0, 0)
meshEntity:add_mesh_component(cubeMesh)