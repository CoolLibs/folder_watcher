#include <folder_watcher/folder_watcher.hpp>
#include <iostream>

auto main() -> int
{
    // Folder Path
    auto const watched_path = std::filesystem::path{TEST_FOLDER};

    // Callbacks
    auto const callbacks = folder_watcher::FolderWatcher_Callbacks{
        .on_added_file          = [](std::string_view path) { std::cout << "file added " << path << std::endl; },
        .on_removed_file        = [](std::string_view path) { std::cout << "file removed " << path << std::endl; },
        .on_changed_file        = [](std::string_view path) { std::cout << "file changed " << path << std::endl; },
        .on_invalid_folder_path = [](std::string_view path) { std::cout << "folder path invalid " << path << std::endl; },
    };

    // Create the folder path
    auto const folder_watcher = folder_watcher::FolderWatcher{watched_path, callbacks};

    // Loop on update
    while (true)
    {
        folder_watcher.update();
    }
}
