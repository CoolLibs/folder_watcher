#include "./FolderWatcher.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <utility>

namespace folder_watcher {

namespace fs                     = std::filesystem;
using RecursiveDirectoryIterator = std::filesystem::recursive_directory_iterator;
using DirectoryIterator          = std::filesystem::directory_iterator;
using Clock                      = std::chrono::steady_clock;

// move to utils file ? put back the try/catch ?
// ToDo use it
static auto get_time_of_last_change(const std::filesystem::path& p_path) noexcept -> std::filesystem::file_time_type
{
    return std::filesystem::last_write_time(p_path);
}

FolderWatcher::FolderWatcher(std::filesystem::path folder_path, FolderWatcher_Config config)
    : _path(std::move(folder_path)), _folder_last_change(get_time_of_last_change(_path)), _config(config)
{
    refresh_files();
};

void FolderWatcher::update(const FolderWatcher_Callbacks& p_callbacks)
{
    // checkValidity()
    // ToDo: Ou alors si le dossier existe pas on le créer ?
    // ToDo state
    if (!std::filesystem::exists(_path))
        return;

    // last_check is static so it won't update every tick
    {
        static auto last_check   = Clock::now();
        const auto  now          = Clock::now();
        const auto  elapsed_time = std::chrono::duration<float>{now - last_check};

        if (elapsed_time.count() < _config.delay_between_checks)
            return;
        last_check = now;
    }
    //auto const a = get_time_of_last_change(_path);
    //if ( a == _folder_last_change)
    //     return;

    std::vector<File> will_be_removed{};
    for (File& file : _files)
    {
        // Files deleted
        if (!std::filesystem::exists(file.path))
        {
            will_be_removed.push_back(file);
            continue;
        }

        // File changed
        const auto last_change = get_time_of_last_change(file.path);
        if (last_change != file.time_of_last_change)
        {
            on_changed_file(file, p_callbacks);
            continue;
        }
    }

    remove_files(p_callbacks, will_be_removed);
    check_for_new_paths(p_callbacks);
    _folder_last_change = get_time_of_last_change(_path);
}

void FolderWatcher::refresh_files()
{
    // sort isn't that useful ? remove operator
    // std::sort(_files.begin(), _files.end());
    //ToDO callbacks
    if (_config.recursive_watcher)
    {
        for (const auto& entry : RecursiveDirectoryIterator(_path))
            add_path_to_files(entry.path());
    }
    else
    {
        for (const auto& entry : DirectoryIterator(_path))
            add_path_to_files(entry.path());
    }
}

void FolderWatcher::set_path(std::filesystem::path path)
{
    _path          = std::move(path);
    _path_validity = Unknown{};
};

void FolderWatcher::add_path_to_files(std::filesystem::path const& path)
{
    if (std::filesystem::is_regular_file(path))
        _files.push_back({.path = path, .time_of_last_change = get_time_of_last_change(path)});
}

void FolderWatcher::check_for_new_paths(const FolderWatcher_Callbacks& p_callbacks)
{
    if (_config.recursive_watcher)
    {
        for (const auto& entry : RecursiveDirectoryIterator(_path))
        {
            check_for_added_files(p_callbacks, entry);
        }
    }

    else
    {
        for (const auto& entry : DirectoryIterator(_path))
        {
            check_for_added_files(p_callbacks, entry);
        }
    }
}

void FolderWatcher::remove_files(const FolderWatcher_Callbacks& p_callbacks, std::vector<File>& will_be_removed)
{
    if (will_be_removed.empty())
        return;

    std::vector<File> new_files;

    // TODO std::filter ?
    std::set_difference(_files.begin(), _files.end(), will_be_removed.begin(), will_be_removed.end(), std::back_inserter(new_files));
    _files = new_files;

    for (File const& file : will_be_removed)
        on_removed_file(file, p_callbacks);
}

void FolderWatcher::check_for_added_files(const FolderWatcher_Callbacks& p_callbacks, std::filesystem::directory_entry entry)
{
    if (!entry.is_regular_file())
        return;

    // If the file is not found in _files
    auto const file_iterator = std::find_if(_files.begin(), _files.end(), [entry](File const& file) { return file.path == entry; });
    if (file_iterator == _files.end())
        on_added_file(entry.path(), p_callbacks);
}

// répétition, on peut refacto ?
void FolderWatcher::on_added_file(const std::filesystem::path& p_path, const FolderWatcher_Callbacks& p_callbacks)
{
    add_path_to_files(p_path);
    p_callbacks.on_added_file(_files.back().path.string());
}

void FolderWatcher::on_removed_file(File const& p_file, const FolderWatcher_Callbacks& p_callbacks)
{
    p_callbacks.on_removed_file(p_file.path.string());
    _files.erase(std::remove_if(_files.begin(), _files.end(), [p_file](const File& cur_file) { return p_file == cur_file; }), _files.end());
}

void FolderWatcher::on_changed_file(File& p_file, const FolderWatcher_Callbacks& p_callbacks)
{
    _path_validity             = Valid{};
    p_file.time_of_last_change = get_time_of_last_change(p_file.path);
    p_callbacks.on_changed_file(p_file.path.string());
}

void FolderWatcher::on_folder_path_invalid(const FolderWatcher_Callbacks& p_callbacks)
{
    _path_validity = Invalid{};
    p_callbacks.on_invalid_folder_path(_path.string());
}

} // namespace folder_watcher
