project(imgui LANGUAGES CXX)
set(BUILD_SHARED_LIBS OFF)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW=1)
add_library(${PROJECT_NAME}
	imgui.cpp
	imgui_draw.cpp
	imgui_demo.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
	imgui_stdlib.cpp
	backends/imgui_impl_sdl2.cpp
)
target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}
	${SDL2_LIBRARIES}
)
