################################################################################
#
# util
#
################################################################################

# util #########################################################################

add_library(util
  callsign.cc
  file.cc
  log.h
  string.cc
  time.cc
)

target_link_libraries(util
  b64
)

target_link_directories(util PRIVATE "/usr/lib/x86-64-linux-gnu")

target_include_directories(util PUBLIC ${PROJECT_SOURCE_DIR})
