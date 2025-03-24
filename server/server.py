import utils
import config
import socket 
import sys
import logging
from threading import Thread
from protocol import Codes, Sizes, Request, Response, Message
from user import User
from database import Database

logger = logging.getLogger("server")
logger.setLevel(logging.INFO)
logger.addHandler(logging.StreamHandler(sys.stdout))

class Server:

    BUFFER_SIZE = 4096

    def __init__(self):
        self.host = config.DEFAULT_SERVER_HOST
        self.port = utils.get_server_port()
        self.db : Database = Database()
        self.socket = None
        self.handlers = {
            Codes.REGISTER_REQUEST_CODE: self.handle_register,
            Codes.LIST_CLIENT_REQUEST_CODE: self.handle_list_clients,
            Codes.GET_PUBLIC_KEY_REQUEST_CODE: self.handle_public_key_request,
            Codes.SEND_MESSEGE_REQUEST_CODE: self.handle_messege,
            Codes.GET_MESSEGES_REQUEST_CODE: self.handle_pending_messeges
        }
    
    def start(self):
        self.db.connect_and_init()
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
            logger.error(f"Exception: {e}")
            response = self.handle_error()
        logger.info(F"Sending Response {response}")
        res_bytes = response.to_bytes()
        logger.debug(f"Response bytes: {res_bytes if len(res_bytes) < 4096 else res_bytes[:52] + b'...' }")
        conn.sendall(res_bytes)
    
    def handle_register(self, request: Request):
        Server.validate_payload_size(Sizes.PUBLIC_KEY_SIZE + Sizes.CLIENT_NAME_SIZE, request.payload_size)
        payload = request.payload
        name = payload[:Sizes.CLIENT_NAME_SIZE]
        public_key = payload[Sizes.CLIENT_NAME_SIZE:]
        user = User(name, public_key)
        if self.db.username_exists(name):
            raise Exception(f"User with username {user.name} already exists")
        self.db.add_client(user)
        print(self.db.retrieve_all_users())
        return Response(config.SERVER_VERSION, Codes.REGISTER_RESPONSE_CODE, len(user.id.bytes), user.id.bytes)
    
    def handle_list_clients(self, request: Request):
        Server.validate_payload_size(0, request.payload_size)
        client_id = request.cid
        client_list = b''.join([_id + name for _id, name in self.db.retrieve_all_users() if _id != client_id])
        return Response(config.SERVER_VERSION, Codes.LIST_CLIENT_RESPONSE_CODE, len(client_list), client_list)
    
    def handle_public_key_request(self, request: Request):
        Server.validate_payload_size(Sizes.CLIENT_ID_SIZE, request.payload_size)
        client_id = request.payload
        public_key_result = self.db.get_public_key_of_user(client_id)
        if not public_key_result:
            raise Exception(f"can't find public key for {client_id}")
        response_payload = client_id + public_key_result[0]
        return Response(config.SERVER_VERSION, Codes.GET_PUBLIC_KEY_RESPONSE_CODE, len(response_payload), response_payload)
    
    def handle_messege(self, request: Request):
        client_id = request.cid
        target_client_id = request.payload[:Sizes.CLIENT_ID_SIZE]
        message = Message(client_id, request.payload[Sizes.CLIENT_ID_SIZE:])
        self.db.save_message(message, target_client_id)
        payload = target_client_id + message.message_id
        return Response(config.SERVER_VERSION, Codes.SEND_MESSEGE_RESPONSE_CODE, len(payload), payload)
    
    def handle_pending_messeges(self, request: Request):
        Server.validate_payload_size(0, request.payload_size)
        client_id = request.cid
        messages = self.db.retrieve_and_delete_all_waiting_messages(client_id)
        messages = b''.join([Message.convert_message_from_db_to_bytes(from_client_id, message_id, message_type, len(message_content), message_content) for from_client_id, message_id, message_type, message_content in messages])
        response = Response(config.SERVER_VERSION, Codes.GET_MESSEGES_RESPONSE_CODE, len(messages), messages)
        return response
    
    def handle_error(self, request: Request = None):
        logger.info("Returning error response")
        return Response.error_response(config.SERVER_VERSION)
    
    @staticmethod
    def validate_payload_size(expected_size, payload_size):
        if payload_size != expected_size:
            raise Exception("Payload size is not expected")

