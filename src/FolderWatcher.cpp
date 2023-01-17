#include "./FolderWatcher.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <utility>

namespace folder_watcher {

using RecursiveDirectoryIterator = fs::recursive_directory_iterator;
using DirectoryIterator          = fs::directory_iterator;
using Clock                      = std::chrono::steady_clock;

FolderWatcher::FolderWatcher(fs::path folder_path, FolderWatcher_Config config)
    : _path(std::move(folder_path)), _folder_last_change(fs::last_write_time(_path)), _config(config)
{
    init_files();
};

void FolderWatcher::update(const FolderWatcher_Callbacks& p_callbacks)
{
    // ToDo state
    if (!fs::exists(_path) || hasCheckTooRecently())
        return;

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
        const auto last_change = fs::last_write_time(file.path);
        if (last_change != file.time_of_last_change)
        {
            on_changed_file(file, p_callbacks);
            continue;
        }
    }

    remove_files(p_callbacks, will_be_removed);
    check_for_new_paths(p_callbacks);
    _folder_last_change = fs::last_write_time(_path);
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
    // sort isn't that useful ? remove operator
    // std::sort(_files.begin(), _files.end());
    // ToDO callbacks
    if (_config.recursive_watcher)
    {
        for (const auto& entry : RecursiveDirectoryIterator(_path))
            add_path_to_files(entry.path());
    }
    else
    {
        for (const auto& entry : DirectoryIterator(_path))
            add_path_to_files(entry.path());
    }
}

void FolderWatcher::set_path(fs::path path)
{
    _path          = std::move(path);
    _path_validity = Unknown{};
};

void FolderWatcher::add_path_to_files(fs::path const& path)
{
    if (fs::is_regular_file(path))
        _files.push_back({.path = path, .time_of_last_change = fs::last_write_time(path)});
}

void FolderWatcher::check_for_new_paths(const FolderWatcher_Callbacks& p_callbacks)
{
    if (_config.recursive_watcher)
    {
        for (const auto& entry : RecursiveDirectoryIterator(_path))
        {
            check_for_added_files(p_callbacks, entry);
        }
    }

    else
    {
        for (const auto& entry : DirectoryIterator(_path))
        {
            check_for_added_files(p_callbacks, entry);
        }
    }
}

void FolderWatcher::remove_files(const FolderWatcher_Callbacks& p_callbacks, std::vector<File>& will_be_removed)
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
        on_removed_file(file, p_callbacks);
}

void FolderWatcher::check_for_added_files(const FolderWatcher_Callbacks& p_callbacks, const fs::directory_entry& entry)
{
    if (!entry.is_regular_file())
        return;

    // If the file is not found in _files
    auto const file_iterator = std::find_if(_files.begin(), _files.end(), [entry](File const& file) { return file.path == entry; });
    if (file_iterator == _files.end())
        on_added_file(entry.path(), p_callbacks);
}

// répétition, on peut refacto ?
void FolderWatcher::on_added_file(const fs::path& p_path, const FolderWatcher_Callbacks& p_callbacks)
{
    add_path_to_files(p_path);
    p_callbacks.on_added_file(_files.back().path.string());
}

void FolderWatcher::on_removed_file(File const& p_file, const FolderWatcher_Callbacks& p_callbacks)
{
    p_callbacks.on_removed_file(p_file.path.string());
    _files.erase(std::remove_if(_files.begin(), _files.end(), [p_file](const File& cur_file) { return p_file == cur_file; }), _files.end());
}

void FolderWatcher::on_changed_file(File& p_file, const FolderWatcher_Callbacks& p_callbacks)
{
    _path_validity             = Valid{};
    p_file.time_of_last_change = fs::last_write_time(p_file.path);
    p_callbacks.on_changed_file(p_file.path.string());
}

void FolderWatcher::on_folder_path_invalid(const FolderWatcher_Callbacks& p_callbacks)
{
    _path_validity = Invalid{};
    p_callbacks.on_invalid_folder_path(_path.string());
}

} // namespace folder_watcher
