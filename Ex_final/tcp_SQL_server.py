import sqlite3
import socket
from DB_manager import queries_Factory
from DB_manager import DB_applicationStr as Str

'''@tcp_SQL server
This script implements SQL client which works on tcp protocol.
The connection between the server and the client implements an app of SQL server and client, such that 
the tcp_SQL-server represents a list of 10 queries, the tcp_SQL_client has to choose one of them, and then 
the server has to supply the answer from his connection to the data base. 
'''

# The connection to the database, in order to supply an answers to the queries which are chosen by the client.
queries = queries_Factory()

'''@navigator_queries
Gets an int as parameter, and then return as a string, the requested query from the client.
Each query of the "queries_list" of our application has its own id number which is a number 
between 1 to 10.
'''


def navigator_queries(d: int):
    s = ''''''
    if d == 1:
        s += queries.q1()
    elif d == 2:
        s += queries.q2()
    elif d == 3:
        s += queries.q3()
    elif d == 4:
        s += queries.q4()
    elif d == 5:
        s += queries.q5()
    elif d == 6:
        s += queries.q6()
    elif d == 7:
        s += queries.q7()
    elif d == 8:
        s += queries.q8()
    elif d == 9:
        s += queries.q9()
    else:
        s += queries.q10()

    return s


HOST = '127.0.0.1'
PORT = 20989

if __name__ == "__main__":
    # opens the socket and the tcp connection between both sides.
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((HOST, PORT))
    print(f"bind on {HOST, PORT}")
    server_socket.listen(5)
    msg_Socket, address = server_socket.accept()
    print(f"accept client from: {address}")

    # The server sends the "opening_list" of our application which represents 10 queries to the user.
    # The user has to choose one of them.
    # The decision is made by the user.
    msg_Socket.sendall(Str.encode('utf-8'))
    data = msg_Socket.recv(1024).decode('utf-8')
    print(f"got from client {data}")

    # The server's answer of the query.
    s = navigator_queries(int(data))
    msg_Socket.sendall(s.encode('utf-8'))
    msg_Socket.recv(1024).decode('utf-8')

    cont = ''' Do you want to send me another query? if yes: type Y. otherwise: type N.\n'''

    # The while true of the application - runs as long as the user wants to apply this app.
    while True:
        # Ask the client if he wants to send more data, for another round.
        msg_Socket.sendall(cont.encode('utf-8'))
        print("ask the client if he wants to send more data")
        data1 = msg_Socket.recv(1024).decode('utf-8')
        if data1 == 'N':
            break
        # Ask the client which query he wants to choose in this round.
        msg_Socket.sendall('In order to choose the ith query of the list, type i, while 1 <= i <= 10.\n'.encode('utf-8'))
        data2 = msg_Socket.recv(1024).decode('utf-8')
        print(f"got from client {data2}")
        # Answers the client's query.
        s = navigator_queries(int(data2))
        msg_Socket.sendall(s.encode('utf-8'))
        # Gets an accept from the client.
        msg_Socket.recv(1024).decode('utf-8')

    # close the connection
    msg_Socket.close()
    server_socket.close()
    queries.connector.close()
