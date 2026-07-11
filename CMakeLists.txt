cmake_minimum_required(VERSION 3.20)
project(dx11_sync VERSION 3.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# UTF-8
add_compile_options(/utf-8)

# 静态 CRT = 单文件
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Win10+ SDK
add_definitions(-D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A00000B -DWIN32_LEAN_AND_MEAN)

# 清单通过 resource.rc 显式嵌入，禁用链接器自动生成的默认清单。
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

add_executable(dx11_sync WIN32
    src/main.cpp
    src/app_window.cpp
    src/sync_engine.cpp
    src/window_manager.cpp
    src/key_blacklist.cpp
    resource.rc
)
target_include_directories(dx11_sync PRIVATE src)
target_link_libraries(dx11_sync PRIVATE user32 gdi32 advapi32 dwmapi d2d1 dwrite comctl32)
