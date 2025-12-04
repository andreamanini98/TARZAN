<p align="center">
  <img src="tlogo.png" alt="TARZAN logo" width="50%"/>
</p>

---

TARZAN (Timed Automata Region and Zone library for real-time systems ANalysis) is a region-based verification library for (networks of) Timed Automata (TA).
Although zones are the de facto standard in the formal verifiation of TA, TARZAN exhibits superior performance on TA having punctual guards (i.e., guards restricted to equality constraints) an on closed TA.
Consequently, integrating TARZAN into existing zone-based state-of-the-art tools has the potential to significantly enhance their overall verification capabilities.



# Getting Started

## Operating System Compatibility

| O/S              | Status |
|------------------|:------:|
| macOS            | ✅     |
| Linux            | ⚠️     |
| Windows          | ❌    |

**Warning:** TARZAN have been developed and tested on macOS Sequoia 15.6.1, but should work on Linux (this may require adjusting the CMakeLists.txt file).
A complete test on Linux is deferred to future work.


## Prerequisites

TARZAN needs the following dependencies to be installed:
- <a href="https://www.boost.org" target="_blank" rel="noopener noreferrer">Boost libraries</a> v. 1.89.0 (on macOS, suggested installation via Homebrew).
- <a href="https://abseil.io" target="_blank" rel="noopener noreferrer">Abseil libraries</a> v. 20250814.1 (previous versions should work as well, though they have not been tested; on macOS, suggested installation via Homebrew).
- <a href="https://cmake.org" target="_blank" rel="noopener noreferrer">CMake</a> v. 3.31 (minimum required version).
- A C++20 compatible compiler.
If you want to run the benchmarks against Uppaal and TChecker, you also need to install those tools:
- <a href="https://uppaal.org" target="_blank" rel="noopener noreferrer">Uppaal</a> v. 5.0 (other versions should work as well).
- <a href="https://github.com/ticktac-project/tchecker" target="_blank" rel="noopener noreferrer">TChecker</a> v. 0.8 (other versions should work as well).

## Build and Install

To build TARZAN, simply clone the repository and execute the following commands:
```bash
mkdir build
cd build
cmake ..
make
```
Note that building TARZAN generates executable files necessary to automatically perform pre-built benchmarks comparing the performance of TARZAN against Uppaal and TChecker.

To use TARZAN inside your project, you must install it (installation does not build nor generate any executable file).
This can be done by executing the script `install_library.sh` (note that this may require root permissions).
The script itself contains instructions on how to specify a custom installation path.
If the installation is left to default, TARZAN will be installed in the following directories:

- `/usr/local/include/TARZAN`: this contains the headers.
-  `/usr/local/lib/cmake/TARZAN`: this contains cmake files.
- `/usr/local/lib/libTARZAN.a`: this is the library archive.

Note that the above paths refer to macOS running on an Apple Silicon chip (Intel-based macOS paths may differ).


# Using TARZAN

Assuming TARZAN has been installed as described above, using it in a CMake project requires the following steps:

1. Add these lines to the CMakeLists.txt file inside your new project (f TARZAN has been installed in a custom directory, it may be necessary to also specify the corresponding path in your CMakeLists.txt file.):
   ```cmake
   find_package(TARZAN REQUIRED)
   target_link_libraries(your_project_name PRIVATE TARZAN::TARZAN)
   ```

2. Include TARZAN in your C++ or header files as follows:
   ```c++
   #include "TARZAN/path/to/header_to_include.h"
   ```

# References

For a detailed description of TARZAN, please refer to:
- The TARZAN <a href="https://github.com/andreamanini98/TARZAN/wiki" target="_blank" rel="noopener noreferrer">wiki</a>.
- The TARZAN <a href="https://andreamanini98.github.io/TARZAN/" target="_blank" rel="noopener noreferrer">online documentation</a>.


