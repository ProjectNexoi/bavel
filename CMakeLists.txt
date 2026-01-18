cmake_minimum_required (VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(ftxui_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ftxui/lib/cmake/ftxui)

project(TUIExplorer
  LANGUAGES CXX
  VERSION 1.3.0
)

find_package(ftxui 5 REQUIRED)


add_executable(TUIExplorer
  src/main.cpp
  src/ListItem.cpp
  src/DirectoryReader.cpp
  src/ProcessingFuncs.cpp
)

target_link_libraries(TUIExplorer
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)