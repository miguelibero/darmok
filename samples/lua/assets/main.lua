

function init()
    local program = Program.new(StandardProgramType.Forward)

    local cubeMesh = MeshData.new_cube():create_mesh(program.vertex_layout)
    local greenTex = app.assets:load_color_texture(Color.green)
    local greenMat = Material.new(program, greenTex)

    local scene = app:add_component(SceneAppComponent).scene

    local camEntity = scene:create_entity()
    local camTrans = camEntity:add_component(Transform, { 0, 2, -2 })
    camTrans:look_at({ 0, 0, 0 })
    local cam = camEntity:add_component(Camera)
    cam:set_perspective(60, app.window.size, 0.3, 1000)
    local renderer = cam:add_renderer(ForwardRenderer)
    renderer:add_component(LightingCameraComponent)

    local freelook = scene:add_component(FreelookController, cam)

    local lightEntity = scene:create_entity()
    lightEntity:add_component(Transform, { 1, 1, -2 })
    lightEntity:add_component(PointLight)

    local meshEntity = scene:create_entity()
    local meshTrans = meshEntity:add_component(Transform)
    meshEntity:add_component(Renderable, cubeMesh, greenMat)

    local speed = 0.1

    function move_mesh(dir)
        if freelook.enabled then
            return
        end
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
        if freelook.enabled then
            return
        end
        local p = app.input.mouse.position
        p = app.window:window_to_screen_point(p)
        local ray = cam:screen_point_to_ray(p)
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
end
