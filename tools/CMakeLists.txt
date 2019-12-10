
list(APPEND HYG_DATABASE_PARSER_SOURCES
	hyg-database-to-json.cpp
	${CMAKE_SOURCE_DIR}/src/LZ4Format.cpp
)

find_package(Threads REQUIRED)
add_executable(hyg-database-parser ${HYG_DATABASE_PARSER_SOURCES})
target_link_libraries(hyg-database-parser PRIVATE
	Threads::Threads
	lz4)
target_include_directories(hyg-database-parser PRIVATE
	${CMAKE_SOURCE_DIR}/contrib/string-view-lite
	${CMAKE_SOURCE_DIR}/contrib/csv-parser)

set_target_properties(${PROJECT_NAME} modelcompiler savegamedump pioneerLib PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS ON
)
