cmake_minimum_required(VERSION 3.13.0)
cmake_policy(SET CMP0054 NEW)
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
    cmake_policy(SET CMP0074 NEW)
endif()

project(owent-utils-lua-sample)

# =========== 3rdparty atframe_utils ==================

# make 3rd party detect
if (ATFRAME_UTILS_ROOT)
    set (ATFRAMEWORK_ATFRAME_UTILS_REPO_DIR ${ATFRAME_UTILS_ROOT})
else()
    set (ATFRAMEWORK_ATFRAME_UTILS_REPO_DIR "${CMAKE_CURRENT_LIST_DIR}/3rd_party/atframe_utils")
    if(NOT EXISTS ${ATFRAMEWORK_ATFRAME_UTILS_REPO_DIR})
        if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/3rd_party")
            file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/3rd_party")
        endif()
        find_package(Git)
        if(NOT GIT_FOUND)
            message(FATAL_ERROR "git not found")
        endif()

        execute_process(COMMAND ${GIT_EXECUTABLE} clone --depth=100 -b master https://github.com/atframework/atframe_utils.git ${ATFRAMEWORK_ATFRAME_UTILS_REPO_DIR}
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/3rd_party"
        )
    endif()

    set (ATFRAME_UTILS_ROOT ${ATFRAMEWORK_ATFRAME_UTILS_REPO_DIR})
endif()

set (ATFRAMEWORK_ATFRAME_UTILS_INC_DIR "${ATFRAMEWORK_ATFRAME_UTILS_REPO_DIR}/include")
set (ATFRAMEWORK_ATFRAME_UTILS_SRC_DIR "${ATFRAMEWORK_ATFRAME_UTILS_REPO_DIR}/src")
set (ATFRAMEWORK_ATFRAME_UTILS_LINK_NAME atframe_utils)

########################################################################
# CMake 模块 (递归包含模块, 带颜色输出模块, 平台检测模块)
set (PROJECT_CMAKE_MODULE_DIR "${ATFRAME_UTILS_ROOT}/project/cmake")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_CMAKE_MODULE_DIR}/modules")


include("${PROJECT_CMAKE_MODULE_DIR}/modules/IncludeDirectoryRecurse.cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/modules/EchoWithColor.cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/modules/FindConfigurePackage.cmake")

include("${PROJECT_CMAKE_MODULE_DIR}/FindPlatform.cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/ProjectTools.cmake")

#####################################################################
# 导入编译器和编译选项配置
include("${PROJECT_CMAKE_MODULE_DIR}/CompilerOption.cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/TargetOption.cmake")
EchoWithColor(COLOR GREEN "-- Build Type: ${CMAKE_BUILD_TYPE}")


#####################################################################
# =========== libatframe_utils ==================
add_subdirectory(${ATFRAME_UTILS_ROOT})

# =========== lua ==================
find_package(Lua)
if (LUA_FOUND)
    EchoWithColor(COLOR GREEN "-- Dependency: Lua found.(${LUA_VERSION_STRING})")
    EchoWithColor(COLOR GREEN "-- Dependency: Lua include directory: ${LUA_INCLUDE_DIR}")
    EchoWithColor(COLOR GREEN "-- Dependency: Lua libraries directory: ${LUA_LIBRARIES}")
else()
    EchoWithColor(COLOR RED "-- Dependency: Lua is required")
    message(FATAL_ERROR "Lua not found")
endif ()

#####################################################################
# sample target
include_directories(${ATFRAMEWORK_ATFRAME_UTILS_INC_DIR} ${LUA_INCLUDE_DIR} "${CMAKE_CURRENT_LIST_DIR}/../src_native")

# copy sample.lua
if (NOT ${CMAKE_CURRENT_LIST_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/sample.lua" "${CMAKE_BINARY_DIR}/sample.lua"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
    list(APPEND COMPILER_OPTION_EXTERN_CXX_LIBS pthread)
    if (THREADS_PREFER_PTHREAD_FLAG)
        add_definitions(${THREADS_PREFER_PTHREAD_FLAG})
        list(APPEND COMPILER_OPTION_EXTERN_CXX_LIBS pthread)
    endif ()
endif ()

file(GLOB_RECURSE LUA_NATIVE_SRC_LIST
    RELATIVE ${CMAKE_CURRENT_BINARY_DIR}
    "${CMAKE_CURRENT_LIST_DIR}/../src_native/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src_native/*.cc"
    "${CMAKE_CURRENT_LIST_DIR}/../src_native/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src_native/*.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/../src_native/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/../src_native/*.hpp"
)
source_group_by_dir(LUA_NATIVE_SRC_LIST)

add_executable(sample sample.cpp ${LUA_NATIVE_SRC_LIST})

target_link_libraries(sample
    ${LUA_LIBRARIES}
    ${ATFRAMEWORK_ATFRAME_UTILS_LINK_NAME}
    ${COMPILER_OPTION_EXTERN_CXX_LIBS}
)

