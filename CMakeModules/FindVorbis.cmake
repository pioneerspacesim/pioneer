# Find installed libvorbis

# find include path
find_path(VORBIS_INCLUDE_DIR
   NAMES vorbis/vorbisfile.h
   DOC "vorbis include path"
)

# find static library
find_library(VORBIS_LIBRARY
   NAMES vorbis
   DOC "vorbis API static library path"
)

set(VORBIS_INCLUDE_DIRS ${VORBIS_INCLUDE_DIR})
set(VORBIS_LIBRARIES ${VORBIS_LIBRARY})

if (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY)
   set(VORBIS_FOUND TRUE)
   message (STATUS "Found libvorbis")
else()
   set(VORBIS_FOUND FALSE)
endif()
