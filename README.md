- [WeirdSpace](#weirdspace)
- [Credits](#credits)
  - [External libraries used in this project](#external-libraries-used-in-this-project)
  - [Sources of knowledge and code samples used in this project](#sources-of-knowledge-and-code-samples-used-in-this-project)
- [How to build it?](#how-to-build-it)
  - [Debian based Linux distros](#debian-based-linux-distros)
  - [Windows](#windows)
    - [Enable symbolic links support](#enable-symbolic-links-support)
    - [Required software](#required-software)
    - [Compilation itself](#compilation-itself)
  - [Other OSes/IDEs/platforms/etc.](#other-osesidesplatformsetc)


<br/>
<br/>

# WeirdSpace


<br/>
<br/>

# Credits
## External libraries used in this project
* OpenGL extension loading:         https://github.com/nigels-com/glew
* Windows, input:                   https://www.glfw.org/
* Maths:                            https://github.com/g-truc/glm
* GUI:                              https://github.com/ocornut/imgui
* Images loading:                   https://github.com/nothings/stb
* CPU noise:                        https://github.com/Reputeless/PerlinNoise
* Delaunay triangulation:           https://github.com/delfrrr/delaunator-cpp

## Sources of knowledge and code samples used in this project
* OpenGL knowledge
    * OpenGL Superbible: Comprehensive Tutorial and Reference (7th Edition)
    * http://www.opengl-tutorial.org/
    * https://learnopengl.com/
        - https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
* Procedural generation
    * Polygon map generation:       http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/
    * GPU noise:                    https://github.com/ashima/webgl-noise
    * Fractal Brownian Motion:      https://thebookofshaders.com/13/
    * Fog effect:                   https://iquilezles.org/articles/fog/


<br/>
<br/>

# How to build it?
First clone the repository:
```bash
git clone --recurse-submodules https://github.com/SomePersonFromMars/WeirdSpace.git
cd WeirdSpace
```
Then follow platform specific instructions.

## Debian based Linux distros
```bash
# Required dependencies:
sudo apt-get install cmake make g++ libx11-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxrandr-dev libxext-dev libxcursor-dev libxinerama-dev libxi-dev

# Generate Makefiles:
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```
To build and run the game itself:
```bash
cd game
make # Append -j for faster multicore compilation
./weird_space
```
To build and run the generator playground:
```bash
cd tools/generator_playground
make # Append -j for faster multicore compilation
./generator_playground
```

## Windows
**TODO: VERIFY THOSE INSTRUCTIONS ONE MORE TIME**
### Enable symbolic links support
As the repository uses symbolic links for compilation simplicity, one [needs to enable those](https://stackoverflow.com/a/59761201):
1. Enable "Developer Mode" in Windows 10/11 - that gives `mklink` permissions.
2. Inside of the repository execute:
   ```batch
   git config core.symlinks true
   git reset --hard
   ```

### Required software
Install Visual Studio 2022 with the "Desktop development with C++" packages.

### Compilation itself
Open "x64 Native Tools Command Prompt for VS 2022" app and then:
1. cd into the cloned repository.
2. `mkdir build`
3. `cd build`
4. `cmake -G "Visual Studio 17 2022" ..`
5. `start WeirdSpace.sln` to open the Visual Studio Solution file.
7. In the Solutin Explorer right click on `weird_space` or `generator_playground` target and click "Set as Startup Project".
8. You can now build the executable or build it and run.

## Other OSes/IDEs/platforms/etc.
Instructions from [this tutorial](https://www.opengl-tutorial.org/beginners-tutorials/tutorial-1-opening-a-window/) would *probably* work.