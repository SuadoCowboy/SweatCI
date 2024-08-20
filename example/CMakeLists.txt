add_executable(Example ${PROJECT_SOURCE_DIR}/example/src/CVariableUtils.cpp ${PROJECT_SOURCE_DIR}/example/src/example.cpp)

target_include_directories(Example PRIVATE ${PROJECT_SOURCE_DIR}) # HayBCMD.h
target_link_libraries(Example HayBCMD)