import uuid
class User:
    def __init__(self, name, public_key):
        self.id = uuid.uuid4()
        self.name = name
        self.public_key = public_key
        self.unread_messages = []
    
    def __str__(self) -> str:
        return f"User - id={self.id}, name={self.name}, public_key={self.public_key}"
    