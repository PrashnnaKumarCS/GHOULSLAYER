# Building GHOULSLAYER

## Prerequisites
- **C++ Compiler**: MinGW-w64 (GCC supporting C++17) or MSVC
- **Build System**: CMake 3.21 or newer
- **Make Tool**: `mingw32-make` (or `ninja` / Visual Studio)
- **Git**: Required by CMake to fetch SFML 2.6.1 source during first configuration

No manual installation of SFML is needed; CMake's `FetchContent` downloads and builds SFML 2.6.1 automatically.

---

## Build Instructions (MinGW-w64 on Windows)

Open PowerShell in the project root directory (`GHOULSLAYER`):

### 1. Configure the Project
Create the build directory and generate build files using MinGW Makefiles:
```powershell
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
```

### 2. Compile the Game
Build the executable:
```powershell
cmake --build build --config Release
```

### 3. Run the Game
Run the compiled binary:
```powershell
.\build\GHOULSLAYER.exe
```

---

## Cleaning the Build
To perform a completely fresh build, simply delete the `build` folder:
```powershell
Remove-Item -Recurse -Force build
```
Then re-run the configure and compile steps above.
