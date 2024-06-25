import bpy
import os

# TODO: Create cleanup script, it doesn't matter that much, but some keyframes are tucked away in a place that is tricky
#   to delete

def csv_to_matrix(filename:str):
    text_lines = open( bpy.path.abspath("//" + filename), "r" ).readlines()

    wf_sample_matrix = []

    for row in text_lines:
        wf_sample_matrix.append([float(sample) for sample in row.split(",")])
    
    return wf_sample_matrix

# TODO: add smoothing between keyframes to reduce jitter, le the previous keyframe influence the current
#       don't let the smoothing compound though
# TODO: allow subsamples, ex. recording audio keyframes at 120 fps and creating visual keyframes at 60fps or 30fps
def create_wf_mesh(csv_filename:str, as_grid:bool=False):
    
    wf_kf_matrix = csv_to_matrix(csv_filename)
    
    verts = []
    edges = []
    faces = []
    
    # TODO: make it so that the physical width of the wave remains constant despite the number of samples in the row (padded left and right by -99) 
    
    for row in range(0, len(wf_kf_matrix)):
        for col in range( 0, len(wf_kf_matrix[0]) ):
            vert_index = row * len(wf_kf_matrix[0]) + col
            
            if as_grid:    
                verts.append( (row, -1 * col, wf_kf_matrix[row][col]) )
            else:
                verts.append( (vert_index, 0, wf_kf_matrix[row][col]) )
            
            # avoids adding edges between rows of a grid if it is a grid setup
            if vert_index > 0 and ((not as_grid) or vert_index % len(wf_kf_matrix[0]) > 0):
                edges.append( (vert_index, vert_index - 1) )
    
    obj_name = csv_filename.replace(".csv","") + "_obj"
    
    mesh = bpy.data.meshes.new( csv_filename.replace(".csv","") + "_mesh" ) 
    obj = bpy.data.objects.new( obj_name, mesh ) 
    col = bpy.data.collections.get("vcv_waveforms") 
    col.objects.link(obj)
    bpy.context.view_layer.objects.active = obj

    mesh.from_pydata(verts, edges, faces)
    
    modif = obj.modifiers.new("wf_nodes", "NODES")
    modif.node_group = bpy.data.node_groups["wf_nodes_grid"]
    modif["Socket_2"] = len(wf_kf_matrix[0])
    
    return obj




vcv_dev = os.environ['HOME'] + "/VCV_dev"

kframes    = open( bpy.path.abspath("//keyframes.csv"), "r" ).readlines()

vcv_gn     = bpy.data.collections['vcv_gn_templates']
vcv_tracks = bpy.data.collections['vcv_tracks']
vcv_wfs    = bpy.data.collections['vcv_waveforms']

# cleanup old animation data
bpy.data.node_groups["VCV_keyframes"].animation_data_clear()
for obj in vcv_tracks.objects:
    if len(obj.name) >= 6 and obj.name[0:6] == "track_":
        bpy.data.objects.remove(obj)
for obj in vcv_wfs.objects:
    if len(obj.name) >= 5 and obj.name[0:5] == "wave_":
        bpy.data.objects.remove(obj)

# TODO: Save the set framerate from the vcv rack module
bpy.data.scenes["Scene"].render.fps = int(30)

num_tracks = len(kframes[0].split(","))

# Set the frame range
bpy.data.scenes["Scene"].frame_start = 0
bpy.data.scenes["Scene"].frame_end = len(kframes)

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


wf_mesh_obj = create_wf_mesh("wave_1_keyframes.csv", as_grid=True)
wf_mesh_obj = create_wf_mesh("wave_2_keyframes.csv", as_grid=True)
wf_mesh_obj = create_wf_mesh("wave_3_keyframes.csv", as_grid=True)
wf_mesh_obj = create_wf_mesh("wave_4_keyframes.csv", as_grid=True)
wf_mesh_obj = create_wf_mesh("wave_5_keyframes.csv", as_grid=True)