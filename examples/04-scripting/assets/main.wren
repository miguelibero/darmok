import "math" for Vec3
import "base" for App, Program, Mesh, Material, Colors

class ScriptingExample {
    construct new() {}

    init(app, args) {
        var program = app.assets.loadStandardProgram("ForwardPhong")
        var layout = program.vertexLayout
        var cubeMesh = Mesh.newCube(layout)

        var greenTex = app.assets.loadColorTexture(Colors.green)
        cubeMesh.material = Material.new(greenTex)

        var camEntity = app.scene.newEntity()
        var camTrans = camEntity.addTransformComponent()
        camTrans.position = Vec3.new(0, 2, -2)
        camTrans.rotation = Vec3.new(45, 0, 0)
        var cam = camEntity.addCameraComponent()
        cam.setProjection(60, app.window.size, 0.3, 1000)
        cam.setForwardPhongRenderer(program)

        var lightEntity = app.scene.newEntity()
        var lightTrans = lightEntity.addTransformComponent()
        lightTrans.position = Vec3.new(1, 1, -2)
        lightEntity.addPointLightComponent()

        var meshEntity = app.scene.newEntity()
        var meshTrans = meshEntity.addTransformComponent()
        meshTrans.position = Vec3.new(0, 0, 0)
        meshEntity.addMeshComponent(cubeMesh)
    }
}

var main = ScriptingExample.new()