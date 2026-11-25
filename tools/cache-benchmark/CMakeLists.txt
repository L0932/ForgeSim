add_executable(ForgeSimCacheBenchmark CacheBenchmark.cpp)
target_link_libraries(ForgeSimCacheBenchmark PRIVATE ForgeSim::Assets libdeflate::libdeflate_static ForgeSimLz4)
target_compile_features(ForgeSimCacheBenchmark PRIVATE cxx_std_20)
FORGESIM_SET_PROJECT_WARNINGS(ForgeSimCacheBenchmark)
set_target_properties(ForgeSimCacheBenchmark PROPERTIES FOLDER "ForgeSim/Tools")
add_custom_command(TARGET ForgeSimCacheBenchmark POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${forgesim_lz4_SOURCE_DIR}/lib/LICENSE"
        "$<TARGET_FILE_DIR:ForgeSimCacheBenchmark>/LZ4-LICENSE.txt")
