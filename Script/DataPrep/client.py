import socket
import sys
import json

'''
>>> CONFIG DATAPREP
<<< ACK
>>> CONTENT
<<< ACK
>>> START DATAPREP
<<< ACK
>>> WAIT FOR STATUSES
<<< FINISH DATAPREP
>>> ACK
<<< FINISH COOK
>>> ACK
<<< FINISH COPY
>>> ACK
<<< WAITING COMMANDS
>>> STOP SERVER
'''

############### server helper fonctions
def sendAck(socket):
	socket.sendall(cmd_ack.encode('utf-8'))
	
def waitForAck(socket):
	msg = socket.recv(1024).decode('utf-8')
	if not msg.startswith(cmd_ack):
		print 'weird reply from server'

def track_preparation(socket):
	message = cmd_cli_waiting_for_statuses
	print >> sys.stderr, 'sending "%s"' % message
	sock.sendall(message.encode('utf-8'))
	msg = socket.recv(1024).decode('utf-8')
	if not msg.startswith(cmd_ser_finish_dataprep):
		print 'weird reply from server'
	else:
		sendAck(socket)
	print('data prep finished')
	msg = socket.recv(1024).decode('utf-8')
	if not msg.startswith(cmd_ser_finish_cook):
		print 'weird reply from server'
	else:
		sendAck(socket)
	print('cook finished')
	msg = socket.recv(1024).decode('utf-8')
	if not msg.startswith(cmd_ser_finish_copy):
		print 'weird reply from server'
	else:
		sendAck(socket)
	print('move finished')
	msg = socket.recv(1024).decode('utf-8')
	if not msg.startswith(cmd_ser_waiting_commands):
		print 'weird reply from server'

############### Main starts here ###############

cmd_cli_config_dataprep = 'CONFIG DATAPREP'
cmd_cli_start_dataprep = 'START DATAPREP'
cmd_cli_waiting_for_statuses = 'WAIT FOR STATUSES'
cmd_cli_stop_server = 'STOP SERVER'
cmd_ser_finish_dataprep = 'FINISH DATAPREP'
cmd_ser_finish_cook = 'FINISH COOK'
cmd_ser_finish_copy = 'FINISH COPY'
cmd_ser_waiting_commands = 'WAITING COMMANDS'
cmd_ack = 'ACK'

# create job
data = {}
data['Project Name'] = 'Test_Project'
data['Output Folder'] = 'D:/temp'
#data['CAD Files Path'] = ['D:/projects/HotLoad/DataPrep/03_Front rim.SLDPRT','D:/projects/HotLoad/DataPrep/DassaultAviationTest.3DXML']
data['CAD Files Path'] = ['D:/projects/data/01_Left Turbo part 1.SLDPRT']
data['Content Folder'] = '/Game/Temp_Import'
data['Tesselation Parameters'] = {'Chord Tolerance':'1.0','Edge Length':'100','Normal Tolerance':'40'}
#data['Template Map Name'] = '/Game/Default'
data['Template Map Name'] = '/Game/TimeOfDay'

'''
data['CAD Files Path'] =['D:/projects/McLaren/P1516/Exterior/16W0034CG CAS DATA SR4/16WA282CG_FRONT_ZONE/16WA340CG_FENDER-SR4/16WA340CG01_03A_WIP_FENDER_SR4_160318.CATPart',
'D:/projects/McLaren/P1516/Exterior/16W0034CG CAS DATA SR4/16WA282CG_FRONT_ZONE/16WA341CG_BONNET- SR4/16WA341CG01_03A_BONNET_16032018.CATPart',
'D:/projects/McLaren/P1516/Exterior/16W0034CG CAS DATA SR4/16WA282CG_FRONT_ZONE/16WA342CG_HEADLAMP-SR4/16WA342CG01_01A_HEADLAMP_09032018.CATPart',
'D:/projects/McLaren/P1516/Exterior/16W0034CG CAS DATA SR4/16WA282CG_FRONT_ZONE/16WA343CG_BUMPER-FRNT-SR4/16WA343CG01_01A_BUMPER-FRNT-SR4_160318.CATPart',
'D:/projects/McLaren/P1516/Exterior/16W0034CG CAS DATA SR4/16WA282CG_FRONT_ZONE/16WA345CG_SIGNAL LAMP-SR4/16WA345CG01_01A_SIGNAL_LAMP-SR4_230218.CATPart']
'''
''',
'D:/projects/McLaren/P1516/Exterior/16W0034CG CAS DATA SR4/16WA282CG_FRONT_ZONE/16WA346CG_COVER-WIPER-LHD-SR4/16WA346CG01_01A_COVER-WIPER-LHD-08032018.CATPart',
'D:/projects/McLaren/P1516/Exterior/16W0034CG CAS DATA SR4/16WA282CG_FRONT_ZONE/16WA353CG_BADGE-FRNT-SR4/16WA353CG01_01A_BADGE_FRNT_16032018.CATPart',
'D:/projects/McLaren/P1516/Exterior/16W0034CG CAS DATA SR4/16WA282CG_FRONT_ZONE/16WA357CG_PLINTH-LICENCE PLATE-FRONT-EU-SR4/16WA357CG01_01A_PLINTH-LICENCE PLATE-FRONT-EU_230218.CATPart']'''

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Connect the socket to the port where the server is listening
server_address = ('localhost', 9999)
print >> sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)
try:
	# Send data
	message = cmd_cli_config_dataprep + json.dumps(data)
	print >> sys.stderr, 'sending "%s"' % message
	sock.sendall(message.encode('utf-8'))
	waitForAck(sock)
	message = cmd_cli_start_dataprep
	print >> sys.stderr, 'sending "%s"' % message
	sock.sendall(message.encode('utf-8'))
	waitForAck(sock)
	track_preparation(sock)
	message = cmd_cli_stop_server
	print >> sys.stderr, 'sending "%s"' % message
	sock.sendall(message.encode('utf-8'))
	waitForAck(sock)

finally:
	print >>sys.stderr, 'closing socket'
	sock.close()