add_executable(lesson-1 main.cpp)
target_link_libraries(lesson-1 PUBLIC tga)

add_executable(wireframe-lesson-1 wireframe.cpp)
target_link_libraries(wireframe-lesson-1 PUBLIC tga model)