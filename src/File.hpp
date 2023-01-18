#pragma once

#include <filesystem>
#include "utils.hpp"

namespace folder_watcher {

namespace fs = std::filesystem;

class File {
public:
    explicit File(fs::path const& path)
        : _path{path}
        , _time_of_last_change{compute_time_of_last_change(path)}
    {}
    [[nodiscard]] auto get_path() const -> fs::path { return _path; };
    [[nodiscard]] auto get_time_of_last_change() const -> fs::file_time_type { return _time_of_last_change; };
    void               set_time_of_last_change(fs::file_time_type time_of_last_change) { _time_of_last_change = time_of_last_change; };

    friend auto operator==(File const& a, File const& b) -> bool { return a._path == b._path; }

private:
    fs::path           _path;
    fs::file_time_type _time_of_last_change;
};
} // namespace folder_watcher
