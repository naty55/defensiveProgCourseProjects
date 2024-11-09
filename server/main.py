from utils import get_server_port
from logs import Logger

logger = Logger()

logger.info("Starting server on " + str(get_server_port()))

