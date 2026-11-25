# LZ4 is used only by this opt-in benchmark, never the Sandbox runtime.
include(FetchContent)
FetchContent_Declare(forgesim_benchmark_lz4
    GIT_REPOSITORY https://github.com/lz4/lz4.git
    GIT_TAG ebb370ca83af193212df4dcbadcc5d87bc0de2f0 # v1.10.0
)
FetchContent_MakeAvailable(forgesim_benchmark_lz4)
add_library(ForgeSimBenchmarkLz4 STATIC "${forgesim_benchmark_lz4_SOURCE_DIR}/lib/lz4.c")
target_include_directories(ForgeSimBenchmarkLz4 SYSTEM PUBLIC "${forgesim_benchmark_lz4_SOURCE_DIR}/lib")
set_target_properties(ForgeSimBenchmarkLz4 PROPERTIES FOLDER "ThirdParty")
# Keep the license with the development tool, even though it is not packaged.
configure_file("${forgesim_benchmark_lz4_SOURCE_DIR}/lib/LICENSE"
    "${CMAKE_CURRENT_BINARY_DIR}/LZ4-LICENSE.txt" COPYONLY)

add_executable(ForgeSimCacheBenchmark CacheBenchmark.cpp)
target_link_libraries(ForgeSimCacheBenchmark PRIVATE ForgeSim::Assets libdeflate::libdeflate_static ForgeSimBenchmarkLz4)
target_compile_features(ForgeSimCacheBenchmark PRIVATE cxx_std_20)
FORGESIM_SET_PROJECT_WARNINGS(ForgeSimCacheBenchmark)
set_target_properties(ForgeSimCacheBenchmark PROPERTIES FOLDER "ForgeSim/Tools")
add_custom_command(TARGET ForgeSimCacheBenchmark POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/LZ4-LICENSE.txt"
        "$<TARGET_FILE_DIR:ForgeSimCacheBenchmark>/LZ4-LICENSE.txt")
