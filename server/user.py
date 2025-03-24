import uuid
from protocol import Sizes
class User:
    def __init__(self, name: bytes, public_key: bytes):
        diff_len: int = Sizes.CLIENT_NAME_SIZE - len(name)
        self.id = uuid.uuid4()
        self.name: bytes = name + b'\0' * diff_len
        self.public_key:bytes = public_key
        self.unread_messages = []
    
    def __str__(self) -> str:
        return f"User - id={self.id}, name={self.name}, public_key={self.public_key}, unread_messages={self.unread_messages}"
    
    def __repr__(self) -> str:
        return self.__str__()