add_executable(data_parser
    data_parser.cpp
    data_log.cpp)

target_link_libraries(data_parser fit)

message("PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}")

target_include_directories(data_parser PRIVATE ${PROJECT_SOURCE_DIR})