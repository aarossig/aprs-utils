################################################################################
#
# proto
#
################################################################################

# Packet Proto #################################################################

PROTOBUF_GENERATE_CPP(packet_proto_hdrs
  packet_proto_srcs
  packet.proto
)

add_library(packet_proto
  ${packet_proto_hdrs}
  ${packet_proto_srcs}
)

target_include_directories(packet_proto PUBLIC ${PROJECT_BINARY_DIR})
target_include_directories(packet_proto PUBLIC ${Protobuf_INCLUDE_DIRS})
target_link_libraries(packet_proto ${Protobuf_LIBRARIES})
