#pragma once

#include <filesystem>
#include <folder_watcher/folder_watcher.hpp>
#include <functional>
#include <utility>
#include <variant>

namespace folder_watcher {
namespace fs = std::filesystem;

static auto time_of_last_change(const std::filesystem::path& path) -> std::filesystem::file_time_type;

struct File {
    fs::path           path;                // NOLINT
    fs::file_time_type time_of_last_change; // NOLINT
    fs::file_time_type time_of_last_check;  // NOLINT

    auto operator==(const File& a) const -> bool
    {
        return path.string() == a.path.string();
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
    void update();

public:
    [[nodiscard]] inline constexpr auto        is_folder_path_invalid() const -> bool { return std::holds_alternative<Invalid>(_path_validity); };
    [[nodiscard]] inline constexpr auto        is_folder_path_valid() const -> bool { return std::holds_alternative<Valid>(_path_validity); };
    [[maybe_unused]] [[nodiscard]] inline auto folder_path() const -> fs::path { return _path; }
    [[maybe_unused]] inline void               set_callbacks(FolderWatcher_Callbacks callbacks) { _callbacks = std::move(callbacks); };
    [[maybe_unused]] inline void               set_path(fs::path);

private:
    void               init_files();
    void               check_for_new_paths();
    void               remove_files(std::vector<File>& will_be_removed);
    void               check_for_added_files(const fs::directory_entry&);
    [[nodiscard]] auto hasCheckTooRecently() const -> bool;

private:
    void on_added_file(const fs::path&);
    void on_removed_file(File const&);
    void on_changed_file(File&);

    void on_folder_path_invalid();

private:
    struct Valid {};
    struct Invalid {};
    struct Unknown {};
    using PathValidity = std::variant<Valid, Invalid, Unknown>;

private:
    fs::path                _path{};
    fs::file_time_type      _folder_last_change{};
    PathValidity            _path_validity = Unknown{};
    FolderWatcher_Config    _config{};
    FolderWatcher_Callbacks _callbacks{};
    std::vector<File>       _files{};
};

} // namespace folder_watcher
