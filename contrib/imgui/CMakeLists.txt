project(imgui LANGUAGES CXX)

set(BUILD_SHARED_LIBS OFF)

add_library(${PROJECT_NAME}
	imgui.cpp
	imgui_draw.cpp
	examples/sdl_opengl3_example/imgui_impl_sdl_gl3.cpp
	examples/sdl_opengl2_example/imgui_impl_sdl.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}
	${SDL2_LIBRARIES}
)
