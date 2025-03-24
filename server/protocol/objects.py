import struct 
from logs import logger
import random
from dataclasses import dataclass
from .protocol import Sizes

@dataclass
class Request:
    cid: bytes
    client_version: int
    req_code: int
    payload_size: int
    payload: bytes

    #### STATIC VARIABLES ####
    header_format = f"<{Sizes.CLIENT_ID_SIZE}sBHI" 
    
    def __str__(self):
        return f"[Request] client_id={self.cid}, client_version={self.client_version}, code={self.req_code}, payload_size={self.payload_size}, payload={self.payload if self.payload_size < 52 else self.payload[:52] + b'...'}"

    @staticmethod
    def from_bytes(req_bytes):
        client_id, client_version, req_code, payload_size = Request.read_header(req_bytes)
        received_payload_size = len(req_bytes) - Request.header_size()
        if received_payload_size != payload_size:
            logger.warn(f"payload size={payload_size}, actual payload size = {received_payload_size}")
            raise Exception("Invalid request")
        payload = None
        if payload_size > 0:
            payload = req_bytes[Request.header_size():]
        return Request(client_id, client_version, req_code, payload_size, payload)
    
    @staticmethod
    def header_size():
        return struct.calcsize(Request.header_format)
    
    @staticmethod
    def read_header(req: bytes):
        header_length = Request.header_size()
        if len(req) < header_length:
            raise Exception("Header is too short")
        return struct.unpack(Request.header_format, req[:Request.header_size()])
    

@dataclass
class Response:
    server_version: int
    code: int
    payload_size: int
    payload: bytes

    #### STATIC VARIABLES ####
    header_format = "<BHI"
    
    def to_bytes(self):
        return struct.pack(self.header_format, self.server_version, self.code, self.payload_size) + (self.payload if self.payload != None else b'')
    
    def __str__(self) -> str:
        return f"Response - version={self.server_version}, response_code={self.code}, payload_size={self.payload_size}, payload={self.payload}"
    
    def error_response(server_version):
        return Response(server_version, 9000, 0, None)
    

class Message:
    message_header_format = "<16s4sBI"
    def __init__(self, client_id, message_bytes) -> None:
        self.from_client_id = client_id
        self.message_id = Message.generate_msg_id()
        self.message_type = message_bytes[0]
        self.message_size = struct.unpack("<I", message_bytes[1:5])[0] 
        self.messsage_content = message_bytes[5:]
        if len(self.messsage_content) != self.message_size:
            raise Exception("Invalid message")
    
    def to_bytes(self):
        return struct.pack(Message.message_header_format, self.from_client_id, self.message_id, self.message_type, self.message_size) + self.messsage_content
    def __str__(self):
        return f"Message - from={self.from_client_id}, messae_id={self.message_id}"
    
    def __repr__(self) -> str:
        return self.__str__()
    
    @staticmethod
    def generate_msg_id():
        return random.randbytes(Sizes.MESSAGE_ID_SIZE)
    
    @staticmethod
    def convert_message_from_db_to_bytes(from_client_id, message_id, message_type, message_size, message_content):
        return struct.pack(Message.message_header_format, from_client_id, message_id, message_type, message_size) + message_content
