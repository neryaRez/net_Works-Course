import socket
import pickle
from Rudp_resource import rudp_packet
from Rudp_resource import rudp_user

'''@rudp_SQL client
This script implements SQL client which works on UDP protocol , but according to the rules and the 
techniques of TCP protocol.
The connection between the server and the client implements an app of SQL server and client, such that 
the rudp_SQL-server represents a list of 10 queries, the rudp_SQL_client has to choose one of them, and then
the server has to supply the answer from his connection to the data base. 
'''
HOST = "127.0.0.1"
PORT = 30989

'''@Checker_3dupACK_daemon
This function implements packet-loss scenario such that, the server gets a segment which is not of the original
order of the segments that he has to get, and therefore he sends 3 dup_ACK messages to the client.
Then the client searches the segment that he has to send (which is the packet that he has not succeeded to send before),
and send it to the server.
This function should be applied on daemon thread, which he's always waits and stands to 3 dupACKs from the server.
'''


def Checker_3dupACK_daemon(user: rudp_user):
    counter_dupAck = 0
    while counter_dupAck < 3:
        data, server_ = user.recv_packet()
        counter_dupAck += 1

    if counter_dupAck == 3:
        for packet in user.packets_SendList:
            if packet.seq == data.acknowledge:
                user.send_packet(packet, server_)


'''@Hand_Shake
This Function implements the synchronization part, which is also known as "The 3 way HandShake" process, of
the tcp connection.
The client sends: SYN 
The server accepts the message and sends: SYN ACK
Then the client sends : ACK
'''


def Hand_Shake(user: rudp_user):
    syn = rudp_packet(SYN=True)
    user.send_packet(syn, (HOST, PORT))
    print("send SYN")

    user.recv_packet()
    print("get SYN ACK")
    ACK_Packet = rudp_packet(ACK=True)
    user.send_packet(ACK_Packet, (HOST, PORT))
    print("send ACK")


'''@Close_Connection
This Function implements the "end connection" part, of the tcp connection.
The client sends: FIN 
The server accepts the message and sends: FIN ACK
Then the client sends : ACK
'''


def Close_Connection(user: rudp_user):
    user.send_packet(rudp_packet(FIN=True, seq=user.last_seq, acknowledge=user.last_acknowledge), (HOST, PORT))
    print("send FIN")
    user.recv_packet()
    print("get FIN ACK")
    ACK_Packet = rudp_packet(ACK=True, seq=user.last_seq, acknowledge=user.last_acknowledge)
    user.send_packet(ACK_Packet, (HOST, PORT))
    print("send ACK")
    user.sock.close()


if __name__ == "__main__":
    # opens the socket and the rudp connection between both sides
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_user = rudp_user(sock)
    Hand_Shake(client_user)

    # stats the connection between both sides of a start number of the seq_number, and the ack_number.
    # The start number of the rudp_connection process is 1.
    client_user.last_acknowledge = 1
    client_user.last_seq = 1

    # Those lines show our try to set a daemon thread, which is role is to stand and wait for 3 dup_ACK
    # messages.
    # We try to implement this part from a different rudp_user, which has to be created from a copy - constructor,
    # so there will not be a dead_block in our code

    # THE LINES - >
    # daemon_user = rudp_user(server_user)
    # threading.Thread(target=Checker_3dupACK_daemon(daemon_user), daemon=True).start()

    # This is the opening data, that should represent the "10 queries_list", to the client.
    # Then, the client has to choose which query he wants to ask from the server.
    # The decision is made by the user.
    op_data, server = client_user.recv_packet()
    choise = input(op_data.data)

    # The client sends his own choise to the server as rudp_packet
    client_user.send_packet(rudp_packet(ACK=True, PSH=True, seq=client_user.last_seq, acknowledge=client_user
                                        .last_acknowledge, data=choise), (HOST, PORT))

    # The client gets his answer, and then print it.
    op_res, server = client_user.recv_packet()
    print(op_res.data)

    # The while loop of our application-running.
    y = 'Y'
    while y == 'Y':
        # The server sends an "ask-message" to the client, if he wants to continue to another round.
        # The decision is made by the user.
        packet_cont, server = client_user.recv_packet()
        y = input(packet_cont.data)
        # The client sends the decision to the server.
        if y == 'N':
            # if the answer is 'N', then the loop is broken.
            client_user.send_packet(rudp_packet(ACK=True, PSH=True, seq=client_user.last_seq, acknowledge=
            client_user.last_acknowledge, data=y), (HOST, PORT))
            break
        # Otherwise, the loop continues, and again:
        # The server asks the client which query he wants to ask just like as before
        client_user.send_packet(rudp_packet(ACK=True, PSH=True, seq=client_user.last_seq, acknowledge=
        client_user.last_acknowledge, data=y), (HOST, PORT))

        packet_cont2, server = client_user.recv_packet()
        data_input = input(packet_cont2.data)

        client_user.send_packet(rudp_packet(ACK=True, PSH=True, seq=client_user.last_seq, acknowledge=
        client_user.last_acknowledge, data=data_input), (HOST, PORT))
        packet_navigate, server = client_user.recv_packet()
        print(packet_navigate.data)

    # After the break's loop, the "end connection" part is about to start.
    Close_Connection(client_user)
