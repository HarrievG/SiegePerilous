cmake_minimum_required(VERSION 3.28.3)
project(SiegePerilous)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(libraries)

include(FetchContent)

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


# --- Fetch Zlib ---
FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib
  GIT_TAG master
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(zlib)

message(STATUS "zlib_SOURCE_DIR: ${zlib_SOURCE_DIR}")
message(STATUS "zlib_BINARY_DIR: ${zlib_BINARY_DIR}")


set (headers
"src/SiegePerilous.h"
"src/b2_sdl_draw.h"
"src/World.h"
"src/tiled_data.h"
"src/PhysicsShapeCreator.h"
"src/ShapeFactory.h" 
"src/Sprite.h"
"src/ContentFactory.h"
"src/include/Declarations.h"

)

set (sources
"src/main.cpp"
"src/b2_sdl_draw.cpp"
"src/tiled_data_loader.cpp"
"src/World.cpp"
"src/FileSystem.cpp"
"src/ShapeFactory.cpp"
"src/ChainShapeCreator.cpp"
"src/gfx/cube_atlas.cpp"
"src/ContentFactory.cpp"

)


add_executable(${PROJECT_NAME} ${sources} ${headers} "src/tiled_data.h")
SDL_AddCommonCompilerFlags(${PROJECT_NAME})
 
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)
target_link_libraries(${PROJECT_NAME} 
	PRIVATE SDL3::SDL3-static 
	PRIVATE SDL3_image::SDL3_image 
	PRIVATE glaze::glaze 
	PRIVATE box2d::box2d
	PRIVATE zlib
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${glaze_SOURCE_DIR}/include/glaze
  ${SDL3_BINARY_DIR}/include-config-$<LOWER_CASE:$<CONFIG>>
  ${box2d_SOURCE_DIR}/include
)