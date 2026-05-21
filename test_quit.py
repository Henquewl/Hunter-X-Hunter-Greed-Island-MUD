import socket, time

def send(s, data):
    s.sendall(data.encode('latin-1'))

def recv_until(s, marker, timeout=15):
    s.settimeout(timeout)
    buf = b''
    try:
        while True:
            chunk = s.recv(1024)
            if not chunk: break
            i = 0
            while i < len(chunk):
                b = chunk[i]
                if b == 255:
                    if i+1 < len(chunk):
                        cmd = chunk[i+1]
                        if cmd in (251,252,253,254) and i+2 < len(chunk):
                            opt = chunk[i+2]
                            if cmd == 253: s.sendall(bytes([255,252,opt]))
                            elif cmd == 251: s.sendall(bytes([255,254,opt]))
                            i += 3; continue
                        i += 2; continue
                else:
                    buf += bytes([b])
                    i += 1
            if marker and marker.encode('latin-1') in buf:
                return buf.decode('latin-1', errors='replace')
    except socket.timeout:
        pass
    return buf.decode('latin-1', errors='replace')

s = socket.socket()
s.connect(('127.0.0.1', 4000))
recv_until(s, 'Name:')
time.sleep(0.3); send(s, 'henque\r\n')
recv_until(s, 'Password:')
time.sleep(0.3); send(s, 'klapaucius\r\n')
recv_until(s, 'PRESS ENTER')
time.sleep(0.3); send(s, '\r\n')
recv_until(s, 'Enter the game')
time.sleep(0.3); send(s, '1\r\n')
recv_until(s, 'nen')
time.sleep(1)
print('[SENDING QUIT]')
send(s, 'quit\r\n')
text = recv_until(s, 'really', timeout=5)
print('[RECV QUIT PROMPT]', repr(text[-100:]))
time.sleep(0.5)
send(s, 'yes\r\n')
text = recv_until(s, '', timeout=8)
print('[FINAL]', repr(text[-200:]))
s.close()
print('[DONE]')
