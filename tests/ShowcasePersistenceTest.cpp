#include "EditorSession.hpp"
#include "SceneCameraState.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneEnvironment.hpp"
#include "SceneSerializer.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace
{
    using namespace ForgeSim::Sandbox;
    using Json = nlohmann::json;
    namespace fs = std::filesystem;

    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    template<typename Action>
    void ExpectFailure(Action action, const char* detail)
    {
        bool failed = false;
        try { action(); }
        catch (const std::exception& error)
        {
            failed = true;
            Require(std::string{ error.what() }.find(detail) != std::string::npos,
                "Failure omitted the expected diagnostic.");
        }
        Require(failed, "Invalid operation was accepted.");
    }

    class TemporaryDirectory final
    {
    public:
        TemporaryDirectory()
        {
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            for (int attempt = 0; attempt < 100; ++attempt)
            {
                const auto candidate = fs::temp_directory_path() /
                    ("ForgeSimShowcase-" + std::to_string(stamp) + "-" + std::to_string(attempt));
                if (fs::create_directory(candidate)) { m_Path = candidate; return; }
            }
            throw std::runtime_error("Could not create test directory.");
        }
        ~TemporaryDirectory() { std::error_code ignored; fs::remove_all(m_Path, ignored); }
        const fs::path& Path() const { return m_Path; }
        TemporaryDirectory(const TemporaryDirectory&) = delete;
        TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
    private:
        fs::path m_Path;
    };

    void WriteFile(const fs::path& path, const std::string& text)
    {
        std::ofstream output{ path, std::ios::binary };
        output << text;
        output.close();
        Require(!output.fail(), "Could not write test fixture.");
    }

    SceneCameraData Camera()
    {
        SceneCameraData camera;
        camera.position = { 7.0f, 3.0f, -2.0f };
        camera.forwardDirection = { 1.0f, 0.0f, 0.0f };
        camera.upDirection = { 0.0f, 0.0f, 1.0f };
        camera.verticalFieldOfViewRadians = glm::radians(60.0f);
        camera.nearClippingPlane = 0.1f;
        camera.farClippingPlane = 400.0f;
        camera.referenceViewport = SceneCaptureExtent{ 1600, 900 };
        return camera;
    }

    void TestBookmarksAndCameraRestoration()
    {
        TemporaryDirectory directory;
        EditorSession session{ SceneDocument{}, directory.Path() };
        SandboxScene& scene = session.Scene();
        const auto camera = Camera();
        const auto runtime = CreateSceneCamera(camera);
        const auto captured = CaptureSceneCamera(runtime, camera.referenceViewport);
        Require(captured.position == camera.position &&
            captured.forwardDirection == camera.forwardDirection &&
            captured.upDirection == camera.upDirection &&
            captured.verticalFieldOfViewRadians == camera.verticalFieldOfViewRadians &&
            captured.nearClippingPlane == camera.nearClippingPlane &&
            captured.farClippingPlane == camera.farClippingPlane &&
            captured.referenceViewport->width == 1600 && captured.referenceViewport->height == 900,
            "Camera pose, roll, projection or capture dimensions changed.");
        scene.AddCameraBookmark("Altar", captured);
        scene.AddCameraBookmark("Entrance", SceneCameraData{});
        scene.SelectCameraBookmark(0);
        Require(scene.SavedCamera()->position == camera.position,
            "Bookmark selection did not establish the active camera.");
        session.SaveAs("Showcase.json");
        auto reopened = session.PrepareOpen("Showcase.json");
        Require(reopened.Scene().CameraBookmarks().size() == 2 &&
            reopened.Scene().CameraBookmarks()[0].name == "Altar" &&
            !reopened.HasUnsavedChanges(), "Bookmarks did not survive file round trip.");
        const auto restored = CreateSceneCamera(*reopened.Scene().SavedCamera());
        const auto beforeProjection = runtime.ProjectionMatrix(1600.0f / 900.0f);
        const auto afterProjection = restored.ProjectionMatrix(1600.0f / 900.0f);
        for (int column = 0; column < 4; ++column)
            for (int row = 0; row < 4; ++row)
                Require(std::abs(beforeProjection[column][row] - afterProjection[column][row]) < 0.00001f,
                    "Restored bookmark changed projection.");
        auto navigating = restored;
        navigating.Translate({ 1.0f, 0.0f, 0.0f });
        Require(!reopened.HasUnsavedChanges(), "Ordinary navigation dirtied the scene.");
        auto edited = camera;
        edited.position.x = 9.0f;
        reopened.Scene().UpdateCameraBookmark(0, edited);
        Require(reopened.HasUnsavedChanges(), "Bookmark update was not detected.");
        Require(reopened.Scene().SavedCamera()->position == camera.position,
            "Updating a bookmark unexpectedly changed the independent active viewpoint.");
        reopened.Scene().SelectCameraBookmark(0);
        Require(reopened.Scene().SavedCamera()->position == edited.position,
            "Selecting updated bookmark restored stale camera data.");
        const auto stable = SerializeSceneDocument(reopened.Scene().ToDocument());
        ExpectFailure([&] { reopened.Scene().AddCameraBookmark("Altar", camera); }, "already exists");
        ExpectFailure([&] { reopened.Scene().AddCameraBookmark("   ", camera); }, "Bookmark names");
        ExpectFailure([&] { reopened.Scene().AddCameraBookmark(std::string(129, 'x'), camera); }, "Bookmark names");
        auto invalid = camera;
        invalid.nearClippingPlane = 0.0f;
        ExpectFailure([&] { reopened.Scene().UpdateCameraBookmark(0, invalid); }, "nearClippingPlane");
        ExpectFailure([&] { reopened.Scene().AddCameraBookmark("Invalid", invalid); }, "nearClippingPlane");
        Require(SerializeSceneDocument(reopened.Scene().ToDocument()) == stable,
            "Rejected bookmark edit changed the scene.");
        reopened.Scene().RemoveCameraBookmark(0);
        Require(reopened.Scene().CameraBookmarks().size() == 1 && reopened.Scene().SavedCamera(),
            "Deleting a bookmark lost the independent active camera.");
        reopened.Save();
        auto finalSession = reopened.PrepareOpen("Showcase.json");
        Require(finalSession.Scene().CameraBookmarks().size() == 1 && !finalSession.HasUnsavedChanges(),
            "Bookmark deletion was not persisted.");
        auto fresh = finalSession.PrepareNew();
        Require(fresh.Scene().CameraBookmarks().empty() && !fresh.Scene().SavedCamera(),
            "New scene inherited authored cameras.");
        auto impossible = camera;
        impossible.position = glm::vec3{ std::numeric_limits<float>::max() };
        ExpectFailure([&] { static_cast<void>(CreateSceneCamera(impossible)); }, "position");
    }

    void TestShadowPersistenceAndDefaults()
    {
        TemporaryDirectory directory;
        EditorSession session{ SceneDocument{}, directory.Path() };
        session.SaveAs("Shadows.json");
        const auto original = session.Scene().RenderingSettings();
        for (int field = 0; field < 4; ++field)
        {
            auto edited = original;
            switch (field)
            {
            case 0: edited.directionalShadows.enabled = false; break;
            case 1: edited.directionalShadows.filteringEnabled = false; break;
            case 2: edited.directionalShadows.minimumBias = 0.004f; break;
            case 3: edited.directionalShadows.slopeBias = 0.01f; break;
            }
            Require(!session.Scene().TrySetRenderingSettings(edited), "Valid shadow edit rejected.");
            Require(session.HasUnsavedChanges(), "Shadow edit did not dirty the scene.");
            session.Save();
            const auto reopened = session.PrepareOpen("Shadows.json");
            const auto& restored = reopened.Scene().RenderingSettings().directionalShadows;
            Require(restored.enabled == edited.directionalShadows.enabled &&
                restored.filteringEnabled == edited.directionalShadows.filteringEnabled &&
                restored.minimumBias == edited.directionalShadows.minimumBias &&
                restored.slopeBias == edited.directionalShadows.slopeBias,
                "Shadow settings were lost on disk.");
            Require(!session.Scene().TrySetRenderingSettings(original), "Shadow reset rejected.");
            session.Save();
        }
        Json legacy = Json::parse(SerializeSceneDocument(SceneDocument{}));
        legacy.erase("cameraBookmarks");
        legacy["rendering"].erase("directionalShadows");
        legacy["rendering"].erase("environment");
        const auto loaded = DeserializeSceneDocument(legacy.dump());
        Require(loaded.cameraBookmarks.empty() && !loaded.camera &&
            loaded.rendering.directionalShadows.minimumBias == 0.0005f &&
            loaded.rendering.environmentSource == SceneEnvironmentSource::BundledStudio,
            "Older v2 scene did not receive defaults.");
        legacy["version"] = 1;
        legacy.erase("camera");
        legacy.erase("rendering");
        Require(!DeserializeSceneDocument(legacy.dump()).camera,
            "Legacy v1 scene did not retain automatic framing.");
        SceneDocument oldCamera;
        oldCamera.camera = Camera();
        Json cameraJson = Json::parse(SerializeSceneDocument(oldCamera));
        cameraJson["camera"].erase("referenceViewport");
        Require(!DeserializeSceneDocument(cameraJson.dump()).camera->referenceViewport,
            "Old camera was given fabricated capture metadata.");
    }

    void TestMalformedShowcaseData()
    {
        SceneDocument source;
        source.camera = Camera();
        source.cameraBookmarks.push_back({ "Altar", Camera() });
        const Json valid = Json::parse(SerializeSceneDocument(source));
        auto reject = [](const Json& value, const char* property)
        {
            ExpectFailure([&] { static_cast<void>(DeserializeSceneDocument(value.dump())); }, property);
        };
        Json candidate = valid;
        candidate["cameraBookmarks"] = Json::object();
        reject(candidate, "cameraBookmarks");
        candidate = valid;
        candidate["cameraBookmarks"].push_back(candidate["cameraBookmarks"][0]);
        reject(candidate, "cameraBookmarks[1].name");
        candidate = valid;
        candidate["cameraBookmarks"][0]["camera"]["farClippingPlane"] = 0;
        reject(candidate, "cameraBookmarks[0].camera.farClippingPlane");
        for (const char* field : { "enabled", "filteringEnabled", "minimumBias", "slopeBias" })
        {
            candidate = valid;
            candidate["rendering"]["directionalShadows"].erase(field);
            reject(candidate, field);
        }
        candidate = valid;
        candidate["rendering"]["directionalShadows"]["enabled"] = 1;
        reject(candidate, "directionalShadows.enabled");
        candidate = valid;
        candidate["rendering"]["directionalShadows"]["minimumBias"] = 0.1;
        reject(candidate, "directionalShadows.minimumBias");
        candidate = valid;
        candidate["rendering"]["directionalShadows"]["slopeBias"] = -1;
        reject(candidate, "directionalShadows.slopeBias");
        for (const Json& width : Json::array({ 0, -1, 1.5, 4294967296ULL, "wide" }))
        {
            candidate = valid;
            candidate["camera"]["referenceViewport"]["width"] = width;
            reject(candidate, "camera.referenceViewport.width");
        }
        candidate = valid;
        candidate["rendering"]["environment"]["source"] = "unknown";
        reject(candidate, "rendering.environment.source");
        for (const char* path : { "../outside.hdr", "/absolute.hdr", "C:/outside.hdr",
            "env\\studio.hdr", "env//studio.hdr", "env/./studio.hdr", "" })
        {
            candidate = valid;
            candidate["rendering"]["environment"] = { { "source", "projectAsset" }, { "asset", path } };
            reject(candidate, "rendering.environment.asset");
        }
        candidate = valid;
        candidate["rendering"]["environment"]["asset"] = "unexpected.hdr";
        reject(candidate, "rendering.environment.asset");
        const auto output = Json::parse(SerializeSceneDocument(source));
        for (const char* editorField : { "showGrid", "showLights", "selection", "navigationSpeed", "theme", "layout" })
            Require(!output.contains(editorField) && !output["rendering"].contains(editorField),
                "Editor preference leaked into the authored scene schema.");
    }

    void TestEnvironmentFiles()
    {
        TemporaryDirectory directory;
        fs::create_directories(directory.Path() / "environments");
        fs::create_directories(directory.Path() / "bundled");
        const fs::path asset = directory.Path() / "environments/Studio.hdr";
        // File I/O validates references, not HDR decoding (the renderer does that).
        WriteFile(asset, "HDR placeholder");
        WriteFile(directory.Path() / "bundled/Studio_Small_01_4k.hdr", "bundled placeholder");
        SceneDocument document;
        document.rendering.environmentSource = SceneEnvironmentSource::ProjectAsset;
        document.rendering.environmentAsset = "environments/Studio.hdr";
        const auto resolved = ResolveSceneEnvironment(document.rendering, directory.Path(), directory.Path() / "bundled");
        Require(resolved && fs::equivalent(*resolved, asset), "Environment did not resolve against project root.");
        EditorSession session{ document, directory.Path() };
        session.SaveAs("Environment.json");
        const auto reopened = session.PrepareOpen("Environment.json");
        Require(reopened.Scene().RenderingSettings().environmentAsset == document.rendering.environmentAsset &&
            reopened.Scene().RenderingSettings().environmentSource == SceneEnvironmentSource::ProjectAsset,
            "Environment reference did not survive save/load.");
        const auto before = SerializeSceneDocument(session.Scene().ToDocument());
        fs::remove(asset);
        ExpectFailure([&] { static_cast<void>(session.PrepareOpen("Environment.json")); }, "rendering.environment.asset");
        ExpectFailure([&] { session.Save(); }, "rendering.environment.asset");
        Require(!session.HasUnsavedChanges() && SerializeSceneDocument(session.Scene().ToDocument()) == before,
            "Failed environment operation damaged the active session.");
        fs::create_directory(asset);
        ExpectFailure([&] { static_cast<void>(ResolveSceneEnvironment(document.rendering,
            directory.Path(), directory.Path() / "bundled")); }, "rendering.environment.asset");
        auto settings = document.rendering;
        settings.environmentSource = SceneEnvironmentSource::None;
        settings.environmentAsset.clear();
        Require(!ResolveSceneEnvironment(settings, directory.Path(), {}), "None did not disable environment lighting.");
        Require(!session.Scene().TrySetRenderingSettings(settings) && session.HasUnsavedChanges(),
            "Environment source change did not mark the scene modified.");
        session.Save();
        settings.environmentSource = SceneEnvironmentSource::BundledStudio;
        const auto bundled = ResolveSceneEnvironment(settings, directory.Path(), directory.Path() / "bundled");
        Require(bundled && bundled->filename() == "Studio_Small_01_4k.hdr", "Bundled default was not resolved.");
    }
}

int main()
{
    try
    {
        TestBookmarksAndCameraRestoration();
        TestShadowPersistenceAndDefaults();
        TestMalformedShowcaseData();
        TestEnvironmentFiles();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
