
# Windows
* https://stackoverflow.com/a/59761201
* ~~https://sourceforge.net/projects/mingw/~~
    * mingw-developer-toolkit
    * mingw32-base
    * mingw32-gcc-g++
    * msys-base
* ~~https://www.msys2.org/~~
	* Inside MSYS2 URCT64
		* Install via `pacman -S ...`
			* mingw-w64-ucrt-x86_64-gcc
			* mingw-w64-ucrt-x86_64-cmake
		* Use cmake with Ninja generator
* Visual Studio 2022
	* Install desktop development with C++
	* Open "x64 Native Tools Command Prompt for VS 2022" app
	* cd to the cloned folder
	* `cmake -G "Visual Studio 17 2022"
	* `start WeirdSpace.sln` to open Visual Studio Solution file
		* Click Build -> Build Solution (F7)
		* In the Solutin Explorer right click on `weird_space` or `generator_playground` target and click "Set as Startup Project".
		* You can now build the executable or build it and run.
