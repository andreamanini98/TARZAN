<p align="center">
  <img src="tlogo.png" alt="TARZAN logo" width="50%"/>
</p>

---

TARZAN (Timed Automata Region and Zone library for real-time systems ANalysis) is a region-based verification library for (networks of) Timed Automata (TA).
Although zones are the de facto standard in the formal verification of TA, TARZAN exhibits superior performance on TA having punctual guards (i.e., guards restricted to equality constraints) and on closed TA.
Consequently, integrating TARZAN into existing zone-based state-of-the-art tools has the potential to significantly enhance their overall verification capabilities.

TARZAN currently supports forward and backward reachability analysis of Timed Automata and Timed Games solving.
More information about the supported Timed Games variants in the wiki.



# Getting Started

## Operating System Compatibility

| O/S              | Status |
|------------------|:------:|
| macOS            | ✅     |
| Linux            | ⚠️     |
| Windows          | ❌    |

**Warning:** TARZAN have been developed and tested on macOS Sequoia 15.6.1, but should work on Linux (this may require adjusting the CMakeLists.txt file).
A complete test on Linux is deferred to future work.
A working CMakeLists.txt file for Linux is under development and will be soon updated.


## Prerequisites

TARZAN needs the following dependencies to be installed:
- <a href="https://www.boost.org" target="_blank" rel="noopener noreferrer">Boost libraries</a> v. 1.89.0 (previous versions should work as well, though they have not been tested; on macOS, suggested installation via Homebrew).
- <a href="https://abseil.io" target="_blank" rel="noopener noreferrer">Abseil libraries</a> v. 20250814.1 (previous versions should work as well, though they have not been tested; on macOS, suggested installation via Homebrew).
- <a href="https://cmake.org" target="_blank" rel="noopener noreferrer">CMake</a> v. 3.31 (minimum required version).
- A C++20 compatible compiler (should also support OpenMP, e.g., Clang).
- Python v. 3.10 (required for benchmark summary generation).
- A proper LaTeX distributon (required for benchmark summary generation).
- <a href="https://graphviz.org" target="_blank" rel="noopener noreferrer">Graphviz</a> (every version should work, required for the visualization of Strategy Graphs).

When performing Timed Games analysis, OpenMP parallelization is enabled by default.
You can change this behavior before installing or building the library by setting to OFF the relative toggle in the `CMakeLists.txt` file.
Currently, the only tested OpenMP-compatible compiler is Clang, which requires the following dependency:
-  <a href="https://formulae.brew.sh/formula/libomp" target="_blank" rel="noopener noreferrer">libmop</a> (every version should work).
  
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

# Publications

[1] A. Manini, M. Rossi, and P. San Pietro, "TARZAN: A Region-Based Library for Forward and Backward Reachability of Timed Automata (Extended Version)", arXiv:2602.15435 [cs.FL], 2026. Available: https://arxiv.org/abs/2602.15435

[2] A. Manini, M. Rossi, and P. San Pietro, "TARZAN: A Region-Based Library for Forward and Backward Reachability of Timed Automata", to appear in Proc. 46th International Conference on Formal Techniques for Distributed Objects, Components, and Systems (FORTE 2026), 2026.

[3] A. Manini, M. Rossi, and P. San Pietro, "TARZAN Artifact". Zenodo, feb. 16, 2026. doi: <a href="https://doi.org/10.5281/zenodo.18656202" target="_blank" rel="noopener noreferrer">10.5281/zenodo.18656203</a>. Artifact evaluated at FORTE 2026; awarded Available and Functional badges.

[4] A. Manini, M. Rossi, and P. San Pietro, "Timed Games under Environmental Interference with Real-Time Objectives", to appear in Proc. 20th International Symposium on Theoretical Aspects of Software Engineering (TASE 2026), 2026.


