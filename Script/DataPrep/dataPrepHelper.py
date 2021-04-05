import json

def get_output_folder(json_data):
	res = ''
	if 'Output Folder' in json_data:
		res = json_data['Output Folder']
	return res

def get_project_name(json_data):
	res = ''
	if 'Project Name' in json_data:
		res = json_data['Project Name']
	return res
    
def get_template_map_name(json_data):
	res = ''
	if 'Template Map Name' in json_data:
		res = json_data['Template Map Name']
	return res

def get_CAD_files_path(json_data):
	res = ['']
	if 'CAD Files Path' in json_data:
		res = json_data['CAD Files Path']
	return res

def get_tesselation_paramenters(json_data):
	#Chord Tolerance/Edge Length/Normal Tolerance
	res = [10.0,100.0,90.0]
	if 'Tesselation Parameters' in json_data:
		temp = json_data['Tesselation Parameters']
		if 'Chord Tolerance' in temp and 'Edge Length' in temp and 'Normal Tolerance' in temp:
			res = [float(temp['Chord Tolerance']),float(temp['Edge Length']),float(temp['Normal Tolerance'])]
	return res

def get_content_folder(json_data):
	res = ''
	if 'Content Folder' in json_data:
		res = json_data['Content Folder']
	return res