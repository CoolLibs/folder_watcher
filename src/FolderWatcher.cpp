#include "./FolderWatcher.hpp"
#include <algorithm>
#include <compare>
#include <filesystem>
#include <iostream>

namespace folder_watcher {

FolderWatcher::FolderWatcher(std::filesystem::path folder_path, Config config)
    : _watched_folder_path{std::move(folder_path)}
    , _config{config}
{
    check_and_store_path_existence();
};

static auto compute_sorted_files_entries(
    std::filesystem::path const& folder_path,
    bool                         check_recursively
) -> std::vector<internal::FileEntry>
{
    auto files_entries = std::vector<internal::FileEntry>{};
    {
        auto const on_entry = [&](std::filesystem::directory_entry const& entry) {
            if (!entry.is_regular_file() && !entry.is_symlink())
                return;
            files_entries.emplace_back(internal::FileEntry{entry.path(), entry.last_write_time()});
        };

        if (check_recursively)
        {
            for (auto const& entry : std::filesystem::recursive_directory_iterator(folder_path))
                on_entry(entry);
        }
        else
        {
            for (auto const& entry : std::filesystem::directory_iterator(folder_path))
                on_entry(entry);
        }
    }

    std::sort(files_entries.begin(), files_entries.end(), [](internal::FileEntry const& a, internal::FileEntry const& b) {
        return a.path < b.path; // Important to sort this way because our update() algorithm will expect the vectors to be sorted like that.
    });

    return files_entries;
}

void FolderWatcher::update(Callbacks const& callbacks)
{
    if (has_checked_too_recently())
        return;

    update_folder_path_validity(callbacks);
    if (!is_folder_path_valid())
        return;

    auto const current_sorted_files_entries = compute_sorted_files_entries(_watched_folder_path, _config.watch_all_subfolders_recursively);
    if (_previous_sorted_files_entries == current_sorted_files_entries)
        return; // Early return to optimize the most common case: no files have changed.

    auto it_previous = _previous_sorted_files_entries.begin();
    auto it_current  = current_sorted_files_entries.begin();
    while (it_previous != _previous_sorted_files_entries.end()
           && it_current != current_sorted_files_entries.end())
    {
        auto const comp = it_previous->path.string() <=> it_current->path.string(); // NB: we use <=> on the .string() because MacOS doesn't have it for std::filesystem::path just yet.
        // File exists in Previous but not in Current
        if (comp == std::strong_ordering::less)
        {
            callbacks.on_file_removed(it_previous->path);
            it_previous++;
            continue;
        }
        // File exists in Current but not in Previous
        if (comp == std::strong_ordering::greater)
        {
            callbacks.on_file_added(it_current->path);
            it_current++;
            continue;
        }
        // File exists in both Previous and Current
        if (it_previous->last_write_time != it_current->last_write_time)
            callbacks.on_file_changed(it_current->path);
        ++it_previous;
        ++it_current;
    }
    while (it_previous != _previous_sorted_files_entries.end())
    {
        // File exists in Previous but not in Current
        callbacks.on_file_removed(it_previous->path);
        it_previous++;
    }
    while (it_current != current_sorted_files_entries.end())
    {
        // File exists in Current but not in Previous
        callbacks.on_file_added(it_current->path);
        it_current++;
    }

    _previous_sorted_files_entries = current_sorted_files_entries;
}

void FolderWatcher::set_folder_path(std::filesystem::path const& path)
{
    _watched_folder_path = path;
    check_and_store_path_existence();
};

void FolderWatcher::check_and_store_path_existence()
{
    _watched_folder_exists = std::filesystem::exists(_watched_folder_path);
}

auto FolderWatcher::has_checked_too_recently() -> bool
{
    auto const now          = Clock::now();
    auto const elapsed_time = std::chrono::duration<float>{now - _folder_last_check};

    if (elapsed_time.count() < _config.seconds_between_checks)
        return true;

    _folder_last_check = now;
    return false;
}

void FolderWatcher::update_folder_path_validity(Callbacks const& callbacks)
{
    bool const was_valid = _watched_folder_exists;
    check_and_store_path_existence();

    if (!_watched_folder_exists && was_valid)
    {
        callbacks.on_invalid_folder_path(_watched_folder_path);
        for (auto const& file : _previous_sorted_files_entries)
            callbacks.on_file_removed(file.path);
        _previous_sorted_files_entries = {};
    }
}

} // namespace folder_watcher
