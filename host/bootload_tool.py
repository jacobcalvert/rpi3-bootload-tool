import serial
import time
import os
class BootloadTool(object):
	CMD_ECHO = 0x00
	CMD_START_TRANSFER = 0x01
	CMD_WRITE_CHUNK	= 0x02
	CMD_FINALIZE_TRANSFER = 0x03
	CMD_JUMP_ADDR = 0x04
	
	START = 0x93
	END	= 0x39

	def __init__(self, port, baud):
		self._serial = serial.Serial(port, baud)

	

	def __wb(self, b):
		#print "Writing byte: %s" % hex(b)
		self._serial.write(chr(b))
	
	def __ws(self, s):
		print "Writing string: %s" % s
		for c in s:
			self.__wb(ord(c))

	def __rb(self):
		return ord(self._serial.read(1))

	def __rs(self, n):
		s = ""
		print "inWaiting = %u" % self._serial.inWaiting()
		while n != 0:
			s += self._serial.read(1)
			n -=1
		return s

	def __wint(self, n):
		self.__wb(n&0x0FF)
		self.__wb(n>>8&0x0FF)
		self.__wb(n>>16&0x0FF)
		self.__wb(n>>24&0x0FF)
		
	def command_echo(self, what):
		self.__wb(self.START)
		self.__wb(self.CMD_ECHO)
		self.__wint(len(what))
		self.__ws(what)
		print self.__rs(len(what))
		if self.__rb() == 0x00:
			print "Result was OK"
		if self.__rb() == self.END:
			print "TRANSACTION COMPLETE"


	def command_tx_setup(self, start_addr, size, cs):
		self.__wb(self.START)
		self.__wb(self.CMD_START_TRANSFER)
		self.__wint(start_addr)
		self.__wint(size)
		self.__wb(cs)
		if self.__rb() == 0x00:
			print "Setup was OK"
		else:
			print "------> Setup failed!!"
		if self.__rb() == self.END:
			print "TRANSACTION COMPLETE"

	def command_write_chunk(self, chunk):
		self.__wb(self.START)
		self.__wb(self.CMD_WRITE_CHUNK)
		sz = len(chunk)
		cs = checksum(chunk)
		print "Writing chunk of size %u with cs = %s" % (sz, hex(cs))
		self.__wint(sz)
		self.__ws(chunk)
		self.__wb(cs)
		result = self.__rb()
		if result == 0x00:
			print "Chunk successful"
		else:
			print "------> Chunk failed!! CODE WAS %s" % hex(result)
		if self.__rb() == self.END:
			print "TRANSACTION COMPLETE"
	
	def command_finalize_transfer(self):
		self.__wb(self.START)
		self.__wb(self.CMD_FINALIZE_TRANSFER)
		result = self.__rb()
		if result == 0x00:
			print "Transfer successful"
		else:
			print "------> Transfer failed!! result = %s" % hex(self.__rb())
		if self.__rb() == self.END:
			print "TRANSACTION COMPLETE"
	
	def command_send_file(self, start_addr, fname):
		fp = open(fname, "rb")
		blob = fp.read()
		size = os.fstat(fp.fileno()).st_size
		fp.close()
		cs = checksum(blob)
		self.command_tx_setup(start_addr, size, cs)
		# do tx
		idx = 0
		chunk_size = 64
		while idx < size:
			l = min(size-idx, chunk_size)
			self.command_write_chunk(blob[idx:l+idx])
			idx += l
		
		self.command_finalize_transfer()


	def command_jump_addr(self, addr):
		self.__wb(self.START)
		self.__wb(self.CMD_JUMP_ADDR)
		self.__wint(addr)
		print "No return expected"
		
		


def checksum(blob):
	r = 0
	for c in blob:
		r += ord(c)

	return r & 0x0FF

tool = BootloadTool("/dev/ttyUSB0", 115200)
s = tool._serial

tool.command_echo("Hello world")
path="/media/jacob/jacob/Documents/Workspaces/OSDev/rpi3-bootload-tool/test-image/build/kernel7.img"
path2="/media/jacob/jacob/Documents/Workspaces/OSDev/rpi3-bootload-tool/test-image/build/file.txt"

