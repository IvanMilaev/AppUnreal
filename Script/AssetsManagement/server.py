import socket
import sys
import os
import json
import shutil
import zipfile
import subprocess

config= open('config.json').read()
cfg=json.loads(config)
job_description = {}
print cfg

log_file = open('server.log','a')
server_folder = os.path.dirname(os.path.realpath(__file__)).replace('\\','/')

log_file.write("python version " + sys.version + "\n")
log_file.write("server folder " + server_folder + "\n")

template_project_zip = '/'.join([server_folder,'Template.zip'])
template_project_folder = '/'.join([server_folder,'Template'])
template_project_path = '/'.join([template_project_folder,'Template.uproject'])

#clean the template directory
if os.path.exists(template_project_folder):
    log_file.write('deleting existing template folder ' + template_project_folder +'\n')
    shutil.rmtree(template_project_folder)
log_file.write('unziping archive ' + template_project_zip + ' to folder ' + server_folder +'\n')
zip_ref = zipfile.ZipFile(template_project_zip, 'r')
zip_ref.extractall(server_folder)
zip_ref.close()

asset_library_directory = "d:/ue/AssetLibrary"
output_folder = "d:/temp"
project_name = "AutomatedImport"
content_folder = "/Game/Hotloadimport"
editor_directory = '/'.join([template_project_folder,'content',content_folder])
cooked_content_directory = template_project_folder+'/Saved/Cooked/WindowsNoEditor/Template/Content'
cooked_directory = cooked_content_directory + '/' + content_folder
output_content_directory = '/'.join([output_folder,project_name])
output_editor_directory = '/'.join([output_content_directory,'editor/content'])
output_cooked_directory = '/'.join([output_content_directory,'cooked/content'])


def updateJobDescription():
    asset_library_directory = job_description['assetLibrary']
    output_folder = job_description['preparedPakFolder']
    project_name = job_description['assetID']
    content_folder = job_description['contentFolderInGame']

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

    #  "Output Folder": "d:/temp",
    # "Content Folder": "/Game/Import",
    #"Project Name": "AutomatedImport"

def import_asset(asset_name):
    shutil.copytree(asset_library_directory+'/'+asset_name, editor_directory)
    

def do_job(socket):
    import_asset(job_description['assetID'])
    asset_library_directory = job_description['assetLibrary']
    output_folder = job_description['preparedPakFolder']
    project_name = job_description['assetID']
    content_folder = job_description['contentFolderInGame']

    log_file.write('Cooking assets\n')
     # remove existing cooked content if any
    if os.path.exists(cooked_directory):
        shutil.rmtree(cooked_directory)
    cooking_success = execute_and_print_with_check_success([cfg['ue4editor_cmd'], template_project_path, '-run=cook', '-targetplatform=WindowsNoEditor', '-cookall', '-skipeditorcontent'])
    if not cooking_success:
        log_file.write('Failed cooking assets\n')
    else:
        log_file.write('Finish cooking assets\n')

    log_file.write('Moving assets to destination folders\n')

    if not cooking_success:
        log_file.write('Skipping due to fails in cooking\n')
    else:
        if not os.path.exists(output_content_directory):
            os.makedirs(output_content_directory)

        log_file.write("pak " + cooked_content_directory + " inside " + output_content_directory+'/'+project_name+'.pak\n')
        packing_success = execute_and_print_with_check_success([cfg['ue4pak'], output_content_directory+'/'+project_name+'.pak', '-Create='+cooked_content_directory, '-compress'])
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


port = 10000
if len(sys.argv) == 2:
    port = int(sys.argv[1])

server_address = ('localhost', port)
log_file.write('try starting up on %s port %s \n' % (server_address, port))

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

log_file.write('starting up on %s port %s \n' % (server_address, port))
print "starting up on " + str(server_address) + " port " + str(port)

sock.bind(server_address)
sock.listen(1)

#one of the client is stopping the server
b_stop_server = False


#list of commands that the server can receive
cmd_echo = 'ECHO'
cmd_confirmed = 'CONFIRMED'
cmd_kill_server = 'KILL_SERVER'
cmd_waiting_for_statuses = 'WAITING_FOR_STATUSES'
cmd_send_job_to_server = 'SEND_JOB_TO_SERVER'
cmd_start_job = 'START_JOB',
cmd_job_completed = 'JOB_COMPLETED'
cmd_test = "TEST_COMMAND"



while not b_stop_server:
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
                print >>sys.stderr, 'received "%s"' % data

                if data.startswith(cmd_echo):
                    print >>sys.stderr, 'received echo"%s"' % data
                    connection.sendall(data)
                    log_file.write('received echo\n')
                    connection.sendall(cmd_echo)

                elif data.startswith(cmd_test):
                    print >>sys.stderr, 'received echo"%s"' % data
                    import_asset('test_asset')
                    connection.sendall(cmd_test)

                elif data.startswith(cmd_send_job_to_server):
                    print >>sys.stderr, 'received new job"%s"' % data
                    log_file.write(data)
                    job_description = json.loads(data.split('|')[1])
                    print >>sys.stderr, job_description['assetID']
                    updateJobDescription()
                    log_file.write('received new job \n')
                    connection.sendall(cmd_confirmed)
                    do_job(connection)
                    connection.sendall(cmd_job_completed)

                elif data.startswith(cmd_start_job):
                    print >>sys.stderr, 'Starting job..."%s"' % data
                    log_file.write('Starting job \n')
                    connection.sendall(cmd_confirmed)
                    do_job(connection)
                    connection.sendall(cmd_job_completed)


                elif data.startswith(cmd_kill_server):
                    print >>sys.stderr, 'killing server..."%s"' % data
                    log_file.write('stop server \n')
                    connection.sendall(cmd_kill_server)
                    b_stop_server = True

                else:
                    log_file.write('received unknown payload\n')
            else:
                break
    finally:
        # Clean up the connection
        connection.close()
log_file.close()
