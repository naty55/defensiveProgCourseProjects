from enum import Enum



class LoggingLevel(Enum):
    DEBUG = 1
    INFO = 2
    WARN = 3
    ERROR = 4

class Logger:
    """
    Really simple implementation of logger with basic functionality
    (I was not sure if we are allowed to use standard python logger in the scope of the course task)
    """

    def __init__(self, logging_level = LoggingLevel.DEBUG) -> None:
        self.logging_level = logging_level

    def debug(self, message):
        if LoggingLevel.DEBUG.value >= self.logging_level.value:
            print(f"DEBUG - {message}")

    def warn(self, message):
        if LoggingLevel.WARN.value >= self.logging_level.value:
            print(f"WARN - {message}")

    def info(self, message):
        if LoggingLevel.INFO.value >= self.logging_level.value:
            print(f"INFO - {message}")
    
    def error(self, message):
        if LoggingLevel.ERROR.value >= self.logging_level.value:
            print(f"ERROR - {message}")
        
        
    
logger = Logger(LoggingLevel.DEBUG)