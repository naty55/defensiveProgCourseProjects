from utils import get_server_port
from logs import Logger
from user import User
from protocol import Request, Response, Message
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
            handle_connection(conn, addr)
                            

def handle_connection(conn, addr):
    with conn:
        logger.debug(f"Connected by {addr}")
        data = conn.recv(1024)
        if len(data) >= Request.header_size():
            try:
                logger.debug(f"Request bytes={data}")
                request = Request.from_bytes(data)
                handle_request(request, conn)
            except Exception as e:
                logger.error(f"Got an error {e}")
                conn.sendall(Response.error_response(SERVER_VERSION).to_bytes())
        else:
            conn.sendall(Response.error_response(SERVER_VERSION).to_bytes())

def handle_request(request, conn):
    print("---Clients---", clients)
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
            client_id = request.payload
            public_key = clients[client_id].public_key
            response_payload = client_id + public_key
            response = Response(SERVER_VERSION, Codes.GET_PUBLIC_KEY_RESPONSE_CODE, len(response_payload), response_payload)
        
        case Codes.SEND_MESSEGE_REQUEST_CODE:
            client_id = request.cid
            target_client_id = request.payload[0:16]
            message = Message(client_id, request.payload[16:])
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
    print("Response bytes: ", res_bytes)
    conn.sendall(res_bytes)
        
    


if __name__ == "__main__":
    main()