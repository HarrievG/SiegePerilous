# Siege Perilous

This is a C++ project that uses SDL3 and Box2D. It currently creates a window and allows for audio recording and playback.

## Dependencies

The project has the following dependencies:

*   [SDL3](https://github.com/libsdl-org/SDL)
*   [Box2D](https://github.com/erincatto/box2d)
*   [Glaze](https://github.com/stephenberry/glaze)

These dependencies are automatically fetched using CMake's `FetchContent` feature, so you do not need to install them manually.

## Building

To build the project, you will need to have CMake installed.

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

After building the project, you can run the executable:

```bash
./SiegePerilous
```

Hold down the left mouse button to record audio. Release the mouse button to play the recorded audio back.
