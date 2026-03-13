# Setting the minimum required version for cmake.
cmake_minimum_required(VERSION 3.22)


# Getting the project name as the current directory name.
get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)


# Setting project information.
project(${ProjectId}
        VERSION 0.1.0
        DESCRIPTION "TARZAN: Timed Automata Region and Zone library for real-time systems ANalysis"
        LANGUAGES CXX)


# Use modern C++20.
set(CMAKE_CXX_STANDARD 20)


# Require that CMAKE_CXX_STANDARD is treated as a requirement.
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Creating directories to better retrieve the library and for storing output files.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/benchmark_tarzan_results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/benchmark_tchecker_results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/benchmark_uppaal_results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/lib)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/backwards_tarzan_results)


# The library archive is now written inside this directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/lib)


# When changing the value of the options, remember to:
# (1) click on the cmake icon at the bottom left corner -> (2) three dots -> (3) reset cache and reload project.
option(ENABLE_OPTIMIZATION "Enable O3 optimization level." ON)
option(ENABLE_OPENMP "Enable OpenMP parallelization." OFF)


# Handle O3 optimization.
if (ENABLE_OPTIMIZATION)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
        message(STATUS "O3 Optimization enabled.")
    endif ()
endif ()


# Handle OpenMP.
if (ENABLE_OPENMP)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # Homebrew libomp paths.
        set(LIBOMP_INCLUDE_DIR /opt/homebrew/opt/libomp/include)
        set(LIBOMP_LIB_DIR /opt/homebrew/opt/libomp/lib)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp")
        # Add include path.
        include_directories(${LIBOMP_INCLUDE_DIR})
        link_directories(${LIBOMP_LIB_DIR})
        message(STATUS "OpenMP enabled.")
    endif ()
endif ()


# Find Boost libraries.
# set(BOOST_ROOT "/path/to/boost/dir")
find_package(Boost 1.90.0 REQUIRED filesystem)


include(FetchContent)

FetchContent_Declare(
    abseil-cpp
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG        20240722.0
)

# Ensure Abseil uses the same C++ standard as the project.
set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL ON)

FetchContent_MakeAvailable(abseil-cpp)


# Add compiler warnings.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wall COMPILER_SUPPORTS_WALL)
if (COMPILER_SUPPORTS_WALL)
    add_compile_options(-Wall -Wextra -Wpedantic)
    message(STATUS "Added compiler warnings.")
endif ()


# Define the library source files.
set(LIBRARY_SOURCES
        TARZAN/src/library.cpp
        TARZAN/parser/timed_automaton.cpp
        TARZAN/regions/Region.cpp
        TARZAN/parser/ast.cpp
        TARZAN/regions/RTS.cpp
        TARZAN/regions/RTSArena.cpp
        TARZAN/regions/networkOfTA/RTSNetwork.cpp
        TARZAN/regions/networkOfTA/NetworkRegion.cpp)


# Add library as OBJECT library to allow recompilation with different flags.
add_library(TARZAN_OBJECTS OBJECT ${LIBRARY_SOURCES})

# Add static library that uses the object files.
add_library(TARZAN STATIC $<TARGET_OBJECTS:TARZAN_OBJECTS>)

# Define EARLY_EXIT by default for the object library.
# Individual targets can override this.
target_compile_definitions(TARZAN_OBJECTS PUBLIC EARLY_EXIT)


# Configure library dependencies.
# Link all necessary Abseil libraries to avoid undefined symbols
# Order: high-level containers first, then their dependencies (hash, base internals)
set(TARZAN_LIBRARIES
        Boost::headers
        # Boost::system Removed since it causes build bugs in Boost 1.89.0
        Boost::filesystem
        absl::base
        absl::strings
        absl::flat_hash_map)


# Add conditional libraries.
if (ENABLE_OPENMP)
    list(APPEND TARZAN_LIBRARIES omp)
endif ()


# Link libraries to both object library and static library.
target_link_libraries(TARZAN_OBJECTS PUBLIC 
    Boost::headers 
    Boost::filesystem
)
target_link_libraries(TARZAN_OBJECTS PRIVATE 
    absl::base 
    absl::strings 
    absl::flat_hash_map
)

target_link_libraries(TARZAN PUBLIC 
    Boost::headers 
    Boost::filesystem
)
target_link_libraries(TARZAN PRIVATE 
    absl::base 
    absl::strings 
    absl::flat_hash_map
)


# Setup include directories for both object library and static library.
target_include_directories(TARZAN_OBJECTS
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/src)


target_include_directories(TARZAN
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/src)


# Define the headers to expose.
set(LIBRARY_HEADERS
    TARZAN/headers/library.h
    TARZAN/parser/ast.h
    TARZAN/parser/enums/arithmetic_enum.h
    TARZAN/parser/enums/boolean_op_enum.h
    TARZAN/parser/enums/comparison_op_enum.h
    TARZAN/parser/enums/input_output_action_enum.h
    TARZAN/parser/enums/players_enum.h
    TARZAN/parser/enums/cltloc_op_enum.h
    TARZAN/regions/Region.h
    TARZAN/regions/RTS.h
    TARZAN/regions/RTSArena.h
    TARZAN/regions/enums/state_space_exploration_enum.h
    TARZAN/regions/networkOfTA/NetworkRegion.h
    TARZAN/regions/networkOfTA/RTSNetwork.h
    TARZAN/utilities/hash_utilities.h
    TARZAN/utilities/function_utilities.h
    TARZAN/utilities/file_utilities.h
    TARZAN/utilities/partition_utilities.h
    TARZAN/utilities/printing_utilities.h)


# Copying the headers to expose for better retrieve the library.
foreach (source_file ${LIBRARY_HEADERS})
    get_filename_component(filename ${source_file} NAME)
    configure_file(
            ${source_file}
            ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/include/${filename}
            COPYONLY)
endforeach ()


# Set the output directory containing executables.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/executables)


# Define the library executable files.
set(LIBRARY_EXECUTABLES TARZAN/executableFile.cpp)


# Create executables.
foreach (source IN LISTS LIBRARY_EXECUTABLES)
    get_filename_component(executable_name ${source} NAME_WE)
    add_executable(${executable_name} ${source})

    # Targeting the source with libraries.
    target_link_libraries(${executable_name} PRIVATE TARZAN)
endforeach ()


# Collect all .cpp files from benchmark_executables directory.
file(GLOB BENCHMARK_EXECUTABLES ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/benchmarks/benchmark_executables/*.cpp)

# Create benchmark executables in executables/benchmark_executables subdirectory.
if (BENCHMARK_EXECUTABLES)
    foreach (source IN LISTS BENCHMARK_EXECUTABLES)
        get_filename_component(executable_name ${source} NAME_WE)
        add_executable(${executable_name} ${source})

        # Targeting the source with libraries.
        target_link_libraries(${executable_name} PRIVATE TARZAN)

        # Set output directory to executables/benchmark_executables.
        set_target_properties(${executable_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/executables/benchmark_executables)
    endforeach ()
endif ()


# Collect all .cpp files from backwards_executables directory.
file(GLOB BACKWARDS_EXECUTABLES TARZAN/benchmarks_backwards/backwards_executables/*.cpp)

# Create backwards executables in executables/backwards_executables subdirectory.
# These executables are built WITHOUT EARLY_EXIT defined.
if (BACKWARDS_EXECUTABLES)
    foreach (source IN LISTS BACKWARDS_EXECUTABLES)
        get_filename_component(executable_name ${source} NAME_WE)

        # Add executable with library sources directly to recompile without EARLY_EXIT.
        add_executable(${executable_name} ${source} ${LIBRARY_SOURCES})

        # Link with the same libraries as TARZAN.
        target_link_libraries(${executable_name} PRIVATE ${TARZAN_LIBRARIES})

        # Use same include directories as TARZAN.
        target_include_directories(${executable_name} PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/src)

        # NOTE: EARLY_EXIT is intentionally NOT defined for backwards executables.
        # The LIBRARY_SOURCES are compiled directly into this executable without EARLY_EXIT.

        # Set output directory to executables/backwards_executables.
        set_target_properties(${executable_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/executables/backwards_executables)
    endforeach ()
endif ()


# Collect all .cpp files from games_executables directory.
file(GLOB GAMES_EXECUTABLES ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/benchmarks_games/games_executables/*.cpp)

# Create games executables in executables/games_executables subdirectory.
if (GAMES_EXECUTABLES)
    foreach (source IN LISTS GAMES_EXECUTABLES)
        get_filename_component(executable_name ${source} NAME_WE)
        add_executable(${executable_name} ${source})

        # Targeting the source with libraries.
        target_link_libraries(${executable_name} PRIVATE TARZAN)

        # Set output directory to executables/games_executable.
        set_target_properties(${executable_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/executables/games_executables)
    endforeach ()
endif ()


# Add permissions to the install_library.sh script.
execute_process(COMMAND chmod 777 ${CMAKE_CURRENT_SOURCE_DIR}/install_library.sh)


# Add executable permissions to all shell scripts in TARZAN/benchmarks/benchmark_scripts.
file(GLOB BENCHMARK_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/benchmarks/benchmark_scripts/*.sh)
foreach(script ${BENCHMARK_SCRIPTS})
    execute_process(COMMAND chmod +x ${script})
endforeach()


# Add executable permissions to all shell scripts in TARZAN/benchmarks_backwards/backwards_scripts.
file(GLOB BACKWARDS_SCRIPTS TARZAN/benchmarks_backwards/backwards_scripts/*.sh)
foreach(script ${BACKWARDS_SCRIPTS})
    execute_process(COMMAND chmod +x ${script})
endforeach()


# Add executable permissions to all shell scripts in TARZAN/benchmarks_games/games_scripts.
file(GLOB GAMES_SCRIPTS TARZAN/benchmarks_games/games_scripts/*.sh)
foreach(script ${GAMES_SCRIPTS})
    execute_process(COMMAND chmod +x ${script})
endforeach()

#########################################################################
###   The following code deals with the installation of the library   ###
#########################################################################

# Define GNU standard installation directories.
include(GNUInstallDirs)


# Helper functions for creating config files that can be included by other projects to find and use a package.
include(CMakePackageConfigHelpers)


# Install library.
install(TARGETS TARZAN       # Install target, the one created with add_library().
        EXPORT TARZANTargets # Generates and installs a CMake file to import targets into another project.
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}       # For shared libraries.
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}       # For .a files.
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}       # For executables.
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) # Specifies where to look for include files.


# Install headers with directory structure preserved.
foreach(header ${LIBRARY_HEADERS})
    # Get the relative path from TARZAN/ directory.
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}/TARZAN" "${CMAKE_CURRENT_SOURCE_DIR}/${header}")
    get_filename_component(header_dir ${rel_path} DIRECTORY)

    # Install to the appropriate subdirectory.
    install(FILES ${header}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/TARZAN/${header_dir})
endforeach()


# Export targets to make the library discoverable by other CMake projects.
install(EXPORT TARZANTargets
        FILE TARZANTargets.cmake
        NAMESPACE TARZAN:: # Avoid name clashes.
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TARZAN)


# Configure and install package configuration file.
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/TARZANConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/TARZANConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TARZAN)


# Generate version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/TARZANConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)


# Install config files
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/TARZANConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/TARZANConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TARZAN)
