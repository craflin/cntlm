
find_program(GZIP_EXECUTABLE
    NAMES gzip
)

add_custom_target(manpage ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/cntlm.1.gz"
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cntlm.1.gz"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cntlm.1"
    COMMAND "${GZIP_EXECUTABLE}" -c "${CMAKE_CURRENT_SOURCE_DIR}/cntlm.1" > cntlm.1.gz
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cntlm.1.gz" DESTINATION usr/share/man/man1)
