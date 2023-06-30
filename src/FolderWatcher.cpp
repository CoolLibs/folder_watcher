#include "./FolderWatcher.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>

namespace folder_watcher {

FolderWatcher::FolderWatcher(fs::path folder_path, Config config)
    : _path{std::move(folder_path)}
    , _config{config}
{
    check_and_store_path_existence();
};

void FolderWatcher::update(Callbacks const& callbacks) const
{
    if (has_checked_too_recently())
        return;

    update_folder_path_validity(callbacks);
    if (!is_folder_path_valid())
        return;

    watch_for_edit_and_remove(callbacks);

    watch_for_new_paths(callbacks);
}

void FolderWatcher::set_folder_path(Callbacks const& callbacks, fs::path const& path)
{
    _path = path;
    check_and_store_path_existence();
    remove_files(callbacks, _files); // Clear all files because the path changed and we might not need to watch them anymore.
};

void FolderWatcher::check_and_store_path_existence() const
{
    _path_exists = fs::exists(_path);
}

auto FolderWatcher::has_checked_too_recently() const -> bool
{
    const auto now          = Clock::now();
    const auto elapsed_time = std::chrono::duration<float>{now - _folder_last_check};

    if (elapsed_time.count() < _config.seconds_between_checks)
        return true;

    _folder_last_check = now;
    return false;
}

void FolderWatcher::update_folder_path_validity(Callbacks const& callbacks) const
{
    bool const was_valid = _path_exists;
    check_and_store_path_existence();

    if (!_path_exists && was_valid)
    {
        callbacks.on_invalid_folder_path(_path);
        remove_files(callbacks, _files);
    }
}

void FolderWatcher::watch_for_edit_and_remove(Callbacks const& callbacks) const
{
    auto to_remove = std::vector<internal::FileEntry>{};
    for (auto& file : _files)
    {
        // File deleted
        if (!fs::exists(file.path))
        {
            to_remove.push_back(file);
            continue;
        }

        // File changed
        auto const last_change = compute_time_of_last_change(file.path);
        if (last_change != file.last_write_time)
        {
            file.last_write_time = last_change;
            callbacks.on_file_changed(file.path);
        }
    }
    remove_files(callbacks, to_remove);
}

void FolderWatcher::watch_for_new_paths(Callbacks const& callbacks) const
{
    if (_config.watch_all_subfolders_recursively)
    {
        for (auto const& entry : fs::recursive_directory_iterator(_path))
            add_to_files_if_necessary(callbacks, entry);
    }
    else
    {
        for (auto const& entry : fs::directory_iterator(_path))
            add_to_files_if_necessary(callbacks, entry);
    }
}

void FolderWatcher::add_to_files_if_necessary(Callbacks const& callbacks, fs::directory_entry const& entry) const
{
    if (!entry.is_regular_file() && !entry.is_symlink())
        return;

    // If the file is not found in _files, we add it
    auto const file_iterator = std::find_if(_files.begin(), _files.end(), [&entry](internal::FileEntry const& file) { return file.path == entry.path(); });
    if (file_iterator != _files.end())
        return;

    _files.emplace_back(entry);
    callbacks.on_file_added(entry);
}

void FolderWatcher::remove_files(Callbacks const& callbacks, std::vector<internal::FileEntry> const& to_remove) const
{
    for (internal::FileEntry const& file : to_remove)
        callbacks.on_file_removed(file.path);

    // Erase all common files in to_remove and in _files
    std::erase_if(_files, [&to_remove](internal::FileEntry const& f) {
        return std::find_if(to_remove.begin(), to_remove.end(), [&](auto const& file) { return file.path == f.path; }) != to_remove.end();
    });
}

} // namespace folder_watcher
