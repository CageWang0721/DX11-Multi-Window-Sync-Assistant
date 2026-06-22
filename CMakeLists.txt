cmake_minimum_required(VERSION 3.15)
project(dx11_sync VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 目标 Windows 10+ (Win11 兼容)
add_definitions(-D_WIN32_WINNT=0x0A00)
add_definitions(-DNTDDI_VERSION=0x0A00000B)

# MSVC: 静态 CRT
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ── 源文件 ──
set(SOURCES
    src/main.cpp
    src/window_manager.cpp
    src/sync_engine.cpp
)

# MinGW: 需要 .rc 资源文件嵌入 manifest（管理员权限）
if(MINGW)
    set(MANIFEST_PATH "${CMAKE_SOURCE_DIR}/app.manifest")
    configure_file("${CMAKE_SOURCE_DIR}/resource.rc.in" "${CMAKE_BINARY_DIR}/resource.rc" @ONLY)
    list(APPEND SOURCES "${CMAKE_BINARY_DIR}/resource.rc")
endif()

add_executable(dx11_sync WIN32 ${SOURCES})

target_include_directories(dx11_sync PRIVATE src)

target_link_libraries(dx11_sync PRIVATE user32 gdi32 comctl32)

# ── MinGW 专属配置 ──
if(MINGW)
    # -municode: 识别 wWinMain 入口
    # -static: 静态链接所有 MinGW 运行时 DLL
    target_link_options(dx11_sync PRIVATE -municode -static)
endif()

# MSVC: 嵌入 manifest
if(MSVC)
    set_target_properties(dx11_sync PROPERTIES
        VS_GLOBAL_ApplicationManifest "${CMAKE_SOURCE_DIR}/app.manifest"
    )
endif()
