function init()
    local program = Program.new(StandardProgramType.Forward)

    local cubeMesh = MeshData.new_cube():create_mesh(program.vertex_layout)
    local greenMat = Material.new(program, Color.green)

    local scene = app:add_component(SceneAppComponent).scene

    local camEntity = scene:create_entity()
    local camTrans = camEntity:add_component(Transform, { 0, 2, -2 })
    camTrans:look_at({ 0, 0, 0 })
    local cam = camEntity:add_component(Camera)
    cam:set_perspective(60, app.window.size, 0.3, 1000)
    cam:add_component(ForwardRenderer)
    cam:add_component(LightingRenderComponent)

    local freelook = scene:add_component(FreelookController, cam)

    local lightEntity = scene:create_entity()
    local lightTrans = lightEntity:add_component(Transform)
    lightTrans.position = { -1, 1, -1 }
    lightTrans:look_at({ 0, 0, 0 })
    lightEntity:add_component(PointLight, 2)
        .radius = 40
    lightEntity:add_component(AmbientLight, 0.4)

    local meshEntity = scene:create_entity()
    local meshTrans = meshEntity:add_component(Transform)
    meshEntity:add_component(Renderable, cubeMesh, greenMat)

    local speed = 0.1

    function move_mesh(tag)
        if freelook.enabled then
            return
        end
        local dir = Vec3.zero
        if tag == "left" then
            dir = Vec3.left
        elseif tag == "right" then
            dir = Vec3.right
        elseif tag == "forward" then
            dir = Vec3.forward
        elseif tag == "back" then
            dir = Vec3.backward
        end
        meshTrans.position = meshTrans.position + (dir * speed)
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

    app.input:add_listener("left", {
        KeyboardKey.Left, KeyboardKey.KeyA,
        { GamepadStick.Left, InputDirType.Left }
    }, move_mesh)
    app.input:add_listener("right", {
        KeyboardKey.Right, KeyboardKey.KeyD,
        { GamepadStick.Left, InputDirType.Right }
    }, move_mesh)
    app.input:add_listener("forward", {
        KeyboardKey.Up, KeyboardKey.KeyW,
        { GamepadStick.Left, InputDirType.Up }
    }, move_mesh)
    app.input:add_listener("back", {
        KeyboardKey.Down, KeyboardKey.KeyS,
        { GamepadStick.Left, InputDirType.Down }
    }, move_mesh)
    app.input:add_listener("mouse", MouseButton.Left, move_mouse)
end

function coroutineTest()
    app:start_coroutine(function()
        print("lalala1")
        coroutine.yield(app:start_coroutine(function()
            print("lalala2")
            coroutine.yield(nil)
            print("lalala3")
            return 1
        end))
        print("lalala4")
    end)
end