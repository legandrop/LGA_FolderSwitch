cmake_minimum_required(VERSION 3.16)
project(LGA_FolderSwitch VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(WIN32 AND NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.5.3/mingw_64")

find_package(Qt6 COMPONENTS Core Gui Widgets Svg REQUIRED)

set(RESOURCES
    resources/resources.qrc
)

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/LGA_FolderSwitch.ico")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_icon.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windows_icon.rc" @ONLY)
    # El .rc solo NOMBRA al .ico, asi que cambiar el icono no cambia el .rc y
    # windres no se volvia a correr: el exe se quedaba con el icono viejo.
    # OBJECT_DEPENDS ata el .rc al .ico para que se recompile cuando cambia.
    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/windows_icon.rc"
        PROPERTIES OBJECT_DEPENDS "${APP_ICON_RESOURCE_WINDOWS}")
    set(RESOURCES ${RESOURCES} "${CMAKE_CURRENT_BINARY_DIR}/windows_icon.rc")
endif()

set(SOURCES
    src/main.cpp
    src/tray/TrayController.cpp
    src/ui/MainWindow.cpp
    src/core/ForegroundWatcher.cpp
    src/core/FolderResolver.cpp
    src/core/DialogSwitcher.cpp
    src/core/UiaSwitcher.cpp
    src/core/HotkeyFilter.cpp
    src/core/WindowUtils.cpp
    src/windows/AutoStart.cpp
    src/windows/RegistryHelper.cpp
)

set(HEADERS
    src/tray/TrayController.h
    src/ui/MainWindow.h
    src/core/ForegroundWatcher.h
    src/core/FolderResolver.h
    src/core/DialogSwitcher.h
    src/core/UiaSwitcher.h
    src/core/HotkeyFilter.h
    src/core/WindowUtils.h
    src/windows/AutoStart.h
    src/windows/RegistryHelper.h
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    shell32
    ole32
    oleaut32
    user32
    uuid
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    FOLDERSWITCH_VERSION="${PROJECT_VERSION}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)
