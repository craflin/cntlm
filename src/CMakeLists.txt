
find_package(Threads)

add_executable(cntlm
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

target_compile_definitions(cntlm
    PRIVATE VERSION="${PROJECT_VERSION}"
)

if(MSVC)
    target_sources(cntlm PRIVATE
        win32/arpa/inet.h
        win32/netinet/in.h
        win32/sys/select.h
        win32/sys/socket.h
        win32/sys/time.h
        win32/fnmatch.cpp
        win32/fnmatch.h
        win32/netdb.h
        win32/pthread.cpp
        win32/pthread.h
        win32/pwd.h
        win32/strings.h
        win32/syslog.cpp
        win32/syslog.h
        win32/unistd.h
        win32/getopt.cpp
        win32/getopt.h
    )
    target_include_directories(cntlm
        PRIVATE win32
    )
    target_compile_options(cntlm
        PRIVATE /wd4996
    )
endif()

target_link_libraries(cntlm
    PRIVATE Threads::Threads
)

if(WIN32)
    target_link_libraries(cntlm
        PRIVATE ws2_32
    )
endif()

# repair file listing in Visual Studio
get_target_property(sources cntlm SOURCES)
foreach(file ${sources})
    get_filename_component(dir "${file}" DIRECTORY)
    string(REPLACE "/" "\\" dir "${dir}")
    source_group("${dir}" FILES ${file})
endforeach()

