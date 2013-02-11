Gear2D Component: Lua Proxy (lua-proxy)
=======================================

Using this component you will be able to create and load components made in the
Lua language. Components made in Lua and standard components (made in C++) can
use the parameter table transparently.

Compiling from source code
--------------------------

You will need Gear2D and Lua51 previously installed. CMake is also necessary.
We recommend building it in another folder (usually creating the build/ folder
and calling 'cmake ../' inside).

 1. Use the cmake-gui application. Make the "source directory" field point to
the folder where you unpacked lua-proxy. Make the "build directory" point to the
build/ folder inside it.

 2. You will need the environment variable "LUA_DIR" pointing to where you
installed Lua (development version) (this is necessary only if you installed
it to a non-standard location). Refer to
http://luabinaries.sourceforge.net/download.html to find the correct version
for your platform and compiler, or head to your package manager to install
it for you.
  
 3. Hit the "Generate" button in cmake-gui, chose target compiler and done.

 4. Use your compiler/toolchain/IDE inside the build folder to compile everything.
  
 5. If you intend to make any package/installer for it, make your toolchain do
 the target PACKAGE.