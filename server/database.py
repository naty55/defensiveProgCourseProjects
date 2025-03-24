import sqlite3
import config
from protocol import Message
from user import User
from threading import Lock
import logging
import sys
logger = logging.getLogger("database")
logger.addHandler(logging.StreamHandler(sys.stdout))
logger.setLevel(logging.INFO)

class Database:
    def __init__(self):
        self.file = config.DB_FILE
        self.connection = None
        self.lock = Lock()
    
    def connect_and_init(self):
        logger.info("conneting to database")
        self.connection = sqlite3.connect(config.DB_FILE, check_same_thread=False)
        c = self.connection.cursor()
        with self.lock:
            c.execute(f"""CREATE TABLE IF NOT EXISTS clients(
                    id         TEXT PRIMARY KEY,
                    username   TEXT,
                    public_key TEXT,
                    last_seen  TIMESTAMP
                    );""")
            c.execute(f"""CREATE TABLE IF NOT EXISTS messages(
                    id             TEXT,
                    from_client_id TEXT, 
                    to_client_id   TEXT, 
                    type           INTEGER,
                    content        TEXT
                    );""")
            self.connection.commit()
            logger.info("done conneting and initializing to database")


    def add_client(self, user: User):
        c = self.connection.cursor()
        with self.lock:
            c.execute("INSERT INTO clients (id, username, public_key, last_seen) VALUES (?, ?, ?, datetime('now'))", (user.id.bytes, user.name, user.public_key))
            self.connection.commit()

    def get_client(self, client_id: bytes):
        c = self.connection.cursor()
        with self.lock:
            c.execute("SELECT * FROM clients WHERE id=?", (client_id,))
        return c.fetchone()

    def save_message(self, message: Message, target_client_id: bytes):
        c = self.connection.cursor()
        with self.lock:
            c.execute("INSERT INTO messages (id, from_client_id, to_client_id, type, content) VALUES (?, ?, ?, ?, ?)", (message.message_id, message.from_client_id, target_client_id, message.message_type, message.messsage_content))
            self.connection.commit()

    def retrieve_and_delete_all_waiting_messages(self, client_id: bytes):
        c = self.connection.cursor()
        with self.lock:
            c.execute("SELECT from_client_id, id, type, content FROM messages WHERE to_client_id=?", (client_id,))
            result = c.fetchall()
            c.execute("DELETE FROM messages WHERE to_client_id=?", (client_id,))
            self.connection.commit()
        return result
    
    def retrieve_all_users(self):
        c = self.connection.cursor()
        with self.lock:
            c.execute("SELECT id, username FROM clients")
            return c.fetchall()
    
    def username_exists(self, username: bytes):
        c = self.connection.cursor()
        with self.lock:
            c.execute("SELECT 1 FROM clients WHERE username=? LIMIT 1", (username,))
            return c.fetchone()
    
    def get_public_key_of_user(self, client_id):
        c = self.connection.cursor()
        with self.lock:
            c.execute("SELECT public_key FROM clients WHERE id=?", (client_id,))
            return c.fetchone()