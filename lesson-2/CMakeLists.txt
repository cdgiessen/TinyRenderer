add_executable(line_sweep line_sweep.cpp)
target_link_libraries(line_sweep PUBLIC tga model)

add_executable(lesson-2 main.cpp)
target_link_libraries(lesson-2 PUBLIC tga model)