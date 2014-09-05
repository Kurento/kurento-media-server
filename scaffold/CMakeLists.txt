cmake_minimum_required(VERSION 2.8)

set(SCAFFOLD_TEMPLATES
  CMakeLists_root.txt.template
  CMakeLists_src.txt.template
  CMakeLists_server.txt.template
  CMakeLists_server_plugin.txt.template
  CMakeLists_plugin.txt.template
  filter.kmd.json.template
  filter.Filter.kmd.json.template
  gstplugin.cpp.template
  gstplugin.h.template
  plugin.c.template
  control.template
  name.install.template
  name.install.OpenCV.template
  rules.template
  changelog.template
  config.h.cmake.template
  name-dev.install.template
)

install(FILES ${SCAFFOLD_TEMPLATES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CUSTOM_PREFIX}/scaffold/)

configure_file(kurento-module-scaffold.sh ${CMAKE_CURRENT_BINARY_DIR}/kurento-module-scaffold.sh @ONLY)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/kurento-module-scaffold.sh
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
  DESTINATION bin
)