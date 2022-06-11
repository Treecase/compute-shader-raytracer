# Compute Shader Raytracer
Raytracer running on an OpenGL compute shader.

## Building
Requires [SDL2](https://libsdl.org/), [GLEW](http://glew.sourceforge.net/), and [GLM](https://glm.g-truc.net/0.9.9/index.html).
### Linux
```sh
mkdir build
cd build/
cmake ..
cmake --build .
```
### Windows
Download and extract these libraries into the a folder `externals` in the project root, then run CMake.
