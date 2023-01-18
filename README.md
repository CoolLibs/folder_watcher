# folder_watcher

[![Build](https://github.com/CoolLibs/folder_watcher/actions/workflows/build.yml/badge.svg)](https://github.com/CoolLibs/folder_watcher/actions/workflows/build.yml)

**folder_watcher** is a standalone library that listens the changes [add, edit or remove] of files inside a folder, recursively, or not.    

## Table of contents

- [Table of contents](#table-of-contents)
- [Compatibility](#compatibility)
- [Tutorial](#tutorial)
  - [Including](#including)
  - [Basic usage](#basic-usage)

## Compatibility

This library is tested and compiles with C++20 on:
- [x] Windows
    - [x] Clang
    - [x] MSVC
- [x] Linux
    - [x] Clang
    - [x] GCC
- [x] MacOS
    - [x] Clang

## Tutorial

### Including

To add this library to your project, simply add these two lines to your *CMakeLists.txt*:
```cmake
add_subdirectory(path/to/folder_watcher)
target_link_libraries(${PROJECT_NAME} PRIVATE folder_watcher::folder_watcher)
```

Then include it as:
```cpp
#include <folder_watcher/folder_watcher.hpp>
```

### Basic usage

The library is really use to use. First of all, create a `FolderWatcher` instance. A `FolderWatcher` needs a path (for the watched folder) and some configurations. You will also need to provide some callbacks (in order to react to the changes) to the update() method.

The callbacks can be created as following :
```c++
auto const callbacks = folder_watcher::Callbacks{
    .on_added_file = [](std::filesystem::path path) { std::cout << "File added : " << path.string() << std::endl; },
    .on_removed_file = [](std::filesystem::path path) { std::cout << "File removed : " << path.string() << std::endl; },
    .on_changed_file = [](std::filesystem::path path) { std::cout << "File changed : " << path.string() << std::endl; },
    .on_invalid_folder_path = [](std::filesystem::path path) { std::cout << "Folder path invalid : " << path.string() << std::endl; }
};
```

The default configuration is the following :

```c++
auto const config = folder_watcher::Config{
    .seconds_between_checks = 0.5f,
    .watch_all_subfolders_recursively = true
};
```

You can still easily tweak it as you wish.

Now, you need to create your Folder Watcher :

```c++
auto folder_watcher = folder_watcher::FolderWatcher{watched_path, config};
```

Another clean initialization is the following :
 ```c++
auto folder_watcher = folder_watcher::FolderWatcher{watched_path, {
    .seconds_between_checks = 0.5f,
    .watch_all_subfolders_recursively = false}
};
 ```

If you're not modifying the default configuration, you can create the folder watcher as following :

```c++
auto folder_watcher = folder_watcher::FolderWatcher{watched_path};
```


Finally, you just need to call the `update` method in your main loop.

```c++
folder_watcher.update(callbacks);
```