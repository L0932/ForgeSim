#include "BundledSamples.hpp"
#include "EditorSession.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneSerializer.hpp"
#include "SceneAssetRecovery.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    struct TemporaryDirectory
    {
        fs::path path = fs::temp_directory_path() / ("ForgeSimSamples-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        TemporaryDirectory() { fs::create_directories(path); }
        ~TemporaryDirectory() { std::error_code error; fs::remove_all(path, error); }
    };
}

int main()
{
    try
    {
        TemporaryDirectory temp;
        const auto bundle = temp.path / "relocated application";
        const auto userDirectory = temp.path / "user scenes";
        const auto externalRoot = temp.path / "external project";
        fs::create_directories(bundle / "assets/scenes");
        fs::create_directories(bundle / "assets/models");
        fs::create_directories(userDirectory);
        fs::create_directories(externalRoot);
        const fs::path source = FORGESIM_SAMPLE_ASSET_DIRECTORY;
        for (const auto& sample : BundledSamples)
            fs::copy_file(source / "scenes" / sample.filename,
                bundle / "assets/scenes" / sample.filename);
        for (const auto* model : {"ShadowTestCube.gltf", "PbrMaterialGallery.glb"})
            fs::copy_file(source / "models" / model, bundle / "assets/models" / model);

        SceneDocument document;
        document.name = "External scene";
        EditorSession current(document, externalRoot, std::nullopt, userDirectory);
        Require(!IsBundledSample("../other.scene.json"), "Catalog rejects arbitrary filenames");
        for (const auto& entry : BundledSamples)
        {
            const auto sourcePath = bundle / "assets/scenes" / entry.filename;
            const auto sourceSize = fs::file_size(sourcePath);
            auto sample = current.PrepareSample(sourcePath, bundle);
            Require(sample.ProjectRoot() == fs::canonical(bundle), "Sample uses bundled root");
            Require(!sample.ScenePath(), "Sample opens as an editable copy");
            Require(sample.DefaultSceneDirectory() == userDirectory, "Save dialog keeps user directory");
            Require(sample.ModelPaths().size() == 1 && fs::is_regular_file(sample.ModelPaths().front()),
                "Sample model resolves after relocation");
            bool failed = false;
            try { sample.Save(); } catch (const std::logic_error&) { failed = true; }
            Require(failed, "Save requires destination before writing sample");
            const auto saved = userDirectory / entry.filename;
            sample.SaveAs(saved);
            Require(sample.ScenePath() == saved, "Save As adopts user file");
            Require(LoadSceneDocument(saved, bundle).objects.size() == sample.Scene().Objects().size(),
                "Saved sample reloads with bundled asset root");
            Require(fs::file_size(sourcePath) == sourceSize, "Bundled source is untouched");
        }
        bool failed = false;
        try { static_cast<void>(current.PrepareSample(bundle / "missing.json", bundle)); }
        catch (const std::exception&) { failed = true; }
        Require(failed && current.Scene().Name() == "External scene" &&
            current.ProjectRoot() == fs::canonical(externalRoot) && !current.HasUnsavedChanges(),
            "Failed sample open preserves active session");
        // A recovery attempt must validate every model and project environment
        // before replacement, preserving the current session and its dirty edits.
        SceneDocument recoveryDocument;
        recoveryDocument.name = "Recovery target";
        recoveryDocument.objects.push_back({.id=1, .name="Model", .transform={}, .modelPath="models/model.glb"});
        recoveryDocument.rendering.environmentSource = SceneEnvironmentSource::ProjectAsset;
        recoveryDocument.rendering.environmentAsset = "environments/interior.hdr";
        const auto recoveryPath = userDirectory / "recovery.scene.json";
        std::ofstream(recoveryPath) << SerializeSceneDocument(recoveryDocument);
        SceneDocument activeDocument;
        activeDocument.objects.push_back({.id=1, .name="Active object", .transform={}, .modelPath="old.glb"});
        EditorSession active(activeDocument, externalRoot, std::nullopt, userDirectory);
        active.Scene().SetObjectVisible(1, false);
        const auto original = SerializeSceneDocument(active.Scene().ToDocument());
        try
        {
            static_cast<void>(active.PrepareOpen(recoveryPath, externalRoot));
            throw std::runtime_error("Missing assets should fail");
        }
        catch (const MissingSceneAssets& missing)
        {
            Require(missing.root == fs::canonical(externalRoot), "Recovery reports attempted root");
            const auto description = DescribeMissingSceneAssets(recoveryPath, missing);
            Require(description.find("objects[0]") == std::string::npos &&
                description.find("Missing model: models/model.glb") != std::string::npos &&
                description.find("Missing environment: environments/interior.hdr") != std::string::npos,
                "Recovery uses user-facing labels instead of serialized field names");
            Require(missing.assets.size() == 2, "Recovery includes model and environment failures");
            Require(missing.assets[0].reference == "models/model.glb" &&
                missing.assets[1].field == "rendering.environment.asset", "Recovery has structured references");
        }
        const auto locatedRoot = temp.path / "located assets";
        fs::create_directories(locatedRoot / "models");
        fs::create_directories(locatedRoot / "environments");
        std::ofstream(locatedRoot / "models/model.glb") << "placeholder";
        try
        {
            static_cast<void>(active.PrepareOpen(recoveryPath, locatedRoot));
            throw std::runtime_error("Missing environment should still fail");
        }
        catch (const MissingSceneAssets& missing)
        {
            Require(missing.assets.size() == 1 &&
                missing.assets[0].reference == "environments/interior.hdr",
                "Retry validates environment after model is found");
        }
        Require(active.HasUnsavedChanges() && SerializeSceneDocument(active.Scene().ToDocument()) == original,
            "Failed retries and cancellation preserve dirty active scene");
        std::ofstream(locatedRoot / "environments/interior.hdr") << "placeholder";
        Require(InferSceneAssetRoot(locatedRoot / "models/model.glb", "models/model.glb") ==
            fs::canonical(locatedRoot), "Selecting a model infers the matching root");
        fs::create_directories(locatedRoot / "renamed models");
        std::ofstream(locatedRoot / "renamed models/model.glb") << "placeholder";
        Require(!InferSceneAssetRoot(locatedRoot / "renamed models/model.glb", "models/model.glb"),
            "Renamed subfolders must not silently remap authored references");
        Require(!InferSceneAssetRoot(locatedRoot / "models/model.glb", "../model.glb"),
            "Invalid authored paths cannot establish a root");
        const auto recovered = active.PrepareOpen(recoveryPath, locatedRoot);
        Require(recovered.Scene().Name() == "Recovery target" &&
            recovered.ProjectRoot() == fs::canonical(locatedRoot), "Valid located root prepares replacement");
        Require(SerializeSceneDocument(active.Scene().ToDocument()) == original,
            "Successful preparation does not replace active scene before resource loading");
        std::cout << "Bundled sample relocation, root isolation, and Save As passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
