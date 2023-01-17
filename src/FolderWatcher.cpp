#include "./FolderWatcher.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <utility>

namespace folder_watcher {

using RecursiveDirectoryIterator = fs::recursive_directory_iterator;
using DirectoryIterator          = fs::directory_iterator;
using Clock                      = std::chrono::steady_clock;

static auto time_of_last_change(const std::filesystem::path& path) -> std::filesystem::file_time_type
{
    try
    {
        return std::filesystem::last_write_time(path);
    }
    catch (...)
    {
        return {};
    }
}

FolderWatcher::FolderWatcher(fs::path folder_path, FolderWatcher_Callbacks callbacks, FolderWatcher_Config config)
    : _path(std::move(folder_path)), _folder_last_change(time_of_last_change(_path)), _config(config), _callbacks(std::move(callbacks))
{
    init_files();
};

void FolderWatcher::update()
{
    if (hasCheckTooRecently())
        return;

    if (is_folder_path_invalid() || !fs::exists(_path))
    {
        on_folder_path_invalid();
        return;
    }

    std::vector<File> will_be_removed{};
    for (File& file : _files)
    {
        // Files deleted
        if (!fs::exists(file.path))
        {
            will_be_removed.push_back(file);
            continue;
        }

        // File changed
        const auto last_change = time_of_last_change(file.path);
        if (last_change != file.time_of_last_change)
        {
            on_changed_file(file);
            continue;
        }
    }

    remove_files(will_be_removed);
    check_for_new_paths();
    _folder_last_change = time_of_last_change(_path);
}
auto FolderWatcher::hasCheckTooRecently() const -> bool
{
    static auto last_check   = Clock::now();
    const auto  now          = Clock::now();
    const auto  elapsed_time = std::chrono::duration<float>{now - last_check};

    if (elapsed_time.count() < _config.delay_between_checks)
        return true;

    last_check = now;
    return false;
}

void FolderWatcher::init_files()
{
    _files.clear();

    if (!fs::exists(_path))
    {
        _path_validity = Invalid{};
        return;
    }

    if (_config.recursive_watcher)
    {
        for (const auto& entry : RecursiveDirectoryIterator(_path))
            on_added_file(entry);
    }
    else
    {
        for (const auto& entry : DirectoryIterator(_path))
            on_added_file(entry);
    }
}

[[maybe_unused]] void FolderWatcher::set_path(fs::path path)
{
    _path          = std::move(path);
    _path_validity = Unknown{};
};

void FolderWatcher::check_for_new_paths()
{
    if (_config.recursive_watcher)
    {
        for (const auto& entry : RecursiveDirectoryIterator(_path))
        {
            check_for_added_files(entry);
        }
    }

    else
    {
        for (const auto& entry : DirectoryIterator(_path))
        {
            check_for_added_files(entry);
        }
    }
}

void FolderWatcher::remove_files(std::vector<File>& will_be_removed)
{
    if (will_be_removed.empty())
        return;

    _files.erase(
        std::remove_if(_files.begin(), _files.end(), [&will_be_removed](const File& f) {
            return std::find(will_be_removed.begin(), will_be_removed.end(), f) != will_be_removed.end();
        }),
        _files.end()
    );

    for (File const& file : will_be_removed)
        on_removed_file(file);
}

void FolderWatcher::check_for_added_files(const fs::directory_entry& entry)
{
    if (!entry.is_regular_file())
        return;

    // If the file is not found in _files
    auto const file_iterator = std::find_if(_files.begin(), _files.end(), [entry](File const& file) { return file.path == entry; });
    if (file_iterator == _files.end())
        on_added_file(entry.path());
}

// répétition, on peut refacto ?
void FolderWatcher::on_added_file(const fs::path& path)
{
    if (fs::is_directory(path))
        return;

    _path_validity = Valid{};
    _files.push_back({.path = path, .time_of_last_change = time_of_last_change(path)});
    _callbacks.on_added_file(_files.back().path.string());
}

void FolderWatcher::on_removed_file(File const& file)
{
    _path_validity = Valid{};
    _callbacks.on_removed_file(file.path.string());
    _files.erase(std::remove_if(_files.begin(), _files.end(), [file](const File& cur_file) { return file == cur_file; }), _files.end());
}

void FolderWatcher::on_changed_file(File& file)
{
    _path_validity           = Valid{};
    file.time_of_last_change = time_of_last_change(file.path);
    _callbacks.on_changed_file(file.path.string());
}

void FolderWatcher::on_folder_path_invalid()
{
    _path_validity = Invalid{};
    _callbacks.on_invalid_folder_path(_path.string());
}

} // namespace folder_watcher
