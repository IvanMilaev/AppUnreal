import socket
import sys
import json
import os
import subprocess
import dataPrepHelper as dph
import shutil
import zipfile

'''
<<< CONFIG DATAPREP
>>> ACK
<<< CONTENT
>>> ACK
<<< START DATAPREP
>>> ACK
<<< WAIT FOR STATUSES
>>> FINISH DATAPREP
<<< ACK
>>> FINISH COOK
<<< ACK
>>> FINISH COPY
<<< ACK
>>> WAITING COMMANDS
<<< STOP SERVER
'''

############### server helper fonctions
def sendAck(socket):
    socket.sendall(cmd_ack.encode('utf-8'))
    
def waitForAck(socket):
    msg = socket.recv(1024).decode('utf-8')
    if not msg.startswith(cmd_ack):
        log_file.write('weird reply from client ' + msg + '\n')

############### data prep helper functions
# helper function to execute external command and progressively display results
def execute_and_print(args):
    process = subprocess.Popen(args, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
    for line in process.stdout:
        log_file.write(line.rstrip().decode('latin-1')+'\n')
    #process.wait()
    return process.returncode

def test_success(line):
    word_success = 'Success -'
    return line.find(word_success) >=0

def execute_and_print_with_check_success(args):
    log_file.write(' '.join(args)+'\n')
    process = subprocess.Popen(args, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
    success = False
    for line in process.stdout:
        if not success and test_success(line):
            success = True
        log_file.write(line.rstrip().decode('latin-1')+'\n')
    #process.wait()
    return success

    
#the actual data preparation
def do_data_preparation(socket, data):
    log_file.write(data+'\n')
    log_file.write('do_data_preparation\n')
    json_data = json.loads(data)
    #1 - we assume data is a "job" file that contains necessary information regarding preparation as a JSON format
    log_file.write('writing job to file\n')
    with open(job_file, 'w') as outfile:
        outfile.write(data)
    #2 - define output folders
    output_folder = dph.get_output_folder(json_data).replace('\\','/')
    project_name = dph.get_project_name(json_data).replace('\\','/')
    content_folder = dph.get_content_folder(json_data).split('/Game/')[1].replace('\\','/')
    editor_directory = '/'.join([template_project_folder,'content',content_folder])
    cooked_content_directory = template_project_folder+'/Saved/Cooked/WindowsNoEditor/Template/Content'
    cooked_directory = cooked_content_directory + '/' + content_folder
    output_content_directory = '/'.join([output_folder,project_name])
    output_editor_directory = '/'.join([output_content_directory,'editor/content'])
    output_cooked_directory = '/'.join([output_content_directory,'cooked/content'])
    #3 - perform data prep
    log_file.write('Starting data prep\n')
    msg = socket.recv(1024).decode('utf-8')
    if not msg.startswith(cmd_cli_waiting_for_statuses):
        log_file.write('weird reply from client ' + msg + '\n')
    dataprep_success = execute_and_print_with_check_success([ue4editor_cmd, template_project_path, '-run=pythonscript', '-script='+data_prep_script+' '+job_file])
    if not dataprep_success:
        log_file.write('Failed data prep\n')
    log_file.write('Finish data prep\n')
    socket.sendall(cmd_ser_finish_dataprep.encode('utf-8'))
    waitForAck(socket)
    #4 - cook assets
    log_file.write('Cooking assets\n')
    #dataprep_success = False #todo remove
    if not dataprep_success:
        log_file.write('Skipping due to failed data prep\n')
    else:
        # remove existing cooked content if any
        if os.path.exists(cooked_directory):
            shutil.rmtree(cooked_directory)
        cooking_success = execute_and_print_with_check_success([ue4editor_cmd, template_project_path, '-run=cook', '-targetplatform=WindowsNoEditor', '-cookall', '-skipeditorcontent'])
        if not cooking_success:
            log_file.write('Failed cooking assets\n')
        else: 
            log_file.write('Finish cooking assets\n')
    socket.sendall(cmd_ser_finish_cook.encode('utf-8'))
    waitForAck(socket)
    #return
    #5 - moving asset to data folder
    log_file.write('Moving assets to destination folders\n')
    if not dataprep_success or not cooking_success:
        log_file.write('Skipping due to fails in dataprep or cooking\n')
    else:
        if not os.path.exists(output_content_directory):
            os.makedirs(output_content_directory)
        #try to delete
        #6 - packing assets
        # generate the pak file for some reason to preserver hierarchy do it on the cooked output folder
        log_file.write("pak " + cooked_content_directory + " inside " + output_content_directory+'/'+project_name+'.pak\n')
        packing_success = execute_and_print_with_check_success([ue4pak, output_content_directory+'/'+project_name+'.pak', '-Create='+cooked_content_directory, '-compress'])
        if not packing_success:
            log_file.write('Failed packing assets\n')
        else: 
            log_file.write('Finish packing assets\n')
        # editor files
        if os.path.exists(output_editor_directory):
            try:
                shutil.rmtree(output_editor_directory)
            except OSError as e:
                log_file.write('Error: ' + str(e.filename) + ' - ' + str(e.strerror) + '\n')
        if not os.path.exists(output_editor_directory):
            os.makedirs(output_editor_directory)
        log_file.write(editor_directory+'\n')
        log_file.write(output_editor_directory+'\n')
        shutil.move(editor_directory, output_editor_directory)
        # cooked files
        if os.path.exists(output_cooked_directory):
            shutil.rmtree(output_cooked_directory)
        os.makedirs(output_cooked_directory)
        shutil.move(cooked_directory, output_cooked_directory)
        log_file.write('Finish moving assets\n')
    socket.sendall(cmd_ser_finish_copy.encode('utf-8'))
    waitForAck(socket)
    socket.sendall(cmd_ser_waiting_commands.encode('utf-8'))
    
#update the sys path for the dataprep script to be able to do imports from local folder
def update_sys_path_for_dataprep(dataprep_template_filepath, dataprep_filepath):
    with open(dataprep_template_filepath, 'r') as file :
        filedata = file.read()
    local_folder = '/'.join(dataprep_filepath.replace('\\','/').split('/')[:-1])
    # Replace the target string
    filedata = filedata.replace('import sys', 'import sys\nsys.path=sys.path + [\''+local_folder+'\']')
    # Write the file out again
    with open(dataprep_filepath, 'w') as file:
      file.write(filedata)

############### Main starts here ###############
    
#check if there is some line arguments: port
port = 10000
if len(sys.argv) == 2:
    port = int(sys.argv[1])

# log file
log_file = open('server.log','a')

#retrieve server folder
server_folder = os.path.dirname(os.path.realpath(__file__)).replace('\\','/')
log_file.write("python version " + sys.version + "\n")
log_file.write("server folder " + server_folder + "\n")
#retrieve the uproject from the template folder
template_project_zip = '/'.join([server_folder,'Template.zip'])
template_project_folder = '/'.join([server_folder,'Template'])
template_project_path = '/'.join([template_project_folder,'Template.uproject'])
#retrieve the data prep script from the local folder
data_prep_template_script = '/'.join([server_folder,'dataprep_template.py'])
data_prep_script = '/'.join([server_folder,'dataprep.py'])
update_sys_path_for_dataprep(data_prep_template_script,data_prep_script)
#set the UEditor commande
ue4editor_cmd = 'C:/Program Files/Epic Games/UE_4.25/Engine/Binaries/Win64/UE4Editor-Cmd.exe'
ue4pak = 'D:/UE/UE_4.25/Engine/Binaries/Win64/UnrealPak.exe'
#define job file
job_file = '/'.join([server_folder,'current_job.json'])

#clean the template directory
if os.path.exists(template_project_folder):
    log_file.write('deleting existing template folder ' + template_project_folder +'\n')
    shutil.rmtree(template_project_folder)
log_file.write('unziping archive ' + template_project_zip + ' to folder ' + server_folder +'\n')
zip_ref = zipfile.ZipFile(template_project_zip, 'r')
zip_ref.extractall(server_folder)
zip_ref.close()
    
# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = ('localhost', port)
log_file.write('starting up on %s port %s \n' % (server_address, port))
print "starting up on " + str(server_address) + " port " + str(port)
sock.bind(server_address)
sock.listen(1)

#list of commands that the server can receive
cmd_cli_config_dataprep = 'CONFIG DATAPREP'
cmd_cli_start_dataprep = 'START DATAPREP'
cmd_cli_waiting_for_statuses = 'WAIT FOR STATUSES'
cmd_cli_stop_server = 'STOP SERVER'
cmd_ser_finish_dataprep = 'FINISH DATAPREP'
cmd_ser_finish_cook = 'FINISH COOK'
cmd_ser_finish_copy = 'FINISH COPY'
cmd_ser_waiting_commands = 'WAITING COMMANDS'
cmd_ack = 'ACK'

cmd_cli_start_assets_to_pak = "COMPRESS ASSETS TO PAK" 

#one of the client is stopping the server
b_stop_server = False
#default value
config_import = ''

while not b_stop_server:
    # Wait for a connection
    log_file.write('waiting for a connection \n')
    connection, client_address = sock.accept()
    buffer_size = 1024
    try:
        log_file.write('connection from ' + str(client_address)+'\n')
        # Receive the data in small chunks
        while not b_stop_server:
            data = connection.recv(buffer_size).decode('utf-8')
            if data:
                log_file.write('received "%s"\n' % data)
                if data.startswith(cmd_cli_config_dataprep):
                    config_import = data.split(cmd_cli_config_dataprep)[1]
                    while len(data) == buffer_size:
                        data = connection.recv(buffer_size).decode('utf-8')
                        config_import += data
                    log_file.write('send Ack config\n')
                    sendAck(connection)
                elif data.startswith(cmd_cli_start_dataprep):
                    log_file.write('send Ack start\n')
                    sendAck(connection)
                    do_data_preparation(connection, config_import)
                elif data.startswith(cmd_cli_stop_server):
                    log_file.write('send Ack stop\n')
                    sendAck(connection)
                    b_stop_server = True
                else:
                    log_file.write('received unknown payload\n')
            else:
                break
    finally:
        # Clean up the connection
        connection.close()
log_file.close()
