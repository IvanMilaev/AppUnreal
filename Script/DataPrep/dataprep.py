#execfile("D:/projects/HotLoad/ImportScript/dataprep.py")
import os
import sys
sys.path=sys.path + ['D:/ue/pacs/hotload/WindowsNoEditor/HotLoad/Dataprep']
import unreal
import json
import dataPrepHelper as dph

######################

def import_cad(in_content_folder, in_cad_file_path, in_tesselation_parameters):
	#init datasmith CAD scene import from a CAD file and a target content directory
	datasmith_scene = unreal.DatasmithSceneElement.construct_datasmith_scene_from_file(in_cad_file_path)
	#check scene initialization
	if datasmith_scene is None:
		print "Error: LoadDatasmith Failed"
		quit()

	# set CAD import options
	import_options = datasmith_scene.get_options()
	import_options.base_options.scene_handling = unreal.DatasmithImportScene.CURRENT_LEVEL
	import_options.base_options.static_mesh_options.generate_lightmap_u_vs = False

	tessellation_options = datasmith_scene.get_options(unreal.DatasmithCommonTessellationOptions)
	if tessellation_options:
		tessellation_options.options.chord_tolerance = in_tesselation_parameters[0]
		tessellation_options.options.max_edge_length = in_tesselation_parameters[1]
		tessellation_options.options.normal_tolerance = in_tesselation_parameters[2]
		tessellation_options.options.stitching_technique = unreal.DatasmithCADStitchingTechnique.STITCHING_NONE

	#import the scene into the current level
	result = datasmith_scene.import_scene(in_content_folder)
	#process the scene for import
	if not result.import_succeed:
		print "Error: Datasmith scene import failed"
		quit()
	return result

#the root actors have to be in the list of the actors
#we do not go higher in the world outliner
def find_root_actors_from_imported_actors(actors):
	root_actors = set()
	checked_actors = set()
	for a in actors:
		if a in checked_actors:
			continue
		checked_actors.add(a)
		old_parent_actor = a
		end_parsing = False
		while not end_parsing:
			parent_component = old_parent_actor.root_component.get_attach_parent()
			if parent_component:
				parent_actor = parent_component.get_owner()
				if parent_actor not in actors:
					#found a root (parent out of actors list)
					root_actors.add(old_parent_actor)
					end_parsing = True
				else:
					if parent_actor in checked_actors:
						end_parsing = True
					else:
						checked_actors.add(parent_actor)
						old_parent_actor = parent_actor
			else:
				#found a root (no more parent)
				root_actors.add(old_parent_actor)
				end_parsing = True
	return root_actors
		

def update_actors(result,root_actor):
	#if false flatten hierachy and remove all datasmith scene nodes
	keep_product_tree = False
	#just keep the static mesh actors
	static_mesh_actors = []
	other_actors = []
	if keep_product_tree:
		#Change mobility of all actors
		for a in result.imported_actors:
			a.root_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE)
		#reattach roots
		root_actors = find_root_actors_from_imported_actors(result.imported_actors)
		for a in root_actors:
			a.attach_to_actor(root_actor,'',attachment_rule,attachment_rule,attachment_rule,False)
	else:
		for a in result.imported_actors:
			if a.get_class() != unreal.StaticMeshActor.static_class():
				other_actors.append(a)
			else:
				a.root_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE)
				a.attach_to_actor(root_actor,'',attachment_rule,attachment_rule,attachment_rule,False)
				static_mesh_actors.append(a)
		#move all actors up
		#root_actor.add_actor_world_offset(v,False,False)
		#remove non static mesh actors #TODO this has some issues
		for a in other_actors:
			unreal.EditorLevelLibrary.destroy_actor(a)

			
############### Main starts here ###############

#importing JSON file
#print >> sys.stderr, ('#############################################################################')
#print >> sys.stderr, sys.argv
#import_json_file = 'D:/projects/HotLoad/DataPrep/current_job.json'

if len(sys.argv) == 2:
	import_json_file = sys.argv[1]
	#print >> sys.stderr, 'Job File ' + import_json_file
else:
	unreal.log_error('Data prep: missing job file')
	exit()
	
json_file = open(import_json_file,'r')
json_data = json.load(json_file)
json_file.close()

#setting variable from JSON job file
map_name = dph.get_project_name(json_data)
template_map_name = dph.get_template_map_name(json_data)
cad_files_paths = dph.get_CAD_files_path(json_data)
tesselation_parameters = dph.get_tesselation_paramenters(json_data)
content_folder = dph.get_content_folder(json_data)
#print >> sys.stderr, 'Tesselation Parameters ' + str(tesselation_parameters)

#setting over variables
attachment_rule = unreal.AttachmentRule.KEEP_WORLD
v = unreal.Vector(0,0,100)

#unreal.EditorAssetLibrary.delete_directory(content_folder)
if unreal.EditorAssetLibrary.does_directory_exist(content_folder):
	unreal.EditorAssetLibrary.delete_directory(content_folder)
	unreal.log('deleting folder ' + content_folder)
unreal.EditorLevelLibrary.new_level_from_template(content_folder+'/'+map_name,template_map_name)
#unreal.EditorLevelLibrary.new_level_from_template(content_folder+'/'+map_name,"/Game/ProductViewer/Levels/ProductViewer")
actors = unreal.EditorLevelLibrary.get_all_level_actors()
root = actors[0]
bFoundRoot = False
for a in actors:
	if a.get_actor_label() == 'root':
		root = a
		bFoundRoot = True
		break
if not bFoundRoot:
	print "not found the root"
	exit()
unreal.log('iterating over CAD files')
for cfp in cad_files_paths:
	unreal.log('import ' + cfp)
	scene_import_result = import_cad(content_folder,cfp,tesselation_parameters)
	unreal.log('update actors')
	update_actors(scene_import_result,root)

unreal.log('saving assets')
unreal.EditorAssetLibrary.save_directory(content_folder)
unreal.EditorLevelLibrary.save_current_level()
