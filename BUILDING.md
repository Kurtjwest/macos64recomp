# Building Guide

This guide will help you build the project on your local machine. The process will require you to provide a ROM of the US version of the game.

These steps cover: running the recompiler and building the project

## 1. Clone the Smash64r Repository
This project makes use of submodules so you will need to clone the repository with the `--recurse-submodules` flag. This will take awhile.

```bash
git clone --recurse-submodules https://github.com/zestydevy/smash64r.git
# if you forgot to clone with --recurse-submodules
# cd /path/to/cloned/repo && git submodule update --init --recursive
```

## 2. Install Dependencies

### Linux
For Linux the instructions for Ubuntu are provided, but you can find the equivalent packages for your preferred distro.

```bash
# For Ubuntu, simply run:
sudo apt-get install cmake ninja-build libsdl2-dev libgtk-3-dev lld llvm clang
```

### Windows
You will need to install [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/).
In the setup process you'll need to select the following options and tools for installation:
- Desktop development with C++
- C++ Clang Compiler for Windows
- C++ CMake tools for Windows

The other tool necessary will be `make` which can be installe via [Chocolatey](https://chocolatey.org/):
```bash
choco install make
```

It's also recommend you install this specific version of llvm. Newer versions have dropped some flags that we need
```
choco install llvm --version 18.1.8
```

### MacOS
You will need to install llvm 18.1.8 and some other dependencies:

```
brew install llvm@18 lld
```

Make sure llvm 18.1.8 is used:

```
export PATH="/opt/homebrew/opt/llvm@18/bin:$PATH" && export CC=clang && export CXX=clang++ && export LD=ld.lld
```


## 3. Generating the C code

Now that you have the required files, you must build [N64Recomp](https://github.com/Mr-Wiseguy/N64Recomp) and run it to generate the C code to be compiled. The building instructions can be found [here](https://github.com/Mr-Wiseguy/N64Recomp?tab=readme-ov-file#building). That will build the executables: `N64Recomp` and `RSPRecomp` which you should copy to the root of the Smash64r repository.

After that, go back to the repository root, and run the following commands:
```bash
./N64Recomp smashbrothers.us.toml
./RSPRecomp n_aspMain.toml
```
This will cause N64Recomp to generate recompiled code in `RecompiledFuncs/`, and RSPRecomp will generate code in `rsp/n_aspMain.c`

## 4. Building the Project

Finally, you can build the project! :rocket:

On Windows, you can open the repository folder with Visual Studio, and you'll be able to `[build / run / debug]` the project from there.
Resaving the `CMakeLists.txt` file in Visual Studio will update and reconfigure the build system.

If you prefer the command line or you're on a Unix platform you can build the project using CMake (though using VS is highly recommended):

```bash
cmake -S . -B build-cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -G Ninja -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release # or Debug if you want to debug
cmake --build build-cmake --target Smash64r -j$(nproc) --config Release # or Debug
```

## 5. Success

Voilà! You should now have a `Smash64r` executable in the build directory! If you used Visual Studio this will be `out/build/x64-[Configuration]` and if you used the provided CMake commands then this will be `build-cmake`. You will need to run the executable out of the root folder of this project or copy the assets folder to the build folder to run it.

## 6. Patches
In the `patches/` directory, you can patch over existing functions in the game. You must patch the entire function for this to work, so you need the entire asm of the function from the decomp, or you need a C implementation. To patch a C function, place it in a C file in the `patches/` directory. Then place the attribute `RECOMP_PATCH` before the function declaration. When building the project with CMake in VS, these changes will automatically be picked up and used in place of the original recompiled functions. Ex of patching `syDmaReadRom` (but it just does the same thing it originally did, as a patch test)

```
RECOMP_PATCH void syDmaReadRom(uintptr_t rom_src, void *ram_dst, size_t size) {
    syDmaCopy(gSYDmaRomPiHandle, rom_src, (uintptr_t)ram_dst, size, OS_READ);
}
```
