# Copyright (C) 2022 Trevor Last
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.10)

project(Compute VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(src
    src/main.cpp
    src/Shader.cpp
    src/Program.cpp
    src/Buffer.cpp
    src/VertexArray.cpp
    src/Texture.cpp
    src/ComputeRaytraceRenderer.cpp
)

add_executable(compute "${src}")

if(MSVC)
    target_compile_options(compute PRIVATE /W3)

    # GLM requirements
    target_include_directories(compute PRIVATE externals/glm)

    # SDL requirements
    target_include_directories(compute PRIVATE externals/SDL2-2.0.22/include)
    target_link_libraries(compute PRIVATE "../externals/SDL2-2.0.22/lib/x64/SDL2main")
    target_link_libraries(compute PRIVATE "../externals/SDL2-2.0.22/lib/x64/SDL2")

    # GLEW requirements
    target_include_directories(compute PRIVATE externals/glew-2.2.0/include)
    target_link_libraries(compute PRIVATE OpenGL32)
    target_link_libraries(compute PRIVATE "../externals/glew-2.2.0/lib/Release/x64/glew32")
else()
    target_compile_options(compute PRIVATE -Wall -Wextra -g)

    # SDL requirements
    target_link_libraries(compute -lSDL2)
    target_include_directories(compute PRIVATE /usr/include/SDL2)
    target_compile_definitions(compute PRIVATE -D_REENTRANT)

    # GLEW requirements
    target_compile_options(compute PRIVATE -L/usr/local/lib64)
    target_include_directories(compute PRIVATE -I/usr/local/include)
    target_link_libraries(compute -lGLEW)
    target_link_libraries(compute -lGL)
    target_link_libraries(compute -lX11)
    target_link_libraries(compute -lGLU)
endif()
