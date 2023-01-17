#include "./FolderWatcher.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include "./utils.hpp"

namespace folder_watcher {

using RecursiveDirectoryIterator = fs::recursive_directory_iterator;
using DirectoryIterator          = fs::directory_iterator;
using Clock                      = std::chrono::steady_clock;

FolderWatcher::FolderWatcher(fs::path folder_path, Config config)
    : _path(std::move(folder_path)), _folder_last_change(time_of_last_change(_path)), _config(config)
{
    update_path_validity();
};

void FolderWatcher::update(Callbacks const& callbacks) const
{
    if (has_check_too_recently())
        return;

    bool const was_valid = _path_validity;
    update_path_validity();

    if (!_path_validity)
    {
        if (was_valid)
            on_folder_path_invalid(callbacks);
        return;
    }

    /*if (!was_valid && !_path_validity)
        return;

    if (was_valid && !_path_validity) {
        on_folder_path_invalid(callbacks);
        return;
    }*/

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
            on_changed_file(callbacks, file);
            continue;
        }
    }

    remove_files(callbacks, will_be_removed);
    check_for_new_paths(callbacks);
    _folder_last_change = time_of_last_change(_path);
}

[[maybe_unused]] void FolderWatcher::set_folder_path(Callbacks const& callbacks, fs::path path)
{
    _path = std::move(path);
    update_path_validity();
    remove_files(callbacks, _files);
};

void FolderWatcher::update_path_validity() const
{
    _path_validity = fs::exists(_path);
}

void FolderWatcher::check_for_new_paths(Callbacks const& callbacks) const
{
    if (_config.watch_all_subfolders_recursively)
    {
        for (const auto& entry : RecursiveDirectoryIterator(_path))
        {
            add_to_files(callbacks, entry);
        }
    }

    else
    {
        for (const auto& entry : DirectoryIterator(_path))
        {
            add_to_files(callbacks, entry);
        }
    }
}

void FolderWatcher::remove_files(Callbacks const& callbacks, std::vector<File>& will_be_removed) const
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
        on_removed_file(callbacks, file);
}

void FolderWatcher::add_to_files(Callbacks const& callbacks, const fs::directory_entry& entry) const
{
    if (entry.is_directory())
        return;

    // If the file is not found in _files, we add it
    auto const file_iterator = std::find_if(_files.begin(), _files.end(), [entry](File const& file) { return file.path == entry; });
    if (file_iterator == _files.end())
        on_added_file(callbacks, entry.path());
}

auto FolderWatcher::has_check_too_recently() const -> bool
{
    static auto last_check   = Clock::now();
    const auto  now          = Clock::now();
    const auto  elapsed_time = std::chrono::duration<float>{now - last_check};

    if (elapsed_time.count() < _config.seconds_between_checks)
        return true;

    last_check = now;
    return false;
}

void FolderWatcher::on_added_file(Callbacks const& callbacks, const fs::path& path) const
{
    if (fs::is_directory(path))
        return;

    _files.push_back({.path = path, .time_of_last_change = time_of_last_change(path)});
    callbacks.on_file_added(_files.back().path.string());
}

void FolderWatcher::on_removed_file(Callbacks const& callbacks, File const& file) const
{
    callbacks.on_file_removed(file.path.string());
    _files.erase(std::remove_if(_files.begin(), _files.end(), [file](const File& cur_file) { return file == cur_file; }), _files.end());
}

void FolderWatcher::on_changed_file(Callbacks const& callbacks, File& file) const
{
    file.time_of_last_change = time_of_last_change(file.path);
    callbacks.on_file_changed(file.path.string());
}

void FolderWatcher::on_folder_path_invalid(Callbacks const& callbacks) const
{
    callbacks.on_invalid_folder_path(_path.string());
    remove_files(callbacks, _files);
}

} // namespace folder_watcher
