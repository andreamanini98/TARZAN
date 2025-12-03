<p align="center">
  <img src="tlogo.png" alt="TARZAN logo" width="50%"/>
</p>

---

TARZAN (Timed Automata Region and Zone library for real-time systems ANalysis) is a region-based verification library for (networks of) Timed Automata (TA).
Although zones are the de facto standard in the formal verifiation of TA, TARZAN exhibits superior performance on TA having punctual guards (i.e., guards restricted to equality constraints) an on closed TA.
Consequently, integrating TARZAN into existing zone-based state-of-the-art tools has the potential to significantly enhance their overall verification capabilities.



# Getting started

## Operating System Compatibility

| O/S              | Status |
|------------------|:------:|
| macOS            | ✅     |
| Linux            | ⚠️     |

**Warning:** TARZAN have been developed and tested on macOS Sequoia 15.6.1, but should work on Linux distributions (this may require adjusting the CMakeLists.txt file).
A complete test on Linux is deferred to future work.


## Prerequisites

TARZAN needs the following dependencies to be installed:
- <a href="https://www.boost.org" target="_blank" rel="noopener noreferrer">Boost libraries</a> v. 1.89.0 (on macOS, suggested installation via Homebrew).
- <a href="https://abseil.io" target="_blank" rel="noopener noreferrer">Abseil libraries</a> v. 20250814.1 (previous versions should work as well, though they have not been tested; on macOS, suggested installation via Homebrew).
- <a href="https://cmake.org" target="_blank" rel="noopener noreferrer">CMake</a> v. 3.31.
- A C++20 compatible compiler.

## Build and install

TODO


# Using TARZAN

For a detailed description of TARZAN please refer to:
- The TARZAN <a href="https://github.com/andreamanini98/TARZAN/wiki" target="_blank" rel="noopener noreferrer">wiki</a>.


