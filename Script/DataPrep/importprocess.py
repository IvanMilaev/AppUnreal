import os
import subprocess
import shutil


# helper function to execute external command and progressively display results
def execute_and_print(args):
	process = subprocess.Popen(args, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
	for line in process.stdout:
		print(line.rstrip().decode('latin-1'))
	#process.wait()
	return process.returncode

project_name = 'Import'
main_folder_path = 'D:/projects/HotLoad/DataPrep'
ue4_path = 'D:/UE/UE_4.25/Engine/Binaries/Win64'
ue4editor_cmd = os.path.join(ue4_path, 'UE4Editor-Cmd.exe')
template_project_folder = main_folder_path+'/Template'
template_project = os.path.join(template_project_folder,'Template.uproject')
data_prep_script = main_folder_path+'/dataprep.py'

editor_directory = os.path.join(template_project_folder,'content',project_name)
cooked_directory = os.path.join(template_project_folder,'Saved/Cooked/WindowsNoEditor/Template/Content',project_name)

output_content_directory = os.path.join((main_folder_path+'/Data'),project_name)
output_editor_directory = os.path.join(output_content_directory,'editor/content')
output_cooked_directory = os.path.join(output_content_directory,'cooked/content')

# perform data prep
print(" - data prep")
dataprep_result = execute_and_print([ue4editor_cmd, template_project, '-run=pythonscript', '-script='+data_prep_script+' '+main_folder_path+'/current_job.json'])
if dataprep_result != 0:
	print("Failed data prep")

# cooking assets
print(" - cooking assets")
# remove existing cooked content if any
if os.path.exists(cooked_directory):
	shutil.rmtree(cooked_directory)
cooking_result = execute_and_print([ue4editor_cmd, template_project, '-run=cook', '-targetplatform=WindowsNoEditor', '-cookall', '-skipeditorcontent'])
if cooking_result != 0:
	print("Failed cooking assets")

# moving asset to data folder
print(" - moving assets to destination folders")
if not os.path.exists(output_content_directory):
	os.makedirs(output_content_directory)
if not os.path.exists(output_editor_directory):
	os.makedirs(output_editor_directory)
shutil.move(editor_directory, output_editor_directory)
if not os.path.exists(output_cooked_directory):
	os.makedirs(output_cooked_directory)
shutil.move(cooked_directory, output_cooked_directory)