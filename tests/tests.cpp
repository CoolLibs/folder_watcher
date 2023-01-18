#include <folder_watcher/folder_watcher.hpp>
#include <iostream>

auto main() -> int
{
    // Folder Path
    auto const watched_path = std::filesystem::path{TEST_FOLDER};

    // Callbacks
    auto const callbacks = folder_watcher::Callbacks{
        .on_file_added          = [](std::filesystem::path const& path) { std::cout << "file added " << path.string() << std::endl; },
        .on_file_removed        = [](std::filesystem::path const& path) { std::cout << "file removed " << path.string() << std::endl; },
        .on_file_changed        = [](std::filesystem::path const& path) { std::cout << "file changed " << path.string() << std::endl; },
        .on_invalid_folder_path = [](std::filesystem::path const& path) { std::cout << "folder path invalid " << path.string() << std::endl; },
    };

    // Create the folder path
    auto folder_watcher = folder_watcher::FolderWatcher{watched_path};

    // Loop on update
    while (true)
    {
        folder_watcher.update(callbacks);
    }
}
