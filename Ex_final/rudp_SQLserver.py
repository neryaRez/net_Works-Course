import socket
import pickle
from DB_manager import queries_Factory
from DB_manager import DB_applicationStr as Str
from Rudp_resource import rudp_packet
from Rudp_resource import rudp_user
import threading

'''@rudp_SQL server
This script implements SQL server which works on UDP protocol , but according to the rules and the 
techniques of TCP protocol.
The connection between the server and the client implements an app of SQL server and client, such that 
the rudp_SQL-server represents a list of 10 queries, the rudp_SQL_client has to choose one of them, and then 
the server has to supply the answer from his connection to the data base.
'''

# The connection to the database, in order to supply an answers to the queries which are chosen by the client.
queries = queries_Factory()

'''@Checker_3dupACK_daemon
This function implements packet-loss scenario such that, the client gets a segment which is not of the original
order of the segments that he has to get, and therefore he sends 3 dup_ACK messages to the server.
Then the server searches the segment that he has to send (which is the packet that he has not succeeded to send before),
and send it to the server.
This function should be applied on daemon thread, which he's always waits and stands to 3 dupACKs from the client.
'''


def Checker_3dupACK_daemon(user: rudp_user):
    counter_dupAck = 0
    while counter_dupAck < 3:
        data, client_ = user.recv_packet()
        counter_dupAck += 1

    if counter_dupAck == 3:
        for packet in user.packets_SendList:
            if packet.seq == data.acknowledge:
                user.send_packet(packet, client_)


'''@Hand_Shake
This Function implements the synchronization part, which is also known as "The 3 way HandShake" process, of
the tcp connection.
The client sends: SYN 
The server accepts the message and sends: SYN ACK
Then the client sends : ACK
'''


def Hand_Shake(user: rudp_user):
    data_syn, client = user.recv_packet()
    print("get SYN")
    synAck_packet = rudp_packet(SYN=True, ACK=True)
    user.send_packet(synAck_packet, client)
    print("send SYN ACK")
    user.recv_packet()
    print("get ACK")
    return client


'''@Close_Connection
This Function implements the "end connection" part, of the tcp connection.
The client sends: FIN 
The server accepts the message and sends: FIN ACK
Then the client sends : ACK
'''


def Close_Connection(user: rudp_user):
    data_fin, client = user.recv_packet()
    print("get FIN")
    finAck_packet = rudp_packet(FIN=True, ACK=True, seq=user.last_seq, acknowledge=user.last_acknowledge)
    user.send_packet(finAck_packet, client)
    print("send FIN ACK")
    user.recv_packet()
    print("get ACK")
    user.sock.close()


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


HOST = "127.0.0.1"
PORT = 30989

if __name__ == "__main__":
    # opens the socket and the rudp connection between both sides
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((HOST, PORT))
    server_user = rudp_user(sock)
    client = Hand_Shake(server_user)

    # stats the connection between both sides of a start number of the seq_number, and the ack_number.
    # The start number of the rudp_connection process is 1.
    server_user.last_acknowledge = 1
    server_user.last_seq = 1

    # Those lines show our try to set a daemon thread, which is role is to stand and wait for 3 dup_ACK
    # messages.
    # We try to implement this part from a different rudp_user, which has to be created from a copy - constructor,
    # so there will not be a dead_block in our code

    # THE LINES - >

    # daemon_user = rudp_user(server_user)
    # threading.Thread(target=Checker_3dupACK_daemon(daemon_user), daemon=True).start()

    # The server sends the "opening_list" of our application which represents 10 queries to the user.
    # The user has to choose one of them.
    # The decision is made by the user.
    server_user.send_packet(rudp_packet(ACK=True, PSH=True, seq=server_user.last_seq, acknowledge=server_user
                                        .last_acknowledge, data=Str), client)
    op_choise, client = server_user.recv_packet()
    print(f'got from client {op_choise.data}')

    # The server supplies the answer to the client
    s = navigator_queries(int(op_choise.data))

    server_user.send_packet(rudp_packet(ACK=True, PSH=True, seq=server_user.last_seq, acknowledge=server_user
                                        .last_acknowledge, data=s), client)

    cont = ''' Do you want to send me another query? if yes: type Y. otherwise: type N.\n'''
    cont_choose = 'In order to choose the ith query of the list, type i, while 1 <= i <= 10.\n'

    # The while loop of the application:
    while True:
        cont_packet = rudp_packet(ACK=True, PSH=True, seq=server_user.last_seq,
                                  acknowledge=server_user.last_acknowledge, data=cont)
        server_user.send_packet(cont_packet, client)
        print("ask the client if he wants to send more data")
        # The decision if to continue or not, which is made by the user.
        data_, client = server_user.recv_packet()
        if data_.data == 'N':
            break
        # Server asks the client again which query he wants to ask him.
        server_user.send_packet(rudp_packet(ACK=True, PSH=True, seq=server_user.last_seq, acknowledge=
        server_user.last_acknowledge, data=cont_choose), client)
        data_choose, client = server_user.recv_packet()
        print(f'got from client {data_choose.data}')
        # Again, The server sends the answer to the client.
        s = navigator_queries(int(data_choose.data))

        server_user.send_packet(rudp_packet(ACK=True, PSH=True, seq=server_user.last_seq, acknowledge=
        server_user.last_acknowledge, data=s), client)
    # The close of the rudp - connection
    Close_Connection(server_user)
