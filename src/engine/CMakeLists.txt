FILE(GLOB engine_source_file LIST_DIRECTORIES True "*.cpp")

add_library(engine SHARED ${engine_source_file})

target_include_directories(engine PUBLIC ${CMAKE_SOURCE_DIR}/src/include)
