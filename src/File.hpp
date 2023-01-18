#pragma once

#include <filesystem>
#include "utils.hpp"

namespace folder_watcher {

namespace fs = std::filesystem;

class File {
public:
    explicit File(fs::path const& path)
        : _path{path}, _time_of_last_change{get_time_of_last_change(path)} {};
    [[nodiscard]] auto path() const -> fs::path { return _path; };
    [[nodiscard]] auto time_of_last_change() const -> fs::file_time_type { return _time_of_last_change; };
    void               time_of_last_change(fs::file_time_type time_of_last_change) { _time_of_last_change = time_of_last_change; };

private:
    fs::path           _path;
    fs::file_time_type _time_of_last_change;
    friend auto        operator==(File const& a, File const& b) -> bool { return a._path == b._path; }
};
} // namespace folder_watcher
