################################################################################
#
# aprs-file-copy
#
################################################################################

# aprs-file-copy ###############################################################

add_executable(aprs-file-copy
  file_sender.cc
  main.cc
)

target_link_libraries(aprs-file-copy
  util
  ${SDL_net_LIBRARIES}
)

target_include_directories(aprs-file-copy PRIVATE
  ${SDL_net_INCLUDE_DIRS}
)
