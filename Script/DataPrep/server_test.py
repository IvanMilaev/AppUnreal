import socket
import sys
import json
import os
import subprocess
import dataPrepHelper as dph
import shutil
import time

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
	socket.sendall(cmd_ack)
	
def waitForAck(socket):
	msg = socket.recv(1024)
	if not msg.startswith(cmd_ack):
		log_file.write('weird reply from client ' + msg + '\n')

#the actual data preparation
def do_data_preparation(socket, data):
	json_data = json.loads(data)
	log_file.write('do_data_preparation\n')
	#3 - perform data prep
	log_file.write('start data prep\n')
	msg = socket.recv(1024)
	if not msg.startswith(cmd_cli_waiting_for_statuses):
		log_file.write('weird reply from client ' + msg + '\n')
	time.sleep(2)
	log_file.write('Finish data prep\n')
	socket.sendall(cmd_ser_finish_dataprep)
	waitForAck(socket)
	#4 - cook assets
	log_file.write('cooking assets\n')
	# remove existing cooked content if any
	time.sleep(2)
	log_file.write('Finish cooking assets\n')
	socket.sendall(cmd_ser_finish_cook)
	waitForAck(socket)
	#5 - moving asset to data folder
	time.sleep(2)
	log_file.write('Finish moving assets\n')
	socket.sendall(cmd_ser_finish_copy)
	waitForAck(socket)
	socket.sendall(cmd_ser_waiting_commands)
	

############### Main starts here ###############
	
#check if there is some line arguments: port
port = 10000
if len(sys.argv) == 2:
	port = int(sys.argv[1])

# log file
log_file = open('server_test.log','a')
	
# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = ('localhost', port)
log_file.write('starting up on %s port %s \n' % (server_address, port))
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
			data = connection.recv(buffer_size)
			if data:
				log_file.write('received "%s"\n' % data)
				if data.startswith(cmd_cli_config_dataprep):
					config_import = data.split(cmd_cli_config_dataprep)[1]
					while len(data) == buffer_size:
						data = connection.recv(buffer_size)
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