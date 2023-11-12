import bpy
import os

def create_wf_mesh(samples, as_grid:bool=False):
    
    verts = []
    edges = []
    faces = []
    
    for row in range(0, len(samples)):
        for col in range(0, len(samples[0])):
            vert_index = row * len(samples[0]) + col
            
            if as_grid:    
                verts.append( (row, col, samples[row][col]) )
            else:
                verts.append( (vert_index, 0, samples[row][col]) )
            
            # avoids adding edges between rows of a grid if it is a grid setup
            if vert_index > 0 and ((not as_grid) or vert_index % len(samples[0]) > 0):
                edges.append( (vert_index, vert_index - 1) )
                
    mesh = bpy.data.meshes.new("wf_mesh") 
    obj = bpy.data.objects.new("wf_obj", mesh) 
    col = bpy.data.collections.get("vcv_waveforms") 
    col.objects.link(obj)
    bpy.context.view_layer.objects.active = obj

    mesh.from_pydata(verts, edges, faces)
    
    return

vcv_dev = os.environ['HOME'] + "/VCV_dev"

kframes    = open( bpy.path.abspath("//keyframes.csv"), "r" ).readlines()
wf_kframes = open( bpy.path.abspath("//waveform_keyframes.csv"), "r" ).readlines()

vcv_gn     = bpy.data.collections['vcv_gn_templates']
vcv_tracks = bpy.data.collections['vcv_tracks']

num_tracks = len(kframes[0].split(","))

for i in range(0, num_tracks):
    track_obj = bpy.data.objects.new("track_" + str(i), None)
    track_obj.location = (i, 0, 0)
    vcv_tracks.objects.link(track_obj)

for frame in range(0, len(kframes)):
    
    tracks = [ float(string) for string in (kframes[frame].split(",")) ]
    
    for track_index in range(0, len(tracks)):
        # create empties that move up and down according to the value of each track
        # good for copying and pasting drivers
        vcv_tracks.objects["track_" + str(track_index)].location[2] = tracks[track_index]
        vcv_tracks.objects["track_" + str(track_index)].keyframe_insert("location", frame=frame)
        
        # create a node group that contains the values of each track
        # good for making the keyframes available to multiple geo nodes setups
        bpy.data.node_groups["VCV_keyframes"].nodes["track_" + str(track_index)].outputs[0].default_value = tracks[track_index]
        bpy.data.node_groups["VCV_keyframes"].nodes["track_" + str(track_index)].outputs[0].keyframe_insert("default_value", frame=frame)

wf_sample_matrix = []

for row in wf_kframes:
    wf_sample_matrix.append([float(sample) for sample in row.split(",")])

create_wf_mesh(wf_sample_matrix, as_grid=False)