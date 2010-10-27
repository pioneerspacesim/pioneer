set(SOURCES_COLLIDER
	GeomTree.cpp Geom.cpp CollisionSpace.cpp BVHTree.cpp
)
add_library(collider STATIC ${SOURCES_COLLIDER})
set_target_properties(collider PROPERTIES OUTPUT_NAME_DEBUG collider-dbg)
