add_library(external INTERFACE)

target_include_directories(external INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/source/include)

target_compile_features(external INTERFACE c_std_23)
target_compile_features(external INTERFACE cxx_std_23)
