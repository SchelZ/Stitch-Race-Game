import socket

HOST = '0.0.0.0'
PORT = 5000

def run_server(host=HOST, port=PORT):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.bind((host, port))
        server.listen(1)
        print(f"Listening on {host}:{port}…")
        conn, addr = server.accept()
        with conn:
            print(f"Connected by {addr}")
            while True:
                data = conn.recv(1024)
                if not data:
                    print("Client disconnected.")
                    break
                print("Received:", data)

if __name__ == '__main__':
    try:
        run_server()
    except KeyboardInterrupt:
        print("\nServer shut down.")
