from utils import get_server_port
from logs import Logger
from user import User
from protocol import Request, Response, Message
from config import DEFAULT_SERVER_HOST, SERVER_VERSION
import socket 
from threading import Thread
from protocol import Codes, Sizes

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
            Thread(target=handle_connection, args=(conn, addr)).start()
                            

def handle_connection(conn, addr):
    with conn:
        logger.debug(f"Connected by {addr}")
        request_bytes = b''
        data = conn.recv(1024)
        request_bytes += data
        _, _, _, payload_size = Request.read_header(request_bytes)
        logger.debug(f"Reading payload size: {payload_size}")
        while data and len(request_bytes) < Request.header_size() + payload_size:
            logger.debug(f"Read: {len(request_bytes)}")
            data = conn.recv(1024)
            request_bytes += data
        if len(data) >= Request.header_size():
            try:
                logger.info(f"Processing request, len(request-bytes)={len(request_bytes)}")
                request = Request.from_bytes(request_bytes)
                handle_request(request, conn)
            except Exception as e:
                logger.error(f"Got an error {e}")
                conn.sendall(Response.error_response(SERVER_VERSION).to_bytes())
        else:
            conn.sendall(Response.error_response(SERVER_VERSION).to_bytes())

def handle_request(request: Request, conn):
    print("---Clients---", clients)
    logger.info(f"Handling request {request}")
    code = request.req_code
    response = None
    match code:
        case Codes.REGISTER_REQUEST_CODE:
            payload = request.payload
            name = payload[:Sizes.CLIENT_NAME_SIZE]
            public_key = payload[Sizes.CLIENT_NAME_SIZE:]
            user = User(name, public_key)
            if user.name in [client.name for client in clients.values()]:
                raise Exception(f"User with username {user.name} already exists")
            clients[user.id.bytes] = user
            response = Response(SERVER_VERSION, Codes.REGISTER_RESPONSE_CODE, len(user.id.bytes), user.id.bytes)
        
        case Codes.LIST_CLIENT_REQUEST_CODE:
            client_id = request.cid
            client_list = b''.join([_id + clients[_id].name for _id in clients.keys() if _id != client_id])
            response = Response(SERVER_VERSION, Codes.LIST_CLIENT_RESPONSE_CODE, len(client_list), client_list)
        
        case Codes.GET_PUBLIC_KEY_REQUEST_CODE:
            client_id = request.payload
            public_key = clients[client_id].public_key
            response_payload = client_id + public_key
            response = Response(SERVER_VERSION, Codes.GET_PUBLIC_KEY_RESPONSE_CODE, len(response_payload), response_payload)
        
        case Codes.SEND_MESSEGE_REQUEST_CODE:
            client_id = request.cid
            target_client_id = request.payload[:Sizes.CLIENT_ID_SIZE]
            message = Message(client_id, request.payload[Sizes.CLIENT_ID_SIZE:])
            clients[target_client_id].unread_messages.append(message)
            print("Updated user: ", clients[target_client_id])
            payload = target_client_id + message.message_id
            response = Response(SERVER_VERSION, Codes.SEND_MESSEGE_RESPONSE_CODE, len(payload), payload)
        
        case Codes.GET_MESSEGES_REQUEST_CODE:
            client_id = request.cid
            messages = b''.join([message.to_bytes() for message in clients[client_id].unread_messages])
            response = Response(SERVER_VERSION, Codes.GET_MESSEGES_RESPONSE_CODE, len(messages), messages)
            clients[client_id].unread_messages.clear()
        case _:
            response = Response.error_response(SERVER_VERSION)
    
    logger.info(F"Sending Response {response}")
    res_bytes = response.to_bytes()
    print("Response bytes: ", res_bytes if len(res_bytes) < 4096 else res_bytes[:52] + b'...')
    conn.sendall(res_bytes)
        
    


if __name__ == "__main__":
    main()