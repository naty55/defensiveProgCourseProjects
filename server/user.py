import uuid

class User:
    def __init__(self, name, public_key):
        if len(name) > 255:
            raise Exception("Name is too long")
        diff_len = 255 - len(name)
        self.id = uuid.uuid4()
        self.name = name + b'\0' * diff_len
        self.public_key = public_key
        self.unread_messages = []
    
    def __str__(self) -> str:
        return f"User - id={self.id}, name={self.name}, public_key={self.public_key}, unread_messages={self.unread_messages}"
    
    def __repr__(self) -> str:
        return self.__str__()