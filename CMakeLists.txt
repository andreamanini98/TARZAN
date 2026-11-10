# Setting the minimum required version for CMake.
cmake_minimum_required(VERSION 3.22)

# Getting the project name as the current directory name.
get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# Setting project information.
project(${ProjectId}
        VERSION 0.1.0
        DESCRIPTION "TARZAN: Timed Automata Region and Zone library for real-time systems ANalysis"
        LANGUAGES CXX)

# Export compile commands for tool integration (e.g., Vim, clangd).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use modern C++20.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Creating output directories for library binaries and experiment logs.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/benchmark_tarzan_results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/benchmark_tchecker_results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/benchmark_uppaal_results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/lib)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/backwards_tarzan_results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/games_tarzan_results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/strategy_graphs)

# The static library archive will be generated in this directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/lib)

# Global Build Options
option(ENABLE_OPTIMIZATION "Enable O3 optimization level." ON)
option(ENABLE_OPENMP "Enable OpenMP parallelization." OFF)
option(ENABLE_DEBUG "Enable debug flags." OFF)
option(ENABLE_TBB "Enable Intel oneTBB parallelization." ON)

# ----------------------------------------------------------------------
# APPIMAGE / ARTIFACT MODE
# Generates _seq, _omp, and _tbb binary variants for all executables.
# ----------------------------------------------------------------------
option(ENABLE_APPIMAGE "Build sequential, OpenMP-only, and TBB-only versions." OFF)

# Handle O3 optimization flags.
if (ENABLE_OPTIMIZATION)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
        message(STATUS "O3 Optimization enabled.")
    endif ()
endif ()

# Handle Debug flags.
if (ENABLE_DEBUG)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -g")
        message(STATUS "Debug enabled.")
    endif ()
endif ()

# Force CMake to ignore Anaconda paths to prevent version conflicts.
set(ANACONDA_PATH "/opt/anaconda3" CACHE PATH "Path to Anaconda installation to ignore")
if (ANACONDA_PATH)
    set(CMAKE_IGNORE_PATH "${ANACONDA_PATH}")
endif ()

# Find Boost and Abseil dependencies.
find_package(Boost 1.90.0 REQUIRED filesystem)

include(FetchContent)
FetchContent_Declare(
    abseil-cpp
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG        20240722.0
)
set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL ON)
FetchContent_MakeAvailable(abseil-cpp)

# Public dependencies shared across library and executables to avoid undefined symbol errors.
set(COMMON_TARZAN_PUB_LIBS 
    Boost::headers 
    Boost::filesystem
    absl::base 
    absl::strings 
    absl::flat_hash_map
    absl::flat_hash_set
    absl::hash
    absl::raw_hash_set
)

# Configure OpenMP via standard CMake module.
if (ENABLE_OPENMP OR ENABLE_APPIMAGE)
    find_package(OpenMP REQUIRED)
    if (OpenMP_CXX_FOUND)
        message(STATUS "OpenMP enabled (Found version: ${OpenMP_CXX_VERSION})")
    endif()
endif()

# Configure Intel oneTBB.
if (ENABLE_TBB OR ENABLE_APPIMAGE)
    find_package(TBB REQUIRED)
    if (TBB_FOUND)
        message(STATUS "oneTBB enabled (Found version: ${TBB_VERSION})")
    else()
        message(FATAL_ERROR "oneTBB enabled but not found. Please source the TBB environment variables.")
    endif()
endif()

# Add compiler warnings.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wall COMPILER_SUPPORTS_WALL)
if (COMPILER_SUPPORTS_WALL)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# Core library source files.
set(LIBRARY_SOURCES
        TARZAN/src/library.cpp
        TARZAN/parser/timed_automaton.cpp
        TARZAN/regions/Region.cpp
        TARZAN/parser/ast.cpp
        TARZAN/regions/RTS.cpp
        TARZAN/regions/RTSArena.cpp
        TARZAN/regions/StrategyGraph.cpp
        TARZAN/regions/StrategyGraphForConjunction.cpp
        TARZAN/regions/networkOfTA/RTSNetwork.cpp
        TARZAN/regions/networkOfTA/NetworkRegion.cpp)

# ----------------------------------------------------------------------
# MACRO: Create library variant targets
# ----------------------------------------------------------------------
macro(create_tarzan_lib TARGET_NAME USE_OMP USE_TBB)
    add_library(${TARGET_NAME}_OBJECTS OBJECT ${LIBRARY_SOURCES})
    target_compile_definitions(${TARGET_NAME}_OBJECTS PUBLIC EARLY_EXIT)
    
    target_link_libraries(${TARGET_NAME}_OBJECTS PUBLIC ${COMMON_TARZAN_PUB_LIBS})
    target_include_directories(${TARGET_NAME}_OBJECTS PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include> PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/src)

    if (${USE_OMP})
        target_link_libraries(${TARGET_NAME}_OBJECTS PUBLIC OpenMP::OpenMP_CXX)
    endif()

    if (${USE_TBB})
        target_compile_definitions(${TARGET_NAME}_OBJECTS PUBLIC _TBB)
        target_link_libraries(${TARGET_NAME}_OBJECTS PUBLIC TBB::tbb)
    endif()

    add_library(${TARGET_NAME} STATIC $<TARGET_OBJECTS:${TARGET_NAME}_OBJECTS>)
    target_link_libraries(${TARGET_NAME} PUBLIC ${COMMON_TARZAN_PUB_LIBS})
    
    if (${USE_OMP})
        target_link_libraries(${TARGET_NAME} PUBLIC OpenMP::OpenMP_CXX)
    endif()

    if (${USE_TBB})
        target_link_libraries(${TARGET_NAME} PUBLIC TBB::tbb)
    endif()

    target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include> PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/src)
endmacro()

# Instantiate libraries based on active configuration.
if (ENABLE_APPIMAGE)
    # Generate independent static libraries for each variant.
    create_tarzan_lib(TARZAN_seq FALSE FALSE)
    create_tarzan_lib(TARZAN_omp TRUE  FALSE)
    create_tarzan_lib(TARZAN_tbb FALSE TRUE)
else()
    # Generate single standard library using local flags.
    create_tarzan_lib(TARZAN ${ENABLE_OPENMP} ${ENABLE_TBB})
endif()

# Header files to expose.
set(LIBRARY_HEADERS
        TARZAN/headers/library.h
        TARZAN/parser/ast.h
        TARZAN/parser/enums/arithmetic_enum.h
        TARZAN/parser/enums/boolean_op_enum.h
        TARZAN/parser/enums/cltloc_op_enum.h
        TARZAN/parser/enums/comparison_op_enum.h
        TARZAN/parser/enums/conjunction_type_enum.h
        TARZAN/parser/enums/input_output_action_enum.h
        TARZAN/parser/enums/players_enum.h
        TARZAN/regions/Region.h
        TARZAN/regions/RTS.h
        TARZAN/regions/RTSArena.h
        TARZAN/regions/StrategyGraph.h
        TARZAN/regions/StrategyGraphForConjunction.h
        TARZAN/regions/enums/state_space_exploration_enum.h
        TARZAN/regions/networkOfTA/NetworkRegion.h
        TARZAN/regions/networkOfTA/RTSNetwork.h
        TARZAN/utilities/hash_utilities.h
        TARZAN/utilities/function_utilities.h
        TARZAN/utilities/file_utilities.h
        TARZAN/utilities/partition_utilities.h
        TARZAN/utilities/printing_utilities.h
        TARZAN/exceptions/emptyConjunctionOfFormulae_exception.h
        TARZAN/exceptions/nestedCLTLocFormula_exception.h
        TARZAN/exceptions/cannotSynthesizeStrategies_exception.h)

# Copy header structure to output folder.
foreach (source_file ${LIBRARY_HEADERS})
    get_filename_component(filename ${source_file} NAME)
    configure_file(${source_file} ${CMAKE_CURRENT_SOURCE_DIR}/output/TARZAN/include/${filename} COPYONLY)
endforeach ()

# ----------------------------------------------------------------------
# MACRO: Build single executable target
# ----------------------------------------------------------------------
macro(build_single_executable EXEC_NAME SOURCE OUT_DIR IS_BACKWARDS USE_OMP USE_TBB LIB_TARGET)
    if (${IS_BACKWARDS})
        # Backwards analysis targets build directly from sources without EARLY_EXIT.
        add_executable(${EXEC_NAME} ${SOURCE} ${LIBRARY_SOURCES})
        target_link_libraries(${EXEC_NAME} PRIVATE ${COMMON_TARZAN_PUB_LIBS})
        target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/src)

        if (${USE_OMP})
            target_link_libraries(${EXEC_NAME} PRIVATE OpenMP::OpenMP_CXX)
        endif()

        if (${USE_TBB})
            target_compile_definitions(${EXEC_NAME} PRIVATE _TBB)
            target_link_libraries(${EXEC_NAME} PRIVATE TBB::tbb)
        endif()
    else()
        # Standard executable linking against precompiled library variant.
        add_executable(${EXEC_NAME} ${SOURCE})
        target_link_libraries(${EXEC_NAME} PRIVATE ${LIB_TARGET} ${COMMON_TARZAN_PUB_LIBS})
    endif()

    set_target_properties(${EXEC_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
endmacro()

# ----------------------------------------------------------------------
# MACRO: Process batch executable sources into _seq, _omp, _tbb
# ----------------------------------------------------------------------
macro(build_executables SRC_LIST OUT_DIR IS_BACKWARDS)
    if (${SRC_LIST})
        file(MAKE_DIRECTORY "${OUT_DIR}")

        foreach (source IN LISTS ${SRC_LIST})
            get_filename_component(executable_name ${source} NAME_WE)

            if (ENABLE_APPIMAGE)
                # 1. SEQUENTIAL (No parallelization)
                build_single_executable("${executable_name}_seq" ${source} ${OUT_DIR} ${IS_BACKWARDS} FALSE FALSE TARZAN_seq)
                # 2. OPENMP ONLY
                build_single_executable("${executable_name}_omp" ${source} ${OUT_DIR} ${IS_BACKWARDS} TRUE FALSE TARZAN_omp)
                # 3. TBB ONLY
                build_single_executable("${executable_name}_tbb" ${source} ${OUT_DIR} ${IS_BACKWARDS} FALSE TRUE TARZAN_tbb)
            else()
                # STANDARD SINGLE BUILD MODE
                build_single_executable("${executable_name}" ${source} ${OUT_DIR} ${IS_BACKWARDS} ${ENABLE_OPENMP} ${ENABLE_TBB} TARZAN)
            endif()
        endforeach()
    endif()
endmacro()

# Build main library executables.
set(LIBRARY_EXECUTABLES TARZAN/executableFile.cpp)
build_executables(LIBRARY_EXECUTABLES "${CMAKE_CURRENT_SOURCE_DIR}/executables" FALSE)

# Build benchmark executables.
file(GLOB BENCHMARK_EXECUTABLES ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/benchmarks/benchmark_executables/*.cpp)
build_executables(BENCHMARK_EXECUTABLES "${CMAKE_CURRENT_SOURCE_DIR}/executables/benchmark_executables" FALSE)

# Build backwards analysis executables.
file(GLOB BACKWARDS_EXECUTABLES TARZAN/benchmarks_backwards/backwards_executables/*.cpp)
build_executables(BACKWARDS_EXECUTABLES "${CMAKE_CURRENT_SOURCE_DIR}/executables/backwards_executables" TRUE)

# Build game benchmark executables across all subdirectories.
file(GLOB GAMES_EXECUTABLES_PROD ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/benchmarks_games/games_executables/production_cell/*.cpp)
build_executables(GAMES_EXECUTABLES_PROD "${CMAKE_CURRENT_SOURCE_DIR}/executables/games_executables/production_cell" FALSE)

file(GLOB GAMES_EXECUTABLES_MED ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/benchmarks_games/games_executables/med_app/*.cpp)
build_executables(GAMES_EXECUTABLES_MED "${CMAKE_CURRENT_SOURCE_DIR}/executables/games_executables/med_app" FALSE)

file(GLOB GAMES_EXECUTABLES_SYNTH ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/benchmarks_games/games_executables/synth_reactor/*.cpp)
build_executables(GAMES_EXECUTABLES_SYNTH "${CMAKE_CURRENT_SOURCE_DIR}/executables/games_executables/synth_reactor" FALSE)

file(GLOB GAMES_EXECUTABLES_AUTO ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/benchmarks_games/games_executables/autoclave_sterilization_system/*.cpp)
build_executables(GAMES_EXECUTABLES_AUTO "${CMAKE_CURRENT_SOURCE_DIR}/executables/games_executables/autoclave_sterilization_system" FALSE)

# Grant execution permissions for shell scripts.
execute_process(COMMAND chmod 777 ${CMAKE_CURRENT_SOURCE_DIR}/install_library.sh)

file(GLOB BENCHMARK_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/TARZAN/benchmarks/benchmark_scripts/*.sh)
foreach (script ${BENCHMARK_SCRIPTS})
    execute_process(COMMAND chmod +x ${script})
endforeach ()

file(GLOB BACKWARDS_SCRIPTS TARZAN/benchmarks_backwards/backwards_scripts/*.sh)
foreach (script ${BACKWARDS_SCRIPTS})
    execute_process(COMMAND chmod +x ${script})
endforeach ()

file(GLOB GAMES_SCRIPTS TARZAN/benchmarks_games/games_scripts/*.sh)
foreach (script ${GAMES_SCRIPTS})
    execute_process(COMMAND chmod +x ${script})
endforeach ()

# ----------------------------------------------------------------------
# Library Installation & Export Configuration
# ----------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (NOT ENABLE_APPIMAGE)
    install(TARGETS TARZAN EXPORT TARZANTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT TARZANTargets FILE TARZANTargets.cmake NAMESPACE TARZAN:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TARZAN)
endif()

foreach (header ${LIBRARY_HEADERS})
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}/TARZAN" "${CMAKE_CURRENT_SOURCE_DIR}/${header}")
    get_filename_component(header_dir ${rel_path} DIRECTORY)
    install(FILES ${header} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/TARZAN/${header_dir})
endforeach ()

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/TARZANConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/TARZANConfig.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TARZAN)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/TARZANConfigVersion.cmake VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)

install(FILES 
        ${CMAKE_CURRENT_BINARY_DIR}/TARZANConfig.cmake 
        ${CMAKE_CURRENT_BINARY_DIR}/TARZANConfigVersion.cmake 
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TARZAN)
