#pragma once

#include <filesystem>
#include <folder_watcher/folder_watcher.hpp>
#include <functional>
#include <utility>
#include <variant>

namespace folder_watcher {

struct File {
    std::filesystem::path           path;
    std::filesystem::file_time_type time_of_last_change;
    std::filesystem::file_time_type time_of_last_check;

    auto operator<(const File& a) const -> bool
    {
        return path.string() < a.path.string();
    }
};

struct FolderWatcher_Config {
    bool recursive_watcher = true;
    float delay_between_checks = 0.5f;
};

struct FolderWatcher_Callbacks {
    std::function<void(std::string_view)> on_file_added;
    std::function<void(std::string_view)> on_file_removed;
    std::function<void(std::string_view)> on_file_changed;

    std::function<void(std::string_view)> on_folder_path_invalid;
};

class FolderWatcher {
public:
    explicit FolderWatcher(std::filesystem::path folder_path = {}, FolderWatcher_Config = {});
    void update(FolderWatcher_Callbacks);
    void refresh_files();

public:
    [[nodiscard]] inline constexpr auto folder_path_is_valid() const -> bool { return std::holds_alternative<Valid>(_path_validity); };
    [[nodiscard]] inline auto           folder_path() const -> std::filesystem::path { return _path; }
    inline void                         set_path(std::filesystem::path path);

private:
    void add_path_to_files(std::filesystem::path path);

private:
    void on_file_added(File&, const FolderWatcher_Callbacks&);
    void on_file_removed(File&, const FolderWatcher_Callbacks&);
    void on_file_changed(File&, const FolderWatcher_Callbacks&);

    void on_folder_path_invalid(const FolderWatcher_Callbacks&);

private:
    struct Valid {};
    struct Invalid {};
    struct Unknown {};
    using PathValidity = std::variant<Valid, Invalid, Unknown>;

private:
    std::filesystem::path _path;
    PathValidity          _path_validity = Unknown{};
    FolderWatcher_Config  _config;
    std::vector<File>     _files{};
};

} // namespace folder_watcher
