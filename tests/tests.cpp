#include <filesystem>
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include <folder_watcher/folder_watcher.hpp>
#include <quick_imgui/quick_imgui.hpp>

// Learn how to use Dear ImGui: https://coollibs.github.io/contribute/Programming/dear-imgui

auto main(int argc, char* argv[]) -> int
{
    const int  exit_code              = doctest::Context{}.run(); // Run all unit tests
    const bool should_run_imgui_tests = argc < 2 || strcmp(argv[1], "-nogpu") != 0;
    if (
        should_run_imgui_tests
        && exit_code == 0 // Only open the window if the tests passed; this makes it easier to notice when some tests fail
    )
    {
        auto                          watchedPath = std::filesystem::path(TEST_FOLDER);
        folder_watcher::FolderWatcher folder_watcher{watchedPath};

        folder_watcher::FolderWatcher_Callbacks callbacks;
        callbacks.on_file_added = [](std::string_view path){ std::cout << "file added " << path << std::endl; };
        callbacks.on_file_changed = [](std::string_view path){ std::cout << "file changed " << path << std::endl; };
        callbacks.on_file_removed = [](std::string_view path){ std::cout << "file removed " << path << std::endl; };
        callbacks.on_folder_path_invalid = [](std::string_view path){ std::cout << "folder path invalid " << path << std::endl; };        

        quick_imgui::loop("folder_watcher tests", [&folder_watcher, &callbacks]() { // Open a window and run all the ImGui-related code
            ImGui::Begin("folder_watcher tests");
            ImGui::End();
            folder_watcher.update(callbacks);
        });
    }
    return exit_code;
}

// Check out doctest's documentation: https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md
