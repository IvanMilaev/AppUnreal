import socket
import sys
import os

log_file = open('server.log','a')
server_folder = os.path.dirname(os.path.realpath(__file__)).replace('\\','/')

log_file.write("python version " + sys.version + "\n")
log_file.write("server folder " + server_folder + "\n")

port = 10000
if len(sys.argv) == 2:
    port = int(sys.argv[1])

server_address = ('localhost', port)

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

log_file.write('starting up on %s port %s \n' % (server_address, port))
print "starting up on " + str(server_address) + " port " + str(port)

sock.bind(server_address)
sock.listen(1)

#one of the client is stopping the server
b_stop_server = False


#list of commands that the server can receive
cmd_cli_config_dataprep = 'CONFIG DATAPREP'
cmd_cli_start_dataprep = 'START DATAPREP'
cmd_cli_start_assetprep = 'START ASSETPREP'
cmd_cli_waiting_for_statuses = 'WAIT FOR STATUSES'
cmd_cli_stop_server = 'STOP SERVER'
cmd_ser_finish_dataprep = 'FINISH DATAPREP'
cmd_ser_finish_cook = 'FINISH COOK'
cmd_ser_finish_copy = 'FINISH COPY'
cmd_ser_waiting_commands = 'WAITING COMMANDS'
cmd_ack = 'ACK'


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
                if data.startswith(cmd_cli_start_assetprep):
                    log_file.write('start asset preparation\n')
                elif data.startswith(cmd_cli_stop_server):
                    log_file.write('stop server \n')
                    b_stop_server = True
                else:
                    log_file.write('received unknown payload\n')
            else:
                break
    finally:
        # Clean up the connection
        connection.close()
log_file.close()
