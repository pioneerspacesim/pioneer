# Find installed GLEW

# find include path
find_path(GLEW_INCLUDE_DIR
   NAMES GL/glew.h
   DOC "GLEW include path"
)

# find static library
find_library(GLEW_LIBRARY
   NAMES GLEW
   DOC "GLEW API static library path"
)

set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
set(GLEW_LIBRARIES ${GLEW_LIBRARY})

if (GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
   set(GLEW_FOUND TRUE)
   message (STATUS "Found GLEW")
else()
   set(GLEW_FOUND FALSE)
endif()
