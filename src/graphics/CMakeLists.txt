set(TARGETNAME "graphics")

set( ${TARGETNAME}_H
	"Drawables.h"
	"Frustum.h"
	"Graphics.h"
	"Material.h"
	"Renderer.h"
	"RendererGL2.h"
	"RendererGLBuffers.h"
	"RendererLegacy.h"
	"Shader.h"
	"StaticMesh.h"
	"Surface.h"
	"Texture.h"
	"TextureBuilder.h"
	"TextureGL.h"
	"VertexArray.h"
)
set( ${TARGETNAME}_SRC
	"Drawables.cpp"
	"Frustum.cpp"
	"Graphics.cpp"
	"Material.cpp"
	"Renderer.cpp"
	"RendererGL2.cpp"
	"RendererLegacy.cpp"
	"Shader.cpp"
	"StaticMesh.cpp"
	"TextureBuilder.cpp"
	"TextureGL.cpp"
	"VertexArray.cpp"
)


# Setup dependencies
SETUP_DEFAULT_DEPENDENCIES_LIST(DEPENDENCIES_NAMES)

LINK_DIRS_WITH_VARIABLES(${TARGETNAME} ${DEPENDENCIES_NAMES})
add_library("${TARGETNAME}" STATIC ${${TARGETNAME}_H} ${${TARGETNAME}_SRC})
LINK_WITH_VARIABLES(${TARGETNAME} ${DEPENDENCIES_NAMES})

MAKE_ENUM_DEPEND(${TARGETNAME})
#target_link_libraries(${TARGETNAME} LUA OOLUA)

# ADD_DEFINITIONS( 
	# -DBOOST_ENABLE_ASSERT_HANDLER
	# -DBOOST_ALL_DYN_LINK	# Tell boost to use dynamic libraries
	# -DBOOST_ALL_NO_LIB		# No boost auto link
# )

IF (${PROJECT_NAME}_USE_PRECOMPILED_HEADERS)
	SET_USE_PRECOMPILED_HEADER(${TARGETNAME} ${PCH_DEFAULT_H} ${PCH_DEFAULT_CPP})
ENDIF()
