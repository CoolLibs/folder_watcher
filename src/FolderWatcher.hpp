#pragma once

#include <filesystem>
#include <folder_watcher/folder_watcher.hpp>
#include <functional>
#include <utility>
#include <variant>
#include <vector>

namespace folder_watcher {
namespace fs = std::filesystem;

struct File {
    fs::path           path;                // NOLINT
    fs::file_time_type time_of_last_change; // NOLINT

    auto operator==(const File& a) const -> bool
    {
        return path == a.path;
    }
};

struct FolderWatcher_Config {
    bool  recursive_watcher    = true;
    float delay_between_checks = 0.5f;
};

struct FolderWatcher_Callbacks {
    std::function<void(std::string_view)> on_added_file;
    std::function<void(std::string_view)> on_removed_file;
    std::function<void(std::string_view)> on_changed_file;

    std::function<void(std::string_view)> on_invalid_folder_path;
};

class FolderWatcher {
public:
    explicit FolderWatcher(fs::path folder_path = {}, FolderWatcher_Callbacks = {}, FolderWatcher_Config = {});

    /// `update` needs to be call every tick.
    /// This method is listening for changes inside the `folder_path`
    void update() const;

public:
    [[nodiscard]] inline constexpr auto        is_folder_path_invalid() const -> bool { return std::holds_alternative<Invalid>(_path_validity); };
    [[nodiscard]] inline constexpr auto        is_folder_path_valid() const -> bool { return std::holds_alternative<Valid>(_path_validity); };
    [[maybe_unused]] [[nodiscard]] inline auto folder_path() const -> fs::path { return _path; }
    [[maybe_unused]] inline void               set_callbacks(FolderWatcher_Callbacks callbacks) { _callbacks = std::move(callbacks); };
    [[maybe_unused]] inline void               set_path(fs::path);

private:
    /// This method should only be called in the constructor.
    /// Its goal is to fill the _files attribute.
    /// It also check for invalid directory path.
    void init_files();

    /// This method compares the content of the folder and the content of the _files vector.
    /// If a file is not listed in _files, we add it by calling `add_to_files`
    void check_for_new_paths() const;

    /// Removes a vector of <File> from the _files attribute.
    void remove_files(std::vector<File>& will_be_removed) const;

    /// Add a path to the _files attribute by calling `on_added_file` method.
    void add_to_files(const fs::directory_entry&) const;

    /// Verifies is the update method should be executed or not.
    /// It uses the FolderWatcher_Config::delay_between_checks
    [[nodiscard]] auto hasCheckTooRecently() const -> bool;

    /// These methods call the corresponding callback.
    void on_added_file(fs::path const&) const;
    void on_removed_file(File const&) const;
    void on_changed_file(File&) const;
    void on_folder_path_invalid() const;

private:
    struct Valid {};
    struct Invalid {};
    struct Unknown {};
    using PathValidity = std::variant<Valid, Invalid, Unknown>;

private:
    fs::path                   _path{};
    mutable fs::file_time_type _folder_last_change{};
    mutable PathValidity       _path_validity = Unknown{};
    FolderWatcher_Config       _config{};
    FolderWatcher_Callbacks    _callbacks{};
    mutable std::vector<File>  _files{};
};

} // namespace folder_watcher
