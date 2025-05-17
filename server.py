import socket, struct

HOST = '0.0.0.0'
PORT = 5000

def handle_client(conn, addr):
    print(f"Connected by {addr}")
    buf = b''
    try:
        while True:
            data = conn.recv(1024)
            if not data:
                print(f"Client {addr} disconnected.")
                break
            buf += data
            # process all full 12-byte position packets
            while len(buf) >= 12:
                chunk, buf = buf[:12], buf[12:]
                x, y, z = struct.unpack('<fff', chunk)
                print(f"Position: x={x:.3f}, y={y:.3f}, z={z:.3f}")
    except ConnectionResetError:
        print(f"Connection reset by {addr}.")
    finally:
        conn.close()

def run_server(host=HOST, port=PORT):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.bind((host, port))
        server.listen()
        print(f"Server listening on {host}:{port}")
        while True:
            try:
                conn, addr = server.accept()
                handle_client(conn, addr)
            except KeyboardInterrupt:
                print("\nServer shutting down.")
                break

if __name__ == '__main__':
    run_server()
