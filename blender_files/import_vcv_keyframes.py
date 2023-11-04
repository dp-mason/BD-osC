import bpy
import os

print(os.environ.get('VCV_DEV'))

kframe_file = open( os.path.join("/home/bdc/VCV_dev/VCV-Keyframes/keyframes.csv"), "r" )

kframes = kframe_file.readlines()

vcv_tracks = bpy.data.collections['vcv_tracks']

for i in range(0,16):
    track_obj = bpy.data.objects.new("track_" + str(i), None)
    track_obj.location = (i, 0, 0)
    vcv_tracks.objects.link(track_obj)

for frame in range(0, len(kframes)):
    
    tracks = [ float(string) for string in (kframes[frame].split(","))[0:-1] ]
    
    for track_index in range(0, len(tracks)):
        vcv_tracks.objects[track_index].location[2] = tracks[track_index]
        vcv_tracks.objects[track_index].keyframe_insert("location", frame=frame)