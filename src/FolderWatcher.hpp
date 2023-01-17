#pragma once

#include <filesystem>
#include <functional>
#include <utility>
#include <variant>
#include <vector>

namespace folder_watcher {

struct Config {
    /// If false, only watches the files that are direct childs of the watched path.
    /// If true, all the files in the subfolders will also be watched.
    bool watch_all_subfolders_recursively = true;

    /// Delay between two checks for changes.
    /// The smaller the delay the bigger the performance cost.
    float seconds_between_checks = 0.5f;
};

struct Callbacks {
    std::function<void(std::string_view)> on_file_added;
    std::function<void(std::string_view)> on_file_removed;
    std::function<void(std::string_view)> on_file_changed;
    std::function<void(std::string_view)> on_invalid_folder_path;
};

namespace fs = std::filesystem;

class FolderWatcher {
public:
    explicit FolderWatcher(fs::path folder_path = {}, Config = {});

    /// `update()` needs to be called every tick.
    /// It will call the callbacks whenever an event occurs.
    void update(Callbacks const& = {}) const;

    // [[nodiscard]] constexpr auto        is_folder_path_invalid() const -> bool { return !_path_validity; };
    [[nodiscard]] constexpr auto        is_folder_path_valid() const -> bool { return _path_validity; };
    [[maybe_unused]] [[nodiscard]] auto get_folder_path() const -> fs::path { return _path; }
    [[maybe_unused]] void               set_folder_path(Callbacks const&, fs::path);

private:
    struct File {
        fs::path           path;
        fs::file_time_type time_of_last_change;
        friend auto        operator==(File const& a, File const& b) -> bool { return a.path == b.path; }
    };

private:
    /// Check if the folder path exists and set it into _path_validity
    void update_path_validity() const;

    /// This method compares the content of the folder and the content of the _files vector.
    /// If a file is not listed in _files, we add it by calling `add_to_files`
    void check_for_new_paths(Callbacks const&) const;

    /// Removes a vector of <File> from the _files attribute.
    void remove_files(Callbacks const&, std::vector<File>& will_be_removed) const;

    /// Add a path to the _files attribute by calling `on_added_file` method.
    void add_to_files(Callbacks const&, const fs::directory_entry&) const;

    /// Verifies is the update method should be executed or not.
    /// It uses the FolderWatcher_Config::delay_between_checks
    [[nodiscard]] auto has_check_too_recently() const -> bool;

    /// These methods call the corresponding callback.
    void on_added_file(Callbacks const&, fs::path const&) const;
    void on_removed_file(Callbacks const&, File const&) const;
    void on_changed_file(Callbacks const&, File&) const;
    void on_folder_path_invalid(Callbacks const&) const;

private:
    fs::path                   _path{};
    mutable fs::file_time_type _folder_last_change{};
    mutable bool               _path_validity{};
    Config                     _config{};
    mutable std::vector<File>  _files{};
};

} // namespace folder_watcher
