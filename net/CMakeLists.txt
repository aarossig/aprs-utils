################################################################################
#
# net
#
################################################################################

# net ##########################################################################

add_library(net
  aprs_interface.cc
  internet_aprs_interface.cc
  packet_chunk_receiver.cc
  tnc_aprs_interface.cc
)

target_link_libraries(net
  packet_proto
  util
  ${SDL_net_LIBRARIES}
)

target_include_directories(net PUBLIC
  ${SDL_net_INCLUDE_DIRS}
)
