This library is a C++ wrapper for the gpio acces with the linux ABI.
The functionality is build to v2 of the ABI right now and will be discrpted later when finished.



To add the directory via cmake in your project add the following lines:

add_subdirectory(gpio_lib)

target_link_libraries(project_name PUBLIC gpio_lib)

target_include_directories(project_name PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/gpio_lib"
    )
