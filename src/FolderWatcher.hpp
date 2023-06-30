#pragma once

#include <filesystem>
#include <functional>
#include <vector>
#include "utils.hpp"

namespace folder_watcher {

namespace internal {

struct FileEntry {
    std::filesystem::path           path;
    std::filesystem::file_time_type last_write_time;
};

} // namespace internal

struct Config {
    /// If false, only watches the files that are direct childs of the watched path.
    /// If true, all the files in all the subfolders will also be watched.
    bool watch_all_subfolders_recursively = true;

    /// Delay between two checks for changes.
    /// The smaller the delay the bigger the performance cost.
    float seconds_between_checks = 0.5f;
};

struct Callbacks {
    std::function<void(std::filesystem::path)> on_file_added;
    std::function<void(std::filesystem::path)> on_file_removed;
    std::function<void(std::filesystem::path)> on_file_changed;
    std::function<void(std::filesystem::path)> on_invalid_folder_path;
};

using Clock  = std::chrono::steady_clock;
namespace fs = std::filesystem;

class FolderWatcher {
public:
    explicit FolderWatcher(fs::path folder_path = {}, Config = {});

    /// `update()` needs to be called every tick.
    /// It will call the corresponding callback whenever an event occurs.
    void update(Callbacks const&) const;

    [[nodiscard]] auto is_folder_path_valid() const -> bool { return _path_exists; };
    [[nodiscard]] auto get_folder_path() const -> fs::path const& { return _path; }
    void               set_folder_path(Callbacks const&, fs::path const&);

private:
    void check_and_store_path_existence() const;

    /// Verifies is the update method should be executed or not.
    /// It uses the Config::delay_between_checks
    [[nodiscard]] auto has_checked_too_recently() const -> bool;

    /// Call callback `on_invalid_folder_path()` if necessary
    void update_folder_path_validity(Callbacks const&) const;

    // This method watches into the folder content to detects edit and remove.
    void watch_for_edit_and_remove(Callbacks const&) const;

    /// This method compares the content of the folder and the content of the `_files` vector.
    void watch_for_new_paths(Callbacks const&) const;

    /// Add a path to the `_files` attribute
    void add_to_files_if_necessary(Callbacks const&, const fs::directory_entry&) const;

    /// Removes a vector of `File`s from _files.
    void remove_files(Callbacks const&, std::vector<internal::FileEntry> const& to_remove) const;

private:
    fs::path                                 _path{};
    mutable Clock::time_point                _folder_last_check{};
    mutable bool                             _path_exists{};
    Config                                   _config{};
    mutable std::vector<internal::FileEntry> _files{};
};

} // namespace folder_watcher
