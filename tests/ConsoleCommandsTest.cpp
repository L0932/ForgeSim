#include "ConsoleCommands.hpp"
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
    commands.Register("test.fail", "Throw.", "test.fail", 0, 0,
        [](auto, const auto&) -> ConsoleCommandResult { throw std::runtime_error("handler failed"); });
    Require(commands.Execute("test.fail").error, "Execution failure containment");
    Require(!commands.Execute("version", context).error, "Recovery after execution failure");
    bool duplicate = false;
    try { commands.Register("version", "", "", 0, 0, [](auto, const auto&) { return ConsoleCommandResult{}; }); }
    catch (const std::invalid_argument&) { duplicate = true; }
    Require(duplicate, "Duplicate registration rejected");

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
