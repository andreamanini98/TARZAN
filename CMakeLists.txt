####################################################
###   How to use the library in other projects   ###
####################################################

# (1) Add these lines to the CMakeLists.txt file in the new project if the library has been installed in the default path:
# find_package(TARZAN REQUIRED)
# target_link_libraries(new_project_name PRIVATE TARZAN::TARZAN)

# (2) Include the library in your files as follows:
# #include "TARZAN/file_where_to_include_the_library"


# Setting the minimum required version for cmake.
cmake_minimum_required(VERSION 3.31)


# These were put for problems with threads, but everything seems fine now.
# set(CMAKE_THREAD_LIBS_INIT "-lpthread")
# set(CMAKE_HAVE_THREADS_LIBRARY 1)
# set(CMAKE_USE_WIN32_THREADS_INIT 0)
# set(CMAKE_USE_PTHREADS_INIT 1)
# set(THREADS_PREFER_PTHREAD_FLAG ON)


# Getting the project name as the current directory name.
get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)


# Setting project information.
project(${ProjectId}
        VERSION 0.1.0
        DESCRIPTION "TARZAN: Timed Automata Region and Zone library for Automata aNalysis"
        LANGUAGES CXX)


# Use modern C++20.
set(CMAKE_CXX_STANDARD 20)


# Require that CMAKE_CXX_STANDARD is treated as a requirement.
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Creating directories to better retrieve the library.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/lib)


# The library archive is now written inside this directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/lib)


# Optimizations can be enabled at compile time.
# When changing the value of the option, remember to:
# click on (1) the cmake icon at the bottom left corner -> (2) three dots -> (3) reset cache and reload project.
option(ENABLE_OPTIMIZATION "Enable O2 optimization level" ON)
if (ENABLE_OPTIMIZATION)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")

        # TODO: review this code (the entire file in general). See also omp in target_link_libraries functions below.
        # Homebrew libomp paths
        set(LIBOMP_INCLUDE_DIR /opt/homebrew/opt/libomp/include)
        set(LIBOMP_LIB_DIR /opt/homebrew/opt/libomp/lib)

        # Add include path
        include_directories(${LIBOMP_INCLUDE_DIR})

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp -O2")

        link_directories(${LIBOMP_LIB_DIR})

        message(STATUS "O2 Optimization enabled.")
    endif ()
endif ()


# Find Boost libraries.
# TODO: delete the line below (keep only find_package(Boost 1.89.0 REQUIRED system filesystem) when they fix the 1.89 build bug)
set(BOOST_ROOT "/Users/echo/Desktop/boostinstall")
find_package(Boost 1.88.0 REQUIRED system filesystem)


# Find Abseil libraries.
find_package(absl CONFIG REQUIRED)


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
        TARZAN/regions/RTS.cpp)


# Add library
add_library(TARZAN STATIC ${LIBRARY_SOURCES})


# Linking libraries to library sources.
target_link_libraries(TARZAN PRIVATE
        Boost::headers
        Boost::system
        Boost::filesystem
        absl::base
        absl::strings
        absl::flat_hash_map
        omp)


# Setup include directories for the library.
target_include_directories(TARZAN
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/src)


# Define the headers to expose.
set(LIBRARY_HEADERS TARZAN/headers/library.h)


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
    target_link_libraries(${executable_name} PRIVATE
            TARZAN
            Boost::headers
            Boost::system
            Boost::filesystem
            absl::base
            absl::strings
            absl::flat_hash_map
            omp)

    # Set include directories properly
    # target_include_directories(${executable_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endforeach ()


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


# Install headers.
install(FILES ${LIBRARY_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/TARZAN)


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


# Add permissions to the install_library.sh script.
execute_process(COMMAND chmod 777 ${CMAKE_CURRENT_SOURCE_DIR}/install_library.sh)
