include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(SDL_TEST_LIBRARY FALSE)
set(BUILD_SHARED_LIBS FALSE)
set(SDL_TESTS OFF)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)

FetchContent_Declare(SDL
   GIT_REPOSITORY https://github.com/libsdl-org/SDL
   GIT_TAG main
   #GIT_TAG release-3.2.8
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_JPEG OFF)
set(SDLIMAGE_WEBP OFF)

FetchContent_Declare(SDL_image
   GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
   #GIT_TAG main
   GIT_TAG "release-3.2.4"
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

FetchContent_Makeavailable(
   SDL
   SDL_image)

message(STATUS "SDL_SOURCE_DIR: ${SDL_SOURCE_DIR}")
message(STATUS "SDL_BINARY_DIR: ${SDL_BINARY_DIR}")
message(STATUS "SDL_Image_SOURCE_DIR: ${SDL_Image_SOURCE_DIR}")
message(STATUS "SDL_Image_BINARY_DIR: ${SDL_Image_BINARY_DIR}")