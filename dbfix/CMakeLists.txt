
find_program (RESOURCE_COMPILER glib-compile-resources)
if (${RESOURCE_COMPILER} STREQUAL RESOURCE_COMPILER-NOTFOUND)
  message (FATAL_ERROR "GLib resource compiler not found.")
endif ()

file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/resources")

add_custom_command (OUTPUT "${CMAKE_BINARY_DIR}/resources/hyscan-fix-resources.c"
                    COMMAND ${RESOURCE_COMPILER} --generate-source
                            --target "${CMAKE_BINARY_DIR}/resources/hyscan-fix-resources.c"
                            hyscan-fix-resources.xml
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../schemas"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/hyscan-fix-resources.xml"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/3e65462db44e1dc9317f38a063d60ef1"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/6190124dbc946f010e07a5ffb86f68ee"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/2c71f69b31e8d610da88edcb8a165fe0"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/e38fabcf1b95fced8c87447c1ffa0f32"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/3c282d259e3db686fef4c3b26dde5edc"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/7f9eb90c7f852d6457d7a1bdc5d9b012"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/fd8e8922e61b89e749e7b076444c039e"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/de7491c1298d4368ad87ded88e617fea"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/ad1f40a3292926b566fe8ed6465a6e64"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/b288ba043bea8a11886a458a4bfab4a8"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/2f9c8a4405c52774c22fbd1e00c45ab2"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/19a285f398b930c936df0567abf356b3"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/9726336af32731e31dd3752276981192"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/e8b616ccac5dab4da97444e4b93ac7f4"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/423880d10472f9465725e5d4761b32cd"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/49a23606a3160bb9e5cfdf52a8badd81"
                    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/8c1d17c827ebbc76fca7548e4ca06226"
                    VERBATIM)

add_executable (dbfix-cli dbfix-cli.c
                          hyscan-fix-common.c
                          hyscan-fix-project.c
                          hyscan-fix-track.c
                          hyscan-fix-db.c
                          ${CMAKE_BINARY_DIR}/resources/hyscan-fix-resources.c)

target_link_libraries (dbfix-cli ${GLIB2_LIBRARIES} ${HYSCAN_LIBRARIES})

install (TARGETS dbfix-cli
         COMPONENT runtime
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
