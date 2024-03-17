include(GoogleTest)

aux_source_directory(./ TEST_SOURCE)

message("=============TEST=============")

foreach (filename IN LISTS TEST_SOURCE)
    get_filename_component(target ${filename} NAME_WLE)

    message(${target})

    add_executable(${target} ${filename})

    target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/src/include)

    target_link_libraries(${target} gtest_main)

    gtest_discover_tests(${target})

endforeach ()




