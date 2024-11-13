import struct 
from logs import logger
class Request:
    header_format = "<16sBHI"
    request_header_length = 23 # bytes

    def __init__(self, client_id, version, code, payload_size, payload):
        self.cid = client_id
        self.client_version = version
        self.req_code = code
        self.payload_size = payload_size
        self.payload = payload
    
    def __str__(self):
        return f"[Request] client_id={self.cid}, client_version={self.client_version}, code={self.req_code}, payload_size={self.payload_size}, payload={self.payload}"

    @classmethod
    def from_bytes(cls, req_bytes):
        header_length = cls.header_size()
        if len(req_bytes) < header_length:
            raise Exception("Invalid request")
        client_id, client_version, req_code, payload_size = struct.unpack(cls.header_format, req_bytes[0:23])
        received_payload_size = len(req_bytes) - header_length
        if received_payload_size != payload_size:
            logger.warn(f"payload size={payload_size}, actual payload size = {received_payload_size}")
            raise Exception("Invalid request")
        payload = None
        if payload_size > 0:
            payload = req_bytes[header_length:]
        return cls(client_id, client_version, req_code, payload_size, payload)
    
    @classmethod
    def header_size(cls):
        return struct.calcsize(cls.header_format)
    


class Response:
    header_format = "<BHI"
    def __init__(self, server_version, response_code, payload_size, payload):
        self.server_version = server_version
        self.code = response_code
        self.payload_size = payload_size
        self.payload = payload
    def to_bytes(self):
        return struct.pack(self.header_format, self.server_version, self.code, self.payload_size) + (self.payload if self.payload != None else b'')
    
    def __str__(self) -> str:
        return f"Response - version={self.server_version}, response_code={self.code}, payload_size={self.payload_size}, payload={self.payload}"