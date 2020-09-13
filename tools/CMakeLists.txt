
list(APPEND HYG_DATABASE_PARSER_SOURCES
	hyg-database-to-json.cpp
	${PIONEER_SRC}/core/LZ4Format.cpp
	${PIONEER_CONTRIB}/profiler/Profiler.cpp
)

find_package(Threads REQUIRED)
add_executable(hyg-database-parser ${HYG_DATABASE_PARSER_SOURCES})
target_link_libraries(hyg-database-parser PRIVATE
	Threads::Threads
	lz4)

set_cxx11_properties(hyg-database-parser)
