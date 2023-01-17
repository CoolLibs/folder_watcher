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

The library is really use to use. First of all, create a `FolderWatcher` instance. A `FolderWatcher` needs a path (for the folder who will be watched), some callbacks (in order to react to the changes) and some configurations if needed.

The callbacks can be created as following :
```c++
folder_watcher::FolderWatcher_Callbacks callbacks{
    .on_added_file = [](std::string_view path) { std::cout << "File added : " << path << std::endl; },
    .on_removed_file = [](std::string_view path) { std::cout << "File removed : " << path << std::endl; },
    .on_changed_file = [](std::string_view path) { std::cout << "File changed : " << path << std::endl; },
    .on_invalid_folder_path = [](std::string_view path) { std::cout << "Folder path invalid : " << path << std::endl; }
};
```

The default configuration is the following :

```c++
folder_watcher::FolderWatcher_Config config{
    .recursive_watcher = true,
    .delay_between_checks = 0.5f
};
```

You can still easily tweak it as your wish.

Now, you need to create your Folder Watcher :

```c++
folder_watcher::FolderWatcher folder_watcher{your_watched_path, callbacks, config};
```

You can create it as following if you're not modifying the configuration :

```c++
folder_watcher::FolderWatcher folder_watcher{your_watched_path, callbacks};
```


Finally, you just need to call the `update` method in your main loop.

```c++
folder_watcher.update();
```