####################################################################################################
#             Providing LibXML2 library - https://gitlab.gnome.org/GNOME/libxml2.git               #
#                                                                                                  #
# To use the LibXML2 library simply link to it:                                                    #
# target_link_libraries(<project> PRIVATE LibXml2::LibXml2).                                       #
# This automatically sets all required information such as include directories, definitions etc.   #
####################################################################################################


set(open62541_version_tag "v1.0")

if(NOT TARGET Open62541::Open62541)
  message(STATUS "Make Open62541 (${open62541_version_tag}) available.")
  include(FetchContent)
  FetchContent_Declare(
    open62541provider
    GIT_REPOSITORY "https://github.com/open62541/open62541.git"
    GIT_TAG ${open62541_version_tag}
    )
  FetchContent_GetProperties(open62541provider)

  if(UNIX)
    set(build_options "-j4")
  else()
    set(build_options "-m:4")
  endif()

  set(_link_library "${CMAKE_BINARY_DIR}/install/open62541/lib/${CMAKE_STATIC_LIBRARY_PREFIX}open62541${CMAKE_STATIC_LIBRARY_SUFFIX}")
  
  if(EXISTS "${_link_library}")
    message(STATUS "  * Reusing previously built open62541 library")
  else()
    message(STATUS "  * Donwload open62541")
    FetchContent_Populate(open62541provider)
    
    message(STATUS "  * Configure open62541")
    execute_process(
      COMMAND ${CMAKE_COMMAND}
      -DBUILD_SHARED_LIBS=FALSE
      -DCMAKE_BUILD_TYPE=RelWithDebInfo
      -DUA_NAMESPACE_ZERO=FULL
      -DUA_ENABLE_SUBSCRIPTIONS_EVENTS=ON
      -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
      -DWINVER=${WINVER}
      -DBUILD_SHARED_LIBS=OFF
      -DOPEN62541_VERSION=${open62541_version_tag}
      -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/open62541
      ${open62541provider_SOURCE_DIR}
      WORKING_DIRECTORY ${open62541provider_BINARY_DIR}
      OUTPUT_FILE       ${open62541provider_BINARY_DIR}/configure_output.log
      ERROR_FILE        ${open62541provider_BINARY_DIR}/configure_output.log
      )
    file(READ "${open62541provider_BINARY_DIR}/configure_output.log" _output)
    message("============================================================")
    message("${_output}")
    message("============================================================")
    
    
    message(STATUS "  * Build Open62541")
    execute_process(
      COMMAND ${CMAKE_COMMAND} --build . --target install -- ${build_options}
      WORKING_DIRECTORY ${open62541provider_BINARY_DIR}
      OUTPUT_FILE       ${open62541provider_BINARY_DIR}/build_output.log
      ERROR_FILE        ${open62541provider_BINARY_DIR}/build_output.log
      )
    file(READ "${open62541provider_BINARY_DIR}/build_output.log" _output)
    message("============================================================")
    message("${_output}")
    message("============================================================")
  endif()
  
  # Make Open62541 library available within CMake
  add_library(Open62541::Open62541 STATIC IMPORTED)
  set_target_properties(Open62541::Open62541 PROPERTIES
    IMPORTED_LOCATION "${_link_library}"
    INTERFACE_INCLUDE_DIRECTORIES "${CUSTOM_MAKE_PATH}/include;${CMAKE_BINARY_DIR}/install/open62541/include"
    )
endif()
