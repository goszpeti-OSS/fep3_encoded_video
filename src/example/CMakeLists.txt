#
# Copyright @ 2021 VW Group. All rights reserved.
# 
#     This Source Code Form is subject to the terms of the Mozilla
#     Public License, v. 2.0. If a copy of the MPL was not distributed
#     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
# If it is not possible or desirable to put the notice in a particular file, then
# You may include the notice in a location (such as a LICENSE file in a
# relevant directory) where a recipient would be likely to look for such a notice.
# 
# You may add additional accurate notices of copyright ownership.
# 
#
fep3_participant_create_copy_target(easy_core_example)

##################################################################
# DEMO EASY Sender with a CORE ElementBase
##################################################################
add_executable(video_sender
               video_sender.cpp
)

# link against the fep3_participant
target_link_libraries(video_sender PRIVATE
                      fep3_participant_core
                      qt::qt)
add_dependencies(video_sender fep_participant_file_copy_easy_core_example)
target_compile_options(video_sender PRIVATE "/Od")

# fep3_participant_install(video_sender examples/bin)

##################################################################
# DEMO EASY Receiver with a CORE ElementBase
##################################################################
add_executable(video_player
               video_player.cpp
               video.h
               )
fep_fast_dds_plugin_deploy(video_player)
target_compile_options(video_player PRIVATE "/Od")

# link against the fep3_participant_core API
target_link_libraries(video_player PRIVATE
                      fep3_participant_core
                      qt::qt)
add_dependencies(video_player fep_participant_file_copy_easy_core_example)
# fep3_participant_install(video_player examples/bin)

