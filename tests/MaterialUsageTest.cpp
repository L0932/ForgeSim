#include "MaterialUsage.hpp"

#include <iostream>
#include <stdexcept>

using namespace ForgeSim;
namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
}
int main()
{
    try
    {
        Assets::SceneAsset scene;
        Require(Sandbox::ReferencedMaterials(scene).empty(), "Empty scene has no used materials");
        scene.materials.resize(3);
        // Original material 0 remains in the asset, but the importer remaps the
        // primitive to texture-compatible material 2. Material 1 is independent.
        scene.meshes.resize(2);
        scene.meshes[0].primitives.resize(2);
        scene.meshes[0].primitives[0].materialIndex = 2;
        scene.meshes[1].primitives.resize(1);
        scene.meshes[1].primitives[0].materialIndex = 1;
        auto used = Sandbox::ReferencedMaterials(scene);
        Require(used.size() == 3 && !used[0] && used[1] && used[2],
            "Only referenced variants upload; unused original stays at its index");
        scene.meshes[0].primitives[1].materialIndex = 0;
        Require(Sandbox::ReferencedMaterials(scene)[0],
            "A referenced original still uploads and receives normal validation");
        scene.meshes[0].primitives[1].materialIndex = 3;
        bool failed = false;
        try { static_cast<void>(Sandbox::ReferencedMaterials(scene)); }
        catch (const std::runtime_error&) { failed = true; }
        Require(failed, "Invalid material index fails rather than skipping validation");
        std::cout << "Material usage and fallback variant selection passed\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
