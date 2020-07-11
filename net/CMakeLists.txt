################################################################################
#
# net
#
################################################################################

# net ##########################################################################

add_library(net
  tnc_connection.cc
)

target_link_libraries(net
  util
  ${SDL_net_LIBRARIES}
)

target_include_directories(net PUBLIC
  ${SDL_net_INCLUDE_DIRS}
)
