import socket

'''@tcp_SQL client
This script implements SQL client which works on tcp protocol.
The connection between the server and the client implements an app of SQL server and client, such that 
the tcp_SQL-server represents a list of 10 queries, the tcp_SQL_client has to choose one of them, and then 
the server has to supply the answer from his connection to the data base. 
'''

HOST = '127.0.0.1'
PORT = 20989

if __name__ == "__main__":
    # opens the socket and the tcp connection between both sides
    client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_sock.connect((HOST, PORT))
    print(f"connect on {HOST, PORT}")

    # This is the opening data, that should represent the "10 queries_list", to the client.
    # Then, the client has to choose which query he wants to ask from the server.
    # The decision is made by the user.
    data1 = client_sock.recv(1024).decode('utf-8')
    data_Input = input(data1)

    # The client sends his own choise to the server, and prints the answer of his query.
    client_sock.sendall(data_Input.encode('utf-8'))
    data2 = client_sock.recv(1024).decode('utf-8')
    print(data2)
    # The client accepts this answer.
    client_sock.send("accept".encode('utf-8'))

    # The while loop of the application, which runs as long as the user wants.
    y = 'Y'
    while y == 'Y':
        # The server asks the client if he wants to continue.
        # Then the client sends his answer, which is made by the user.
        data_cont = client_sock.recv(1024).decode('utf-8')
        y = input(data_cont)
        if y == 'N':
            client_sock.sendall(y.encode('utf-8'))
            break

        client_sock.sendall(y.encode('utf-8'))
        # The server asks the client, which query he wants to ask him.
        data_cont2 = client_sock.recv(1024).decode('utf-8')
        data_Input = input(data_cont2)
        client_sock.sendall(data_Input.encode('utf-8'))
        # Client gets his answer and then prints it.
        data2 = client_sock.recv(1024).decode('utf-8')
        print(data2)
        # Client accepts this answer
        client_sock.send("accept".encode('utf-8'))

    # close the connection
    client_sock.close()
