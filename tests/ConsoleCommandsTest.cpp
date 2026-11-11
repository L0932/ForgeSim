#include "ConsoleCommands.hpp"
#include "ConsoleTextureFallback.hpp"
#include "ConsoleImageCache.hpp"
#include "ConsoleLoadingDiagnostics.hpp"
#include <iostream>
#include <stdexcept>
#include <string>

using namespace ForgeSim::Sandbox;
namespace {
void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}
int main() {
 try {
    ConsoleCommands commands;
    Require(commands.Execute("  \t").message.empty(), "Blank input");
    Require(commands.Execute("missing").error, "Unknown command");
    Require(commands.Execute("clear extra").error, "Unexpected arguments");
    Require(commands.Execute("clear").clear, "Clear request");
    Require(commands.Execute("help extra extra").error, "Help argument validation");
    Require(commands.Execute("help scene.info").message.find("scene.info") != std::string::npos, "Command help");
    const ConsoleCommandContext context{"Gallery", "C:/Scenes/gallery.scene.json", true, "0.1.0-dev"};
    const auto scene = commands.Execute("\t scene.info  \n", context);
    Require(!scene.error && scene.message.find("Gallery") != std::string::npos &&
        scene.message.find("C:/Scenes/gallery.scene.json") != std::string::npos &&
        scene.message.find("yes") != std::string::npos, "Current scene context");
    Require(commands.Execute("scene.info").message.find("Not saved") != std::string::npos, "Unsaved scene");
    Require(commands.Execute("version", context).message == "ForgeSim 0.1.0-dev", "Version");
    commands.Register("test.echo", "Test extension.", "test.echo value", 1, 1,
        [](auto args, const auto&) { return ConsoleCommandResult{args[0]}; });
    Require(commands.Execute("help").message.find("test.echo value") != std::string::npos, "Dynamic help");
    Require(commands.Execute("test.echo").error, "Missing argument");
    Require(commands.Execute("test.echo one two").error, "Too many arguments");
    Require(commands.Execute("test.echo hello").message == "hello", "Registered command execution");
    Require(commands.Execute(R"(test.echo "C:\Assets with spaces\Cathedral_01\")").message ==
        R"(C:\Assets with spaces\Cathedral_01\)", "Quoted Windows path with trailing separator");
    Require(commands.Execute(R"(test.echo "C:/Assets/église")").message == "C:/Assets/église", "UTF-8 path");
    Require(commands.Execute(R"(test.echo "unterminated)").error, "Unclosed quote rejected");
    Require(commands.Execute(R"(test.echo "")").message.empty(), "Empty quoted argument retained");

    commands.Register("test.fail", "Throw.", "test.fail", 0, 0,
        [](auto, const auto&) -> ConsoleCommandResult { throw std::runtime_error("handler failed"); });
    Require(commands.Execute("test.fail").error, "Execution failure containment");
    Require(!commands.Execute("version", context).error, "Recovery after execution failure");
    bool duplicate = false;
    try { commands.Register("version", "", "", 0, 0, [](auto, const auto&) { return ConsoleCommandResult{}; }); }
    catch (const std::invalid_argument&) { duplicate = true; }
    Require(duplicate, "Duplicate registration rejected");

    bool fallback = false;
    commands.Register("textures.fallback", "Texture policy", "textures.fallback [on|off]", 0, 1,
        [&fallback](auto args, const auto&) { return ExecuteTextureFallbackCommand(args, fallback); });
    Require(commands.Execute("textures.fallback").message.find("off") != std::string::npos,
        "Query initial fallback policy");
    Require(!commands.Execute("textures.fallback on").error && fallback, "Enable fallbacks");
    Require(commands.Execute("textures.fallback maybe").error && fallback, "Invalid toggle preserves policy");
    Require(commands.Execute("textures.fallback on extra").error && fallback, "Extra toggle argument rejected");
    auto chain = commands.ExecuteLine("textures.fallback off; version", context);
    Require(chain.size() == 2 && !chain[0].error && !fallback &&
        chain[1].message == "ForgeSim 0.1.0-dev", "Sequential commands");
    chain = commands.ExecuteLine(R"(test.echo "C:/folder;name"; textures.fallback on)");
    Require(chain.size() == 2 && chain[0].message == "C:/folder;name" && fallback,
        "Quoted semicolon stays in path argument");
    chain = commands.ExecuteLine("textures.fallback off; missing; textures.fallback on");
    Require(chain.size() == 2 && chain.back().error && !fallback, "Error stops later commands");
    chain = commands.ExecuteLine(R"(textures.fallback on; test.echo "unfinished)");
    Require(chain.size() == 1 && chain[0].error && !fallback, "Bad quoting prevents all execution");
    chain = commands.ExecuteLine(" ; ; textures.fallback on; ; ");
    Require(chain.size() == 1 && fallback, "Ignore empty command segments");
    Require(commands.ExecuteLine("; ; ").empty(), "Empty command chain");
    chain = commands.ExecuteLine("clear; help");
    Require(chain.size() == 2 && chain[0].clear && !chain[1].error, "Clear then help chain");

    bool caching = false;
    commands.Register("images.cache", "Decoded images", "images.cache [on|off]", 0, 1,
        [&caching](auto args, const auto&) { return ExecuteImageCacheCommand(args, caching); });
    Require(commands.Execute("images.cache").message.find("off") != std::string::npos,
        "Query disabled cache");
    Require(!commands.Execute("images.cache on").error && caching, "Enable decoded-image caching");
    Require(commands.Execute("images.cache").message.find("on") != std::string::npos && caching,
        "Query enabled cache preserves setting");
    Require(commands.Execute("images.cache invalid").error && caching,
        "Invalid cache option preserves setting");
    Require(commands.Execute("images.cache on extra").error && caching,
        "Reject extra cache arguments");
    Require(!commands.Execute("images.cache off").error && !caching, "Disable decoded-image caching");
    chain = commands.ExecuteLine(R"(test.echo "C:/Assets with spaces"; textures.fallback on; images.cache on)");
    Require(chain.size() == 3 && !chain.back().error && caching && fallback,
        "Cache and fallback settings in one line");
    chain = commands.ExecuteLine("images.cache off; missing; images.cache on");
    Require(chain.back().error && !caching, "Failed chain leaves later cache setting unapplied");

    bool diagnostics = true;
    commands.Register("loading.diagnostics", "Load timings", "loading.diagnostics [on|off]", 0, 1,
        [&diagnostics](auto args, const auto&) { return ExecuteLoadingDiagnosticsCommand(args, diagnostics); });
    Require(commands.Execute("loading.diagnostics").message.find("on") != std::string::npos && diagnostics,
        "Query startup diagnostics policy");
    Require(!commands.Execute("loading.diagnostics off").error && !diagnostics, "Disable load diagnostics");
    Require(commands.Execute("loading.diagnostics bad").error && !diagnostics,
        "Invalid diagnostics argument preserves setting");
    Require(commands.Execute("loading.diagnostics on extra").error && !diagnostics,
        "Reject extra diagnostics arguments");
    chain = commands.ExecuteLine(R"(test.echo "C:/Assets"; textures.fallback on; images.cache on; loading.diagnostics on)");
    Require(chain.size() == 4 && !chain.back().error && diagnostics && caching && fallback,
        "Combined load settings and timings");
    Require(commands.Execute("help loading.diagnostics").message.find("[on|off]") != std::string::npos,
        "Diagnostics usage available through help");

    ConsoleCommandHistory history;
    Require(history.Previous("draft") == "draft", "Empty history");
    history.Add("help"); history.Add("version"); history.Add("version");
    Require(history.Previous("unfinished") == "version", "Newest history");
    Require(history.Previous("") == "help", "No consecutive duplicates");
    Require(history.Previous("") == "help", "History upper boundary");
    Require(history.Next() == "version", "History next");
    Require(history.Next() == "unfinished", "Draft restoration");
    history.Add("scene.info");
    Require(history.Previous("") == "scene.info", "Resubmit recalled command");
    for(int i=0;i<110;++i) history.Add(std::to_string(i));
    std::string oldest;
    for(int i=0;i<150;++i) oldest=history.Previous("");
    Require(oldest=="10", "Bounded history");
    std::cout << "Console commands and history passed\n";
    return 0;
 } catch(const std::exception& exception) { std::cerr << exception.what() << '\n'; return 1; }
}
