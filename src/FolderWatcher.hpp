#pragma once
#include <filesystem>
#include <functional>
#include <vector>

namespace folder_watcher {

namespace internal {

struct FileEntry {
    std::filesystem::path           path;
    std::filesystem::file_time_type last_write_time;

    friend auto operator==(FileEntry const&, FileEntry const&) -> bool = default;
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

using Clock = std::chrono::steady_clock;

class FolderWatcher {
public:
    explicit FolderWatcher(std::filesystem::path folder_path = {}, Config = {});

    /// `update()` needs to be called every tick.
    /// It will call the corresponding callback whenever an event occurs.
    void update(Callbacks const&);

    [[nodiscard]] auto is_folder_path_valid() const -> bool { return _watched_folder_exists; };
    [[nodiscard]] auto get_folder_path() const -> std::filesystem::path const& { return _watched_folder_path; }
    void               set_folder_path(std::filesystem::path const&);

private:
    void check_and_store_path_existence();

    /// Verifies is the update method should be executed or not.
    /// It uses the Config::delay_between_checks
    [[nodiscard]] auto has_checked_too_recently() -> bool;

    /// Call callback `on_invalid_folder_path()` if necessary
    void update_folder_path_validity(Callbacks const&);

private:
    std::filesystem::path            _watched_folder_path{};
    Clock::time_point                _folder_last_check{};
    bool                             _watched_folder_exists{};
    Config                           _config{};
    std::vector<internal::FileEntry> _previous_sorted_files_entries{};
};

} // namespace folder_watcher
