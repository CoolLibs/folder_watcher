#pragma once

#include <filesystem>

namespace folder_watcher {

/// Compute the last_write_time of a file or a directory.
/// This method ignores the exceptions.
inline auto compute_time_of_last_change(std::filesystem::path const& path) -> std::filesystem::file_time_type
{
    try
    {
        return std::filesystem::last_write_time(path);
    }
    catch (...)
    {
        return {};
    }
}

} // namespace folder_watcher