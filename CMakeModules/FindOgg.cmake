# Find installed libogg

# find include path
find_path(OGG_INCLUDE_DIR
   NAMES ogg/ogg.h
   DOC "ogg include path"
)

# find static library
find_library(OGG_LIBRARY
   NAMES ogg
   DOC "ogg API static library path"
)

set(OGG_INCLUDE_DIRS ${OGG_INCLUDE_DIR})
set(OGG_LIBRARIES ${OGG_LIBRARY})

if (OGG_INCLUDE_DIR AND OGG_LIBRARY)
   set(OGG_FOUND TRUE)
   message (STATUS "Found libogg")
else()
   set(OGG_FOUND FALSE)
endif()
