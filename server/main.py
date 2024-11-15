from utils import get_server_port
from logs import Logger
from user import User
from protocol import Request, Response
from config import DEFAULT_SERVER_HOST, SERVER_VERSION
import socket 
from protocol import Codes

logger = Logger()


server_port = get_server_port()
logger.info("Starting server on " + str(server_port))

clients = dict()

def register():
    pass

def main():
    logger.info(f"Listening on {DEFAULT_SERVER_HOST}:{server_port}")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((DEFAULT_SERVER_HOST, server_port))
        s.listen()
        while True:
            conn, addr = s.accept()
            with conn:
                logger.debug(f"Connected by {addr}")
                data = conn.recv(1024)
                if len(data) >= Request.header_size():
                    try:
                        request = Request.from_bytes(data)
                        handle_request(request, conn)
                    except Exception as e:
                        logger.error(f"Got an error {e}")
                        conn.sendall(Response(1, 9000, 0, None).to_bytes())
                else:
                    conn.sendall(Response(1, 9000, 0, None).to_bytes())
                            
                        

def handle_request(request, conn):
    logger.info(f"Handling request {request}")
    code = request.req_code
    response = None
    match code:
        case Codes.REGISTER_REQUEST_CODE:
            payload = request.payload
            name = payload[:255]
            public_key = payload[255:]
            user = User(name, public_key)
            clients[user.id.bytes] = user
            response = Response(SERVER_VERSION, Codes.REGISTER_RESPONSE_CODE, len(user.id.bytes), user.id.bytes)
        
        case Codes.LIST_CLIENT_REQUEST_CODE:
            client_list = b''.join([id + clients[id].name for id in clients.keys()])
            response = Response(SERVER_VERSION, Codes.LIST_CLIENT_RESPONSE_CODE, len(client_list), client_list)
        
        case Codes.GET_PUBLIC_KEY_REQUEST_CODE:
            response = Response(SERVER_VERSION, Codes.GET_PUBLIC_KEY_RESPONSE_CODE, 0, None)
        
        case _:
            response = Response(1, 9000, 0, None).to_bytes()
    
    logger.info(F"Sending Response {response}")
    conn.sendall(response.to_bytes())
        
    


if __name__ == "__main__":
    main()