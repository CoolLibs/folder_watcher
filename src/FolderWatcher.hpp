#pragma once

#include <filesystem>
#include <folder_watcher/folder_watcher.hpp>
#include <functional>
#include <utility>
#include <variant>

namespace folder_watcher {

namespace fs = std::filesystem;

struct File {
    fs::path           path;
    fs::file_time_type time_of_last_change;
    fs::file_time_type time_of_last_check;

    auto operator<(const File& a) const -> bool
    {
        return path.string() < a.path.string();
    }

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
    explicit FolderWatcher(fs::path folder_path = {}, FolderWatcher_Config = {});
    void update(const FolderWatcher_Callbacks&);

public:
    [[nodiscard]] inline constexpr auto folder_path_is_valid() const -> bool { return std::holds_alternative<Valid>(_path_validity); };
    [[nodiscard]] inline auto           folder_path() const -> fs::path { return _path; }
    inline void                         set_path(fs::path path);

private:
    void init_files();
    void add_path_to_files(fs::path const&);
    void check_for_new_paths(const FolderWatcher_Callbacks&);
    void remove_files(const FolderWatcher_Callbacks&, std::vector<File>& will_be_removed);
    void check_for_added_files(const FolderWatcher_Callbacks&, const fs::directory_entry&);
    bool hasCheckTooRecently() const;

private:
    void on_added_file(const fs::path&, const FolderWatcher_Callbacks&);
    void on_removed_file(File const&, const FolderWatcher_Callbacks&);
    void on_changed_file(File&, const FolderWatcher_Callbacks&);

    void on_folder_path_invalid(const FolderWatcher_Callbacks&);

private:
    struct Valid {};
    struct Invalid {};
    struct Unknown {};
    using PathValidity = std::variant<Valid, Invalid, Unknown>;

private:
    fs::path             _path{};
    fs::file_time_type   _folder_last_change{};
    PathValidity         _path_validity = Unknown{};
    FolderWatcher_Config _config{};
    std::vector<File>    _files{};
};

} // namespace folder_watcher
