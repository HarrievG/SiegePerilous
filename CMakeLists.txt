cmake_minimum_required(VERSION 3.28.3)
project(SiegePerilous)

include(FetchContent)

# --- Fetch SDL 3 ---
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
set(SDL_TESTS OFF)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
FetchContent_MakeAvailable(SDL3)

message(STATUS "SDL3_SOURCE_DIR: ${SDL3_SOURCE_DIR}")
message(STATUS "SDL3_BINARY_DIR: ${SDL3_BINARY_DIR}")

# --- Fetch Glaze ---
FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
# Common options to disable tests and examples (may or may not be respected by Glaze)
set(BUILD_TESTING OFF)
set(BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(glaze)

message(STATUS "glaze_SOURCE_DIR: ${glaze_SOURCE_DIR}")
message(STATUS "glaze_BINARY_DIR: ${glaze_BINARY_DIR}")


# --- Fetch Glaze ---
FetchContent_Declare(
  box2d
  GIT_REPOSITORY https://github.com/erincatto/box2d.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(box2d)

message(STATUS "box2d_SOURCE_DIR: ${box2d_SOURCE_DIR}")
message(STATUS "box2d_BINARY_DIR: ${box2d_BINARY_DIR}")


set (headers
src/SiegePerilous.h
src/b2_sdl_draw.h
)

set (sources
src/main.cpp
src/b2_sdl_draw.cpp
)

add_executable(${PROJECT_NAME} ${sources} ${headers})
SDL_AddCommonCompilerFlags(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)
target_link_libraries(${PROJECT_NAME} PRIVATE PRIVATE SDL3::SDL3-static glaze::glaze box2d::box2d)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${glaze_SOURCE_DIR}/include/glaze
  ${SDL3_BINARY_DIR}/include-config-$<LOWER_CASE:$<CONFIG>>
  ${box2d_SOURCE_DIR}/include
)