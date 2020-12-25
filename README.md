# Dear NES Emulator

This project is a Nintendo Entertainment System (NES) emulator written in modern C++ and using ImGui as graphic layer. It targets to be a emulator focused for game developers, programmers and assembly/OS enthusiasts.

There are no release so far because the emulator is still missing features and stability/accuracy must be improved. [Documentation on the project and how-to-build is in progress].

This repository includes the code for the GUI layer of the emulator. The NES logic is compiler as a static library that the project uses. This library can be found in [this repository](https://github.com/eariassoto/dear_nes_lib). 

## Screenshot

![Dear NES Emulator](screenshot.png)

## Dependencies
+ [imgui](https://github.com/ocornut/imgui)
+ OpenGL
+ [GLFW](https://github.com/glfw/glfw)
+ [GLAD](https://glad.dav1d.de/)
+ [fmt](https://github.com/fmtlib/fmt)
