cmake_minimum_required(VERSION 3.15)

#disable warning that < 3.5 will be deprecated in sub_project
set(CMAKE_POLICY_WARNING_CMP0000 OFF)

# Project Name and specify C++ as the language
project(slugs LANGUAGES CXX)

# Specify the C++ standard (C++23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DGRAPHICS_API_OPENGL_43)

# Add the source file
add_executable(slugs game.cpp)

# Enable all warnings for different compilers
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")  # GCC
    target_compile_options(slugs PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wold-style-cast -Wdouble-promotion -Wnon-virtual-dtor -Woverloaded-virtual -Wcast-align=strict -Wformat=2 -Wnull-dereference -Wunused -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-4")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-4")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -O0")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")  # Clang
    target_compile_options(slugs PRIVATE -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Weverything -Wno-c++98-compat -Wshadow -Wdouble-promotion -Wnon-virtual-dtor -Woverloaded-virtual -Wcast-align=strict -Wformat=2 -Wnull-dereference -Wunused -Wimplicit-fallthrough)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -O0")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")  # MSVC
    target_compile_options(slugs PRIVATE /W4 /permissive- /sdl /w45038 /w45126 /w45400 /w44996 /wd6285 /wd4458)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi /Od")
endif()


include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

#
#FetchContent_Declare(
#  fmt
#  GIT_REPOSITORY https://github.com/fmtlib/fmt
#  GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281 # 10.2.1
#  GIT_PROGRESS TRUE
#)
#FetchContent_MakeAvailable(fmt)

set(RAYLIB_VERSION "5.5") #important
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.5"
    GIT_PROGRESS TRUE
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET YES)
    FetchContent_MakeAvailable(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
  endif()
endif()

set(BUILD_RAYLIB_CPP_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib_cpp
    GIT_REPOSITORY "https://github.com/RobLoach/raylib-cpp.git"
    GIT_TAG "v5.5.0"
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib_cpp)

if(MSVC)
    # Define `_DEBUG` only for MSVC in Debug mode
    target_compile_definitions(slugs PRIVATE $<$<CONFIG:Debug>:_DEBUG>)
    #target_compile_options(fmt PRIVATE /wd4458)
    target_compile_options(raylib PRIVATE /wd4458)
    target_compile_options(glfw PRIVATE /wd4458)
else()
    target_compile_definitions(raylib_cpp INTERFACE Wno-shadow)
    # Define `DEBUG` and `_GLIBCXX_DEBUG` for GCC/Clang in Debug mode
    target_compile_definitions(slugs PRIVATE $<$<CONFIG:Debug>:DEBUG _GLIBCXX_DEBUG>)
endif()

#target_compile_options(seminar_subproject PRIVATE /wd4458)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp") # Define PROJECT_SOURCES as a list of all source files
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src/") # Define PROJECT_INCLUDE to be the path to the include directory of the project

# Declaring our executable
#add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
#target_link_libraries(slugs PRIVATE opengl32 raylib raylib_cpp)
target_link_libraries(slugs PUBLIC shcore raylib_cpp raylib)

#target_link_libraries(slugs PRIVATE  fmt::fmt raylib raylib_cpp)

# Setting ASSETS_PATH
#target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

#target_link_libraries(slugs fmt::fmt Ws2_32)
