# folder_watcher

## Including

To add this library to your project, simply add these two lines to your *CMakeLists.txt*:
```cmake
add_subdirectory(path/to/folder_watcher)
target_link_libraries(${PROJECT_NAME} PRIVATE folder_watcher::folder_watcher)
```

Then include it as:
```cpp
#include <folder_watcher/folder_watcher.hpp>
```

## Running the tests

Simply use "tests/CMakeLists.txt" to generate a project, then run it.<br/>
If you are using VSCode and the CMake extension, this project already contains a *.vscode/settings.json* that will use the right CMakeLists.txt automatically.
