#pragma once

#include <filesystem>
#include <functional>
#include <utility>
#include <variant>
#include <vector>
#include "./File.hpp"
#include "./utils.hpp"

namespace folder_watcher {

using Clock = std::chrono::steady_clock;

struct Config {
    /// If false, only watches the files that are direct childs of the watched path.
    /// If true, all the files in the subfolders will also be watched.
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

namespace fs = std::filesystem;

class FolderWatcher {
public:
    explicit FolderWatcher(fs::path folder_path = {}, Config = {});

    /// `update()` needs to be called every tick.
    /// It will call the callbacks whenever an event occurs.
    void update(Callbacks const& = {}) const;

    [[nodiscard]] auto is_folder_path_valid() const -> bool { return _path_exists; };
    [[nodiscard]] auto get_folder_path() const -> fs::path const& { return _path; }
    void               set_folder_path(Callbacks const&, fs::path const&);

private:
    auto has_invalid_folder_path(Callbacks const&) const -> bool;

    void check_and_store_path_existence() const;

    void watch_for_edit_and_remove(Callbacks const&) const;

    /// This method compares the content of the folder and the content of the _files vector.
    void watch_for_new_paths(Callbacks const&) const;

    /// Add a path to the _files attribute by calling `on_added_file` method.
    void add_to_files_if_necessary(Callbacks const&, const fs::directory_entry&) const;

    /// Removes a vector of `File`s from _files.
    void remove_files(Callbacks const&, std::vector<File>& to_remove) const;

    /// Verifies is the update method should be executed or not.
    /// It uses the Config::delay_between_checks
    [[nodiscard]] auto has_checked_too_recently() const -> bool;

private:
    fs::path                   _path{};
    mutable fs::file_time_type _folder_last_change{};
    mutable Clock::time_point  _folder_last_check{};
    mutable bool               _path_exists{};
    Config                     _config{};
    mutable std::vector<File>  _files{};
};

} // namespace folder_watcher
