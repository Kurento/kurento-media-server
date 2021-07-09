cmake_minimum_required(VERSION 2.8)

set(SCAFFOLD_TEMPLATES
  CMakeLists_plugin.txt.template
  CMakeLists_root.txt.template
  CMakeLists_server_plugin.txt.template
  CMakeLists_server.txt.template
  CMakeLists_src.txt.template
  config.h.cmake.template
  debian-changelog.template
  debian-control.template
  debian-name-dev.install.template
  debian-name.install.template
  debian-name.install.OpenCV.template
  debian-rules.template
  filter.Filter.kmd.json.template
  filter.kmd.json.template
  gstplugin.cpp.template
  gstplugin.h.template
  plugin.c.template
)

install(
  FILES ${SCAFFOLD_TEMPLATES}
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CUSTOM_PREFIX}/scaffold/
)

configure_file(
  kurento-module-scaffold
  ${CMAKE_CURRENT_BINARY_DIR}/kurento-module-scaffold @ONLY
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/kurento-module-scaffold
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
  DESTINATION bin
)
