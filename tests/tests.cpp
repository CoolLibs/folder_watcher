#include <folder_watcher/folder_watcher.hpp>
#include <iostream>

auto main() -> int
{
    // Folder Path
    auto watchedPath = std::filesystem::path(TEST_FOLDER);

    // Callbacks
    const folder_watcher::FolderWatcher_Callbacks callbacks{
        .on_added_file          = [](std::string_view path) { std::cout << "file added " << path << std::endl; },
        .on_removed_file        = [](std::string_view path) { std::cout << "file removed " << path << std::endl; },
        .on_changed_file        = [](std::string_view path) { std::cout << "file changed " << path << std::endl; },
        .on_invalid_folder_path = [](std::string_view path) { std::cout << "folder path invalid " << path << std::endl; },
    };

    // Create the folder path
    folder_watcher::FolderWatcher folder_watcher{watchedPath, callbacks};

    while (true)
    {
        // Loop on update
        folder_watcher.update();
    };
}
