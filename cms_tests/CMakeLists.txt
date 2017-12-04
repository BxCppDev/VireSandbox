# - Top level CMake script for Vire example cms ex01
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(Vire_example_cms_ex01 VERSION "1.0")

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(STATUS "${PROJECT_NAME} requires an out-of-source build.")
  message(STATUS "Please remove these files from ${CMAKE_BINARY_DIR} first:")
  message(STATUS "  CMakeCache.txt")
  message(STATUS "  CMakeFiles")
  message(STATUS "Once these files are removed, create a separate directory")
  message(STATUS "and run CMake from there, pointing it to:")
  message(STATUS "  ${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR "in-source build detected")
endif()

#-----------------------------------------------------------------------
# - Standard UNIX Tool install paths, including relative paths for use
# by applications requiring resources
include(GNUInstallDirs)
# - Validate that certain paths are relative, otherwise relocation may fail
foreach(_dir BINDIR LIBDIR INCLUDEDIR DATAROOTDIR)
  if(IS_ABSOLUTE "${CMAKE_INSTALL_${_dir}}")
    message(FATAL_ERROR "Absolute path for CMAKE_INSTALL_${_dir} not allowed")
  endif()
endforeach()

message(STATUS "[info] Finding Boost 1.60...")
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.60.0 REQUIRED log program_options)
set(Vire_example_cms_ex01_Boost_LIBRARIES Boost::log Boost::program_options)

# BxRabbitMQ
find_package(BxRabbitMQ 0.1 REQUIRED CONFIG COMPONENTS manager) # HINTS )
message (STATUS "BxRabbitMQ_INCLUDE_DIRS ='${BxRabbitMQ_INCLUDE_DIRS}'")
message (STATUS "BxRabbitMQ_LIBRARIES    ='${BxRabbitMQ_LIBRARIES}'")

message(STATUS "[info] Finding Vire 1.0...")
find_package(Vire 1.0 REQUIRED)
get_target_property(Vire_COMPILE_FEATURES Vire::Vire INTERFACE_COMPILE_FEATURES)
get_target_property(Vire_INCLUDE_DIRS     Vire::Vire INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(Vire_LINK_LIBRARIES   Vire::Vire INTERFACE_LINK_LIBRARIES)

# message(STATUS "[info] PROJECT_SOURCE_DIR = '${PROJECT_SOURCE_DIR}'")
message(STATUS "[info] Vire_VERSION              = '${Vire_VERSION}'")
message(STATUS "[info] Vire_INCLUDE_DIRS         = '${Vire_INCLUDE_DIRS}'")
message(STATUS "[info] Vire_INCLUDE_DIR          = '${Vire_INCLUDE_DIR}'")
message(STATUS "[info] Vire_PROTOBUF_INCLUDE_DIR = '${Vire_PROTOBUF_INCLUDE_DIR}'")
message(STATUS "[info] Vire_COMPILE_FEATURES     = '${Vire_COMPILE_FEATURES}'")
message(STATUS "[info] Vire_LINK_LIBRARIES       = '${Vire_LINK_LIBRARIES}'")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${Vire_INCLUDE_DIR})
include_directories(${BxJsontools_INCLUDE_DIRS})
include_directories(${BxProtobuftools_INCLUDE_DIRS})
include_directories(${BxRabbitMQ_INCLUDE_DIRS})
message(STATUS "[info] BxRabbitMQ_INCLUDE_DIRS   = '${BxRabbitMQ_INCLUDE_DIRS}'")

# -- RPATH settings --
# From: https://cmake.org/Wiki/CMake_RPATH_handling
# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)
# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
endif()

link_libraries (${BxRabbitMQ_LIBRARIES} ${BxRabbitMQ_MANAGER_LIBRARIES})

add_executable(vire_domains                  vire_domains.cxx)
add_executable(add_tmpuser_wxCVbn            add_tmpuser_wxCVbn.cxx)
add_executable(del_tmpuser_wxCVbn            del_tmpuser_wxCVbn.cxx)
add_executable(rpc_request                   rpc_request.cxx)
add_executable(fetch_status                  fetch_status.cxx)
add_executable(event_listener                event_listener.cxx)
add_executable(status_event_producer         status_event_producer.cxx)
add_executable(echo_exchange_server          echo_exchange_server.cxx)
#add_executable(exchange_service_perms        exchange_service_perms.cxx)
#add_executable(exchange_request_perms        exchange_request_perms.cxx)
#add_executable(exchange_event_listener_perms exchange_event_listener_perms.cxx)
#add_executable(exchange_event_producer_perms exchange_event_producer_perms.cxx)
#add_executable(direct_service_perms          direct_service_perms.cxx)
#add_executable(direct_request_perms          direct_request_perms.cxx)
#add_executable(direct_event_listener_perms   direct_event_listener_perms.cxx)
#add_executable(direct_event_producer_perms   direct_event_producer_perms.cxx)
#add_executable(echo_direct_server            echo_direct_server.cxx)

target_link_libraries(vire_domains PUBLIC
  Vire::Vire
  ${Vire_example_cms_ex01_Boost_LIBRARIES}
  )

target_link_libraries(add_tmpuser_wxCVbn PUBLIC
  Vire::Vire
  ${Vire_example_cms_ex01_Boost_LIBRARIES}
  )

target_link_libraries(del_tmpuser_wxCVbn PUBLIC
  Vire::Vire
  ${Vire_example_cms_ex01_Boost_LIBRARIES}
  )

target_link_libraries(rpc_request PUBLIC
  Vire::Vire
  ${Vire_example_cms_ex01_Boost_LIBRARIES}
  )

target_link_libraries(fetch_status PUBLIC
  Vire::Vire
  ${Vire_example_cms_ex01_Boost_LIBRARIES}
  )

target_link_libraries(event_listener PUBLIC
  Vire::Vire
  ${Vire_example_cms_ex01_Boost_LIBRARIES}
  )

target_link_libraries(status_event_producer PUBLIC
  Vire::Vire
  ${Vire_example_cms_ex01_Boost_LIBRARIES}
  )

target_link_libraries(echo_exchange_server PUBLIC
  Vire::Vire
  ${Vire_example_cms_ex01_Boost_LIBRARIES}
  )

#target_link_libraries(exchange_service_perms PUBLIC
#  Vire::Vire
#  ${Vire_example_cms_ex01_Boost_LIBRARIES}
#  )
#
#target_link_libraries(exchange_request_perms PUBLIC
#  Vire::Vire
#  ${Vire_example_cms_ex01_Boost_LIBRARIES}
#  )
#
#target_link_libraries(exchange_event_listener_perms PUBLIC
#  Vire::Vire
#  ${Vire_example_cms_ex01_Boost_LIBRARIES}
#  )
#
#target_link_libraries(exchange_event_producer_perms PUBLIC
#  Vire::Vire
#  ${Vire_example_cms_ex01_Boost_LIBRARIES}
#  )
#
#target_link_libraries(direct_service_perms PUBLIC
#  Vire::Vire
#  ${Vire_example_cms_ex01_Boost_LIBRARIES}
#  )
#
#target_link_libraries(direct_request_perms PUBLIC
#  Vire::Vire
#  ${Vire_example_cms_ex01_Boost_LIBRARIES}
#  )
#
#target_link_libraries(direct_event_listener_perms PUBLIC
#  Vire::Vire
#  ${Vire_example_cms_ex01_Boost_LIBRARIES}
#  )
#
#target_link_libraries(direct_event_producer_perms PUBLIC
#  Vire::Vire
#  ${Vire_example_cms_ex01_Boost_LIBRARIES}
#  )
#
#target_link_libraries(echo_direct_server PUBLIC
#  Vire::Vire
#  ${Vire_example_cms_ex01_Boost_LIBRARIES}
#  )

install(FILES
  ${CMAKE_BINARY_DIR}/vire_domains
  ${CMAKE_BINARY_DIR}/add_tmpuser_wxCVbn
  ${CMAKE_BINARY_DIR}/del_tmpuser_wxCVbn
  ${CMAKE_BINARY_DIR}/rpc_request
  ${CMAKE_BINARY_DIR}/fetch_status
  ${CMAKE_BINARY_DIR}/event_listener
  ${CMAKE_BINARY_DIR}/status_event_producer
  ${CMAKE_BINARY_DIR}/echo_exchange_server
#  ${CMAKE_BINARY_DIR}/exchange_service_perms
#  ${CMAKE_BINARY_DIR}/exchange_request_perms
#  ${CMAKE_BINARY_DIR}/exchange_event_listener_perms
#  ${CMAKE_BINARY_DIR}/exchange_event_producer_perms
  #${CMAKE_BINARY_DIR}/direct_service_perms
  #${CMAKE_BINARY_DIR}/direct_request_perms
  #${CMAKE_BINARY_DIR}/direct_event_listener_perms
  #${CMAKE_BINARY_DIR}/direct_event_producer_perms
  #${CMAKE_BINARY_DIR}/echo_direct_server
  DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}"
  PERMISSIONS
  OWNER_READ OWNER_EXECUTE
  GROUP_READ GROUP_EXECUTE
  WORLD_READ WORLD_EXECUTE
  )

# - end
