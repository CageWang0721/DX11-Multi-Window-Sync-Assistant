cmake_minimum_required(VERSION 3.16)

# ═══════════════════════════════════════════════════════════
#  工具链 — 必须在 project() 之前设置
# ═══════════════════════════════════════════════════════════
set(QT_ROOT  "C:/Qt/6.11.1/mingw_64")
set(QT_MINGW "C:/Qt/Tools/mingw1310_64")
set(QT_NINJA "C:/Qt/Tools/Ninja/ninja.exe")

# 仅在未由命令行/preset 指定时自动配置
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "${QT_MINGW}/bin/g++.exe" CACHE FILEPATH "C++ compiler")
endif()
if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER   "${QT_MINGW}/bin/gcc.exe"  CACHE FILEPATH "C compiler")
endif()
if(NOT CMAKE_MAKE_PROGRAM)
    set(CMAKE_MAKE_PROGRAM "${QT_NINJA}"               CACHE FILEPATH "Ninja")
endif()

project(dx11_sync VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# ═══════════════════════════════════════════════════════════
#  Qt 6 — CMAKE_PREFIX_PATH 必须在 find_package 之前
# ═══════════════════════════════════════════════════════════
if(NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "${QT_ROOT}" CACHE PATH "Qt 6 install prefix" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

# 目标 Windows 10+ (Win11 兼容)
add_definitions(-D_WIN32_WINNT=0x0A00)
add_definitions(-DNTDDI_VERSION=0x0A00000B)
add_definitions(-DWIN32_LEAN_AND_MEAN)

# MSVC: 静态 CRT
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ── 源文件 ─────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/window_manager.cpp
    src/sync_engine.cpp
    src/window_list_model.cpp
    src/sync_controller.cpp
)

# MinGW: 需要 .rc 资源文件嵌入 manifest（管理员权限）
if(MINGW)
    set(MANIFEST_PATH "${CMAKE_SOURCE_DIR}/app.manifest")
    configure_file("${CMAKE_SOURCE_DIR}/resource.rc.in" "${CMAKE_BINARY_DIR}/resource.rc" @ONLY)
    list(APPEND SOURCES "${CMAKE_BINARY_DIR}/resource.rc")

    # 使用当前工具链自带的 windres
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(CMAKE_RC_COMPILER "${MINGW_BIN_DIR}/windres.exe")
endif()

# ── Qt 可执行文件 ──────────────────────────────────────
qt_add_executable(dx11_sync WIN32 ${SOURCES})

target_include_directories(dx11_sync PRIVATE src)

target_link_libraries(dx11_sync PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    user32
    gdi32
    comctl32
    dwmapi
)

# ── QML 模块 ───────────────────────────────────────────
qt_add_qml_module(dx11_sync
    URI "dx11sync"
    VERSION 1.0
    QML_FILES src/qml/MainWindow.qml
)

# ── MinGW 专属配置 ─────────────────────────────────────
if(MINGW)
    target_link_options(dx11_sync PRIVATE -municode -static)
endif()

# MSVC: 嵌入 manifest
if(MSVC)
    set_target_properties(dx11_sync PROPERTIES
        VS_GLOBAL_ApplicationManifest "${CMAKE_SOURCE_DIR}/app.manifest"
    )
endif()
