from bootload_tool import *
import time

tool.command_send_file(0x8000, path)
time.sleep(5)
tool.command_jump_addr(0x8000)


