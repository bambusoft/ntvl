project(ntvl)
cmake_minimum_required(VERSION 2.6)

# NTVL information
set(NTVL_VERSION 1.0.0)
set(NTVL_OSNAME ${CMAKE_SYSTEM})

# NTVL specific params
if(NOT DEFINED NTVL_OPTION_AES)
set(NTVL_OPTION_AES ON)
endif(NOT DEFINED NTVL_OPTION_AES)

add_definitions(-DNTVL_VERSION='\"${NTVL_VERSION}\"' -DNTVL_OSNAME='\"${NTVL_OSNAME}\"')

if(NTVL_OPTION_AES)
add_definitions(-DNTVL_HAVE_AES)
endif(NTVL_OPTION_AES)

# Build information
if(NOT DEFINED BUILD_SHARED_LIBS)
set(BUILD_SHARED_LIBS OFF)
endif(NOT DEFINED BUILD_SHARED_LIBS)

if(NOT DEFINED CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE None)
endif(NOT DEFINED CMAKE_BUILD_TYPE)
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

#Ultrasparc64 users experiencing SIGBUS should try the following gcc options
#(thanks to Robert Gibbon) 
#PLATOPTS_SPARC64=-mcpu=ultrasparc -pipe -fomit-frame-pointer -ffast-math -finline-functions -fweb -frename-registers -mapp-regs

# None
set(CMAKE_C_FLAGS "-Wall -Wshadow -Wpointer-arith -Wmissing-declarations -Wnested-externs")
set(CMAKE_CXX_FLAGS "-Wall -Wshadow -Wpointer-arith -Wmissing-declarations -Wnested-externs")
# Debug 
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
# Release
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

## DEBUG FOR CMAKE
#message(${NTVL_VERSION}) 
#message(${NTVL_OSNAME})
##message(${CMAKE_BUILD_TYPE})
#message(${NTVL_OPTION_AES})
## DEBUG FOR CMAKE

add_library(ntvl ntvl.c
                ntvl_keyfile.c
                wire.c
                minilzo.c
                twofish.c
                transform_null.c
                transform_tf.c
                transform_aes.c
                tuntap_freebsd.c
                tuntap_netbsd.c
                tuntap_linux.c
                tuntap_osx.c
                version.c
            )

if(DEFINED WIN32)
add_subdirectory(win32)
target_link_libraries(ntvl ntvl_win32)
endif(DEFINED WIN32)

if(NTVL_OPTION_AES)
target_link_libraries(ntvl crypto)
endif(NTVL_OPTION_AES)

# For Solaris (or OpenSolaris?)
#target_link_libraries(ntvl socket nsl)

add_executable(node node.c)
target_link_libraries(node ntvl)

add_executable(supernode sn.c)
target_link_libraries(supernode ntvl)

#add_executable(tunnel)
#target_link_libraries(tunnel ntvl)

add_executable(test test.c)
target_link_libraries(test ntvl)

add_executable(benchmark benchmark.c)
target_link_libraries(benchmark ntvl)

install(TARGETS node supernode tunnel
        RUNTIME DESTINATION sbin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
       )

# Documentation
if(DEFINED UNIX)
add_dependencies(ntvl doc)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/docs)
add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/docs/node.8.gz
                   COMMAND gzip -c ${PROJECT_SOURCE_DIR}/node.8 > ${PROJECT_BINARY_DIR}/docs/node.8.gz
                   DEPENDS ${PROJECT_SOURCE_DIR}/node.8
                  )

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/docs/supernode.1.gz
                   COMMAND gzip -c ${PROJECT_SOURCE_DIR}/supernode.1 > ${PROJECT_BINARY_DIR}/docs/supernode.1.gz
                   DEPENDS ${PROJECT_SOURCE_DIR}/supernode.1
                  )

 add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/docs/tunnel.1.gz
                   COMMAND gzip -c ${PROJECT_SOURCE_DIR}/tunnel.1 > ${PROJECT_BINARY_DIR}/docs/tunnel.1.gz
                   DEPENDS ${PROJECT_SOURCE_DIR}/tunnel.1
                  )

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/docs/ntvl-v1.0.0.gz
                   COMMAND gzip -c ${PROJECT_SOURCE_DIR}/ntvl-v1.0.0 > ${PROJECT_BINARY_DIR}/docs/ntvl-v1.0.0.gz
                   DEPENDS ${PROJECT_SOURCE_DIR}/ntvl-v1.0.0
                  )

add_custom_target(doc DEPENDS ${PROJECT_BINARY_DIR}/docs/node.8.gz
                              ${PROJECT_BINARY_DIR}/docs/supernode.1.gz
                              ${PROJECT_BINARY_DIR}/docs/ntvl-v1.0.0.gz
                 )

set_source_files_properties(${PROJECT_BINARY_DIR}/docs/node.8.gz
                            ${PROJECT_BINARY_DIR}/docs/supernode.1.gz
                            ${PROJECT_BINARY_DIR}/docs/ntvl-v1.0.0.gz
                            PROPERTIES GENERATED 1)

install(FILES ${PROJECT_BINARY_DIR}/docs/node.8.gz
        DESTINATION /usr/share/man8)
install(FILES ${PROJECT_BINARY_DIR}/docs/tunnel.1.gz
        DESTINATION /usr/share/man1)
install(FILES ${PROJECT_BINARY_DIR}/docs/supernode.1.gz
        DESTINATION /usr/share/man1)
install(FILES ${PROJECT_BINARY_DIR}/docs/ntvl-v1.0.0.gz
        DESTINATION /usr/share/man7)

endif(DEFINED UNIX)
