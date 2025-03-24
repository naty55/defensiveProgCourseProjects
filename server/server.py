import utils
import config
import socket 
import sys
import logging
from threading import Thread
from protocol import Codes, Sizes, Request, Response, Message
from user import User

logger = logging.getLogger("server")
logger.setLevel(logging.INFO)
logger.addHandler(logging.StreamHandler(sys.stdout))

class Server:

    BUFFER_SIZE = 4096

    def __init__(self):
        self.host = config.DEFAULT_SERVER_HOST
        self.port = utils.get_server_port()
        self.clients: dict[str: User] = dict()
        self.socket = None
        self.handlers = {
            Codes.REGISTER_REQUEST_CODE: self.handle_register,
            Codes.LIST_CLIENT_REQUEST_CODE: self.handle_list_clients,
            Codes.GET_PUBLIC_KEY_REQUEST_CODE: self.handle_public_key_request,
            Codes.SEND_MESSEGE_REQUEST_CODE: self.handle_messege,
            Codes.GET_MESSEGES_REQUEST_CODE: self.handle_pending_messeges
        }
    
    def start(self):
        logger.info(f"Listening on {self.host}:{self.port}")
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((self.host, self.port))
            s.listen()
            while True:
                conn, addr = s.accept()
                Thread(target=self.handle_connection, args=(conn, addr)).start()
    
    def handle_connection(self, conn, addr):
        logger.debug(f"Connected by {addr}")
        with conn:
            request_bytes = b''
            data = conn.recv(Server.BUFFER_SIZE)
            request_bytes += data
            _, _, _, payload_size = Request.read_header(request_bytes)
            logger.debug(f"Reading payload size: {payload_size}")
            while data and len(request_bytes) < Request.header_size() + payload_size:
                logger.debug(f"Read: {len(request_bytes)}")
                data = conn.recv(Server.BUFFER_SIZE)
                request_bytes += data
            try:
                logger.info(f"Processing request, len(request-bytes)={len(request_bytes)}")
                request = Request.from_bytes(request_bytes)
                self.handle_request(request, conn)
            except Exception as e:
                logger.error(f"Got an error {e}")
                conn.sendall(Response.error_response(config.SERVER_VERSION).to_bytes())
            
    
    def handle_request(self, request: Request, conn):
        logger.info(f"Handling request {request}")
        response = None
        try:
            handler = self.handlers.get(request.req_code, self.handle_error)
            response = handler(request)
        except Exception as e:
            logger.error(e)
            response = self.handle_error()
        logger.info(F"Sending Response {response}")
        res_bytes = response.to_bytes()
        logger.debug(f"Response bytes: {res_bytes if len(res_bytes) < 4096 else res_bytes[:52] + b'...' }")
        conn.sendall(res_bytes)
    
    def handle_register(self, request: Request):
        payload = request.payload
        name = payload[:Sizes.CLIENT_NAME_SIZE]
        public_key = payload[Sizes.CLIENT_NAME_SIZE:]
        user = User(name, public_key)
        if user.name in [client.name for client in self.clients.values()]:
            raise Exception(f"User with username {user.name} already exists")
        self.clients[user.id.bytes] = user
        return Response(config.SERVER_VERSION, Codes.REGISTER_RESPONSE_CODE, len(user.id.bytes), user.id.bytes)
    
    def handle_list_clients(self, request: Request):
        client_id = request.cid
        client_list = b''.join([_id + self.clients[_id].name for _id in self.clients.keys() if _id != client_id])
        return Response(config.SERVER_VERSION, Codes.LIST_CLIENT_RESPONSE_CODE, len(client_list), client_list)
    
    def handle_public_key_request(self, request: Request):
        client_id = request.payload
        public_key = self.clients[client_id].public_key
        response_payload = client_id + public_key
        return Response(config.SERVER_VERSION, Codes.GET_PUBLIC_KEY_RESPONSE_CODE, len(response_payload), response_payload)
    
    def handle_messege(self, request: Request):
        client_id = request.cid
        target_client_id = request.payload[:Sizes.CLIENT_ID_SIZE]
        message = Message(client_id, request.payload[Sizes.CLIENT_ID_SIZE:])
        self.clients[target_client_id].unread_messages.append(message)
        print("Updated user: ", self.clients[target_client_id])
        payload = target_client_id + message.message_id
        return Response(config.SERVER_VERSION, Codes.SEND_MESSEGE_RESPONSE_CODE, len(payload), payload)
    
    def handle_pending_messeges(self, request: Request):
        client_id = request.cid
        messages = b''.join([message.to_bytes() for message in self.clients[client_id].unread_messages])
        response = Response(config.SERVER_VERSION, Codes.GET_MESSEGES_RESPONSE_CODE, len(messages), messages)
        self.clients[client_id].unread_messages.clear()
        return response
    
    def handle_error(self, request: Request = None):
        logger.info("Returning error response")
        return Response.error_response(config.SERVER_VERSION)

