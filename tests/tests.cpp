#include <iostream>
#include <folder_watcher/folder_watcher.hpp>

auto main() -> int
{
    // Folder Path
    auto watchedPath = std::filesystem::path(TEST_FOLDER);

    // Callbacks
    folder_watcher::FolderWatcher_Callbacks callbacks;
    callbacks.on_added_file = [](std::string_view path) {
        std::cout << "file added " << path << std::endl;
    };
    callbacks.on_changed_file = [](std::string_view path) {
        std::cout << "file changed " << path << std::endl;
    };
    callbacks.on_removed_file = [](std::string_view path) {
        std::cout << "file removed " << path << std::endl;
    };
    callbacks.on_invalid_folder_path = [](std::string_view path) {
        std::cout << "folder path invalid " << path << std::endl;
    };

    // Create the folder path
    folder_watcher::FolderWatcher folder_watcher{watchedPath, callbacks};

    while (true) {
        // Loop on update
        folder_watcher.update();
    };
}

