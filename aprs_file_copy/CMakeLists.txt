################################################################################
#
# aprs-file-copy
#
################################################################################

# aprs-file-copy ###############################################################

add_executable(aprs-file-copy
  file_receiver.cc
  file_sender.cc
  main.cc
)

target_link_libraries(aprs-file-copy
  net
  util
)
