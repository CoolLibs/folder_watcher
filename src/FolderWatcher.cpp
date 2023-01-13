#include "./FolderWatcher.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <utility>

namespace folder_watcher {

using RecursiveDirectoryIterator = std::filesystem::recursive_directory_iterator;
using DirectoryIterator          = std::filesystem::directory_iterator;
using Clock                      = std::chrono::steady_clock;

// move to utils file ? put back the try/catch ?
static auto get_time_of_last_change(const std::filesystem::path& p_path) noexcept -> std::filesystem::file_time_type
{
    return std::filesystem::last_write_time(p_path);
}

FolderWatcher::FolderWatcher(std::filesystem::path folder_path, FolderWatcher_Config config)
    : _path(std::move(folder_path)), _config(config)
{
    refresh_files();
};

void FolderWatcher::update(const FolderWatcher_Callbacks p_callbacks)
{
    //checkValidity()

    // Ou alors si le dossier existe pas on le créer ?
    if (!std::filesystem::exists(_path))
        return;

    // Last check is static so it won't update every tick
    static auto last_check   = Clock::now();
    const auto  now          = Clock::now();
    const auto  elapsed_time = std::chrono::duration<float>{now - last_check};

    if (elapsed_time.count() < _config.delay_between_checks)
        return;

    last_check = now;
    for (File& file : _files)
    {
        const auto last_change = get_time_of_last_change(file.path);
        if (last_change != file.time_of_last_change) {
            on_file_changed(file, p_callbacks);
        }
    }
}

void FolderWatcher::refresh_files()
{
    // sort isn't that useful ? remove operator
    // std::sort(_files.begin(), _files.end());
    if (_config.recursive_watcher)
    {
        for (const auto& entry : RecursiveDirectoryIterator(_path))
        {
            add_path_to_files(entry.path());
        }
    }
    else
    {
        for (const auto& entry : DirectoryIterator(_path))
        {
            add_path_to_files(entry.path());
        }
    }
}

void FolderWatcher::set_path(std::filesystem::path path)
{
    _path          = std::move(path);
    _path_validity = Unknown{};
};

void FolderWatcher::add_path_to_files(const std::filesystem::path path)
{
    if (std::filesystem::is_regular_file(path))
    {
        _files.push_back({.path = path, .time_of_last_change = get_time_of_last_change(path)});
    }
}

// répétition, on peut refacto ?
void FolderWatcher::on_file_added(File& p_file, const FolderWatcher_Callbacks& p_callbacks)
{
    p_callbacks.on_file_added(p_file.path.string());
}

void FolderWatcher::on_file_removed(File& p_file, const FolderWatcher_Callbacks& p_callbacks)
{
    p_callbacks.on_file_removed(p_file.path.string());
}

void FolderWatcher::on_file_changed(File& p_file, const FolderWatcher_Callbacks& p_callbacks)
{
    _path_validity = Valid{};
    p_file.time_of_last_change = get_time_of_last_change(_path);
    p_callbacks.on_file_changed(p_file.path.string());
}

void FolderWatcher::on_folder_path_invalid(const FolderWatcher_Callbacks& p_callbacks)
{
    _path_validity = Invalid{};
    p_callbacks.on_folder_path_invalid(_path.string());
}

} // namespace folder_watcher
