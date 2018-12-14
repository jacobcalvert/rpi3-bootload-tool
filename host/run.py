from bootload_tool import *
import time
import argparse

parser = argparse.ArgumentParser()

parser.add_argument("-e", "--echo", help="perform an echo test", type=str, default=None)
parser.add_argument("-d", "--device", help="tty device to use", type=str, required=True)
parser.add_argument("-f", "--file", help="file to upload", type=str)
parser.add_argument("-s", "--start", help="start address", type=str)

args = parser.parse_args()

tool = BootloadTool(args.device, 115200)
if args.echo:
	tool.command_echo(args.echo)

if args.file:
	tool.command_send_file(0x8000, args.file)


if args.start:
	tool.command_jump_addr(int(args.start, 0))
