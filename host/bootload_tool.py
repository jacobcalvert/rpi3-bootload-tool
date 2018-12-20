import serial
import time
import os
import logging

class BootloadTool(object):
	CMD_ECHO = 0x00
	CMD_START_TRANSFER = 0x01
	CMD_WRITE_CHUNK	= 0x02
	CMD_FINALIZE_TRANSFER = 0x03
	CMD_JUMP_ADDR = 0x04
	
	START = 0x93
	END	= 0x39



	RC_DECODE = {
		0x00: "Generic OK",



		0x80: "Generic Error",
		0x81: "Chunk too big",
		0x82: "Bad Checksum",
		0x83: "Unknown Command"
	
	}

	def __init__(self, port, baud):
		self._serial = serial.Serial(port, baud)
		logging.basicConfig(format="[%(asctime)s] %(levelname)s - %(message)s")
		self._log = logging.getLogger("bootload_tool")
		self._log.setLevel(logging.INFO)
		self._log.info("Bootload Tool Initialized")

	

	def __wb(self, b):
		self._log.debug("writing byte %s" % hex(b))
		self._serial.write(chr(b))
	
	def __ws(self, s):
		self._log.debug("writing str '%s'" % s)
		for c in s:
			self.__wb(ord(c))

	def __rb(self):
		return ord(self._serial.read(1))

	def __rs(self, n):
		s = ""
		self._log.debug("inWaiting = %u" % self._serial.inWaiting())
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
		self._log.info("transaction begin")
		self._log.info("CMD_ECHO")
		self.__wb(self.START)
		self.__wb(self.CMD_ECHO)
		self.__wint(len(what))
		self.__ws(what)
		self._log.info("Rx'd echo of: " +self.__rs(len(what)))
		self.__end_transaction()


	def command_tx_setup(self, start_addr, size, cs):
		self._log.info("transaction begin")
		self._log.info("CMD_START_TRANSFER")
		self.__wb(self.START)
		self.__wb(self.CMD_START_TRANSFER)
		self.__wint(start_addr)
		self.__wint(size)
		self.__wb(cs)
		self.__end_transaction()

	def command_write_chunk(self, chunk):
		self._log.info("transaction begin")
		self._log.info("CMD_WRITE_CHUNK")
		self.__wb(self.START)
		self.__wb(self.CMD_WRITE_CHUNK)
		sz = len(chunk)
		cs = checksum(chunk)
		self._log.info("writing chunk of size %u with cs = %s" % (sz, hex(cs)))
		self.__wint(sz)
		self.__ws(chunk)
		self.__wb(cs)
		self.__end_transaction()
	
	def command_finalize_transfer(self):
		self._log.info("transaction begin")
		self._log.info("CMD_FINALIZE_TRANSFER")
		self.__wb(self.START)
		self.__wb(self.CMD_FINALIZE_TRANSFER)
		self.__end_transaction()
	
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
		self._log.info("transaction begin")
		self._log.info("CMD_JUMP_ADDR")
		self.__wb(self.START)
		self.__wb(self.CMD_JUMP_ADDR)
		self.__wint(addr)
		self._log.info("no return expected")

	def __end_transaction(self):
		self._decode_rc(self.__rb())
		if self.__rb() == self.END:
			self._log.info("transaction done")

	def _decode_rc(self, b):
		if b in self.RC_DECODE:
			self._log.info(self.RC_DECODE[b])
		else:
			self._log.error("unknown rc code = %s" % hex())
		


def checksum(blob):
	r = 0
	for c in blob:
		r += ord(c)

	return r & 0x0FF

