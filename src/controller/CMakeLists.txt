# Create a library for the controller module
add_library(controller controller.cpp)

# Link libraries required by the controller module
target_link_libraries(controller PRIVATE
    view
    scene
    glfw
    ${OPENGL_LIBRARIES}
)

# Include directories for this library
target_include_directories(controller PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
