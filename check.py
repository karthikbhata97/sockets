import socket
import struct
import os
import time

def carry_around_add(a, b):
    c = a + b
    return (c & 0xffff) + (c >> 16)

def checksum(msg):
   s = 0
   for i in range(0, len(msg), 2):
       w = ord(msg[i]) + (ord(msg[i+1]) << 8)
       s = carry_around_add(s, w)
   ans = ~s & 0xffff
   ans = (ans >> 8) | (ans << 8) & 0xFF00
   return ans


s = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.getprotobyname('icmp'))
s.bind(('127.0.0.1', 3003))
header = struct.pack(">BBHHH", 8, 0, 0, os.getpid()&0xFFFF, 1) # type, code, checksum, id, seq_no

data = (192 - struct.calcsize("d")) * "k"
data = struct.pack("d", time.time()) + data

checksum_val = checksum(header + data)
header = struct.pack(">BBHHH", 8, 0, checksum_val, os.getpid()&0xFFFF, 1) # type, code, checksum, id, seq_no
message = header+data
s.sendto(message, ('127.0.0.1', 80))
data, addr = s.recvfrom(1024)
print(len(data))
data = struct.unpack("BBHHH", data[20:28])
print(data)