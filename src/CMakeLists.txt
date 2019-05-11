
find_package(Threads)

set(sources
    acl.c
    auth.c
    config.c
    direct.c
    forward.c
    http.c
    main.c
    ntlm.c
    pages.c
    scanner.c
    socket.c
    utils.c
    xcrypt.c
    acl.h
    auth.h
    config.h
    direct.h
    forward.h
    globals.h
    http.h
    ntlm.h
    pages.h
    scanner.h
    socket.h
    swap.h
    utils.h
    xcrypt.h
)

add_executable(cntlm
    ${sources}
)

target_compile_definitions(cntlm
    PRIVATE VERSION="${PROJECT_VERSION}"
)

target_link_libraries(cntlm
    PRIVATE Threads::Threads
)

source_group("" FILES ${sources})

