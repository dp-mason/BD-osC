import bpy
import os

vcv_dev = os.environ['HOME'] + "/VCV_dev"

kframes    = open( os.path.join(vcv_dev + "/VCV-Keyframes/blender_files/keyframes.csv"), "r" ).readlines()
wf_kframes = open( os.path.join(vcv_dev + "/VCV-Keyframes/blender_files/waveform_keyframes.csv"), "r" ).readlines()

vcv_gn     = bpy.data.collections['vcv_gn_templates']
vcv_tracks = bpy.data.collections['vcv_tracks']

for i in range(0,16):
    track_obj = bpy.data.objects.new("track_" + str(i), None)
    track_obj.location = (i, 0, 0)
    vcv_tracks.objects.link(track_obj)

for frame in range(0, len(kframes)):
    
    tracks = [ float(string) for string in (kframes[frame].split(","))[0:-1] ]
    
    for track_index in range(0, len(tracks)):
        # create empties that move up and down according to the value of each track
        # good for copying and pasting drivers
        vcv_tracks.objects["track_" + str(track_index)].location[2] = tracks[track_index]
        vcv_tracks.objects["track_" + str(track_index)].keyframe_insert("location", frame=frame)
        
        # create a node group that contains the values of each track
        # good for making the keyframes available to multiple geo nodes setups
        bpy.data.node_groups["VCV_keyframes"].nodes["track_" + str(track_index)].outputs[0].default_value = tracks[track_index]
        bpy.data.node_groups["VCV_keyframes"].nodes["track_" + str(track_index)].outputs[0].keyframe_insert("default_value", frame=frame)
        
    