find_path(OSMIUM_INCLUDE_DIR "osmium/version.hpp")
find_path(PROTOZERO_INCLUDE_DIR "protozero/pbf_reader.hpp")
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

add_executable(ingest main.cc)
target_include_directories(ingest PRIVATE
  ${OSMIUM_INCLUDE_DIR}
  ${PROTOZERO_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
)
target_link_libraries(ingest PRIVATE ZLIB::ZLIB Threads::Threads)
