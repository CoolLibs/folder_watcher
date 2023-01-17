#pragma once

#include <filesystem>

namespace folder_watcher {
/// Get the last_write_time of a file or a directory
/// This method ignore the exceptions
static auto time_of_last_change(const std::filesystem::path& path) -> std::filesystem::file_time_type
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