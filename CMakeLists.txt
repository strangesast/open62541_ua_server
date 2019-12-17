IF (WIN32)
    set(Boost_USE_STATIC_LIBS       ON)  # only find static libs
    set(Boost_USE_MULTITHREADED     ON)
    set(Boost_USE_STATIC_RUNTIME    OFF)
    set(Boost_ARCHITECTURE          "-x64")
    set(Boost_DEBUG                 ON)
    set(OPENSSL_USE_STATIC_LIBS     TRUE)
    set(open62541_INCLUDE_DIR               c:/open62541/include)
    set(open62541_LIBRARY               c:/open62541/lib/libopen62541.a)

ENDIF()

project(opcua-MTServer)
cmake_minimum_required(VERSION 3.10)
FIND_PACKAGE(Boost REQUIRED COMPONENTS thread filesystem log)
FIND_PACKAGE(OpenSSL REQUIRED)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} ${open62541_INCLUDE_DIR} )
add_compile_options(-w -std=c++11)

ADD_EXECUTABLE(opcua-MTServer opcua_server.cpp types_mgr.cpp mtconnect_ids.cpp nodeset.cpp agenthandler.cpp httpreader.cpp settings.cpp worker.cpp util.cpp types_mtconnect_generated.cpp)

IF (WIN32)
    TARGET_LINK_LIBRARIES( opcua-MTServer LINK_PUBLIC ${Boost_LIBRARIES} Boost::thread Boost::filesystem OpenSSL::SSL ${OPENSSL_CRYPTO_LIBRARY} ${open62541_LIBRARY} ws2_32 mswsock advapi32)

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin" COMPONENT applications)

    #
    # CPACK macros below here
    #
    set (CPACK_GENERATOR NSIS)
    SET (CPACK_NSIS_MODIFY_PATH ON)

ELSE()

FIND_PACKAGE(open62541 REQUIRED)

    TARGET_LINK_LIBRARIES( opcua-MTServer LINK_PUBLIC ${Boost_LIBRARIES} OpenSSL::SSL open62541::open62541)

    install(TARGETS ${PROJECT_NAME} DESTINATION ".")

    set (CPACK_GENERATOR ZIP)
ENDIF()

set (CPACK_PACKAGE_NAME "opcua-MTServer")
set (CPACK_PACKAGE_VENDOR "MTConnect")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "opcua-MTServer - OPCUA gateway server to MTConnect agents")
set (CPACK_PACKAGE_VERSION "1.0.0")
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "MTConnect OPCUA Server")


# Must be after the last CPACK macros
include(CPack)

