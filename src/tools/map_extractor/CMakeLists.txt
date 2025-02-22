# This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
)

if(WIN32)
  list(APPEND PRIVATE_SOURCES ${sources_windows})
endif()

add_executable(mapextractor
  ${PRIVATE_SOURCES}
)

target_link_libraries(mapextractor
  PRIVATE
    trinity-core-interface
  PUBLIC
    common
    storm)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
)

target_include_directories(mapextractor
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})
    
set_target_properties(mapextractor
  PROPERTIES
    FOLDER
      "tools")

if(UNIX)
  install(TARGETS mapextractor DESTINATION bin)
elseif(WIN32)
  install(TARGETS mapextractor DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
