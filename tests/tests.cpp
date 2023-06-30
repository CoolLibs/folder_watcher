#include <folder_watcher/folder_watcher.hpp>
#include <iostream>

auto main() -> int
{
    auto folder_watcher = folder_watcher::FolderWatcher{TEST_FOLDER};
    while (true)
    {
        folder_watcher.update({
            .on_file_added          = [](std::filesystem::path const& path) { std::cout << "file added " << path.string() << std::endl; },
            .on_file_removed        = [](std::filesystem::path const& path) { std::cout << "file removed " << path.string() << std::endl; },
            .on_file_changed        = [](std::filesystem::path const& path) { std::cout << "file changed " << path.string() << std::endl; },
            .on_invalid_folder_path = [](std::filesystem::path const& path) { std::cout << "folder path invalid " << path.string() << std::endl; },
        });
    }
}
