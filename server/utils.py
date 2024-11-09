from config import *
import os 

def get_server_port():
    if os.path.isfile(PORT_FILE_NAME):
        with open(PORT_FILE_NAME, 'r') as port_file:
            try:
                return int(port_file.read())
            except ValueError:
                return DEFAULT_SERVER_PORT
    else:
        return DEFAULT_SERVER_PORT