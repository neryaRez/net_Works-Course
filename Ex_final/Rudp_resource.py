import socket
import pickle
import sys
import math

'''@rudp_packet
The fields of each packet are according to the TCP protocol.
SYN - to start connection. 
ACK - to ensure receiving of messages.
FIN - to cancel connection.
PSH - to push data.
seq & acknowledge represent the seq_number and the ack_number of each packet.
data - is basically the data.
'''


class rudp_packet:

    def __init__(self, ACK=False, PSH=False, SYN=False, FIN=False, seq=0, acknowledge=0, data="") -> None:
        self.ACK = ACK
        self.PSH = PSH
        self.SYN = SYN
        self.FIN = FIN
        self.data = data
        self.seq = seq
        self.acknowledge = acknowledge


'''@rudp_user
This class represents an user who works on rudp - connection.
User could be server or client.
In order to implement the rudp connection which is just like as TCP, each user has:
1) Windows of how many packets he sends and how many packets he gets which are
   represent by tuple with right and left borders.
2) socket to connect.
3) last_sequence, and last_acknowledge - as data members, in order to ensure packet - loss.
4) lists of:
    a) ACK's packets -  includes all of the acks that he has gotten.
    b) send - packets - includes all of the data - packets which he has sent.
    c) recv - packets - includes all of the data - packets that he has gotten.
Those lists have to help the rudp-user to follow on his network-traffic, so he could be reliable like a TCP user.    
'''


class rudp_user:
    __left_send = 0
    __right_send = 4
    __left_recv = 0
    __right_recv = 4

    def __init__(self, sock: socket.socket):
        self.sock = sock
        self.last_seq = 0
        self.last_acknowledge = 0
        self.packets_SendList: list[rudp_packet] = []
        self.acks_list: list[rudp_packet] = []
        self.packets_RecvList: list[rudp_packet] = []
        self.window_send = (self.__left_send, self.__right_send)
        self.window_recv = (self.__left_recv, self.__right_recv)

    ''' This is a try to implement a deep_copy - constructor, in order to set a method
    that has to listen and follow about dup_ACKS as a daemon thread.
    This try wasn't went so good, but you have to consider our thinking.
    '''

    # def __init__(self, other: rudp_user):
    #     self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #     self.last_seq = other.last_seq
    #     self.last_acknowledge = other.last_acknowledge
    #     self.packets_list = other.packets_list
    #     self.acks_list = other.acks_list

    '''@segmentation_Send
    This method takes a large - sized packets and divide them to a different segments, so the traffic on 
    the internet will be easily and effectively-flows.
    Each segment which is sent on this protocol, should be no longer than 65536 bytes.
    But each packet has own fields which make it as rudp - packet.
    So, to ensure that each segment will be sent as an rudp packet, we decided to divide the data of the packet,
    for a chunks of 32768 bytes, and send all of those chunks as rudp packets. 
    '''

    def segmentation_Send(self, packet: rudp_packet, dest: (str, int)):
        counter = 0
        size = len(packet.data)
        times = math.ceil(size/32768)
        # first of all we let the receiver knows how many bytes we are going to send - therefore
        # we send him the number of times, that we are going to send segments.
        self.send_packet(rudp_packet(ACK=True, PSH=True, seq=packet.seq, acknowledge=packet.acknowledge
                                     , data=str(times)), dest)
        while counter < size:
            if size - counter > 32768:
                self.send_packet(rudp_packet(ACK=True, PSH=True, seq=packet.seq+counter, acknowledge=packet.acknowledge,
                                             data=packet.data[counter: counter+32768]), dest)
            else:
                self.send_packet(rudp_packet(ACK=True, PSH=True, seq=packet.seq+counter, acknowledge=packet.acknowledge,
                                             data=packet.data[counter: size]), dest)
            counter += 32768

    '''@send_packet
    This function sends rudp-packets, to the destination he gets as parameter.
    if the packet is too much long - then it divided to a small segments of no longer than 65536 bytes.
    The packet is added to the packets_SendList.
    Then , it gets an ack of the packet, and add it to the acks_list.
    The window_send is updated. 
    if there is a timeout, or wrong ack, the method retransmits the packet.
    '''

    def send_packet(self, packet: rudp_packet, dest: (str, int)):
        if sys.getsizeof(pickle.dumps(packet)) > 65536:
            self.segmentation_Send(packet, dest)

        self.sock.sendto(pickle.dumps(packet), dest)
        self.packets_SendList.append(packet)
        # ensure time_out, or wrong ack of this message
        self.sock.settimeout(50)
        try:
            # gets an ack of the packet
            dat: rudp_packet = pickle.loads(self.sock.recv(2048))
            self.acks_list.append(dat)
            # updates the window_send
            self.__right_send += 1
            self.__left_send += 1
            self.window_send = (self.__left_send, self.__right_send)
            if not dat.ACK or not dat.acknowledge == self.last_seq + len(packet.data):
                print(
                    "ack fail, expected: " + str(self.last_seq + len(packet.data)) + " but got " + str(dat.acknowledge))
                self.send_packet(packet, dest)
        except socket.timeout:
            self.send_packet(packet, dest)
        # updates the last_seq of the rudp - user.
        self.last_seq += len(packet.data)

    '''@recv_packet
    This method receives rudp-packets from another rudp-user.
    Checks if the seq_number is not as expected, then send 3 dup_ACKS about the correct seq_number's packet, which
    has to be received.
    Updates the last_acknowledge field of the rudp - user. 
    Updates the packets_RecvList.
    Updates the window_recv.
    
    :returns the packet which we get, and the connection to the sender of the packet.
    '''

    def recv_packet(self):
        data_bytes = 0
        (data_bytes, client) = self.sock.recvfrom(2048)
        dat: rudp_packet = pickle.loads(data_bytes)
        # Checks if the seq_number is not as expected
        if dat.seq != self.last_acknowledge:
            self.send_3_dupAck(client)
            return self.recv_packet()
        # Updates: the last_acknowledge, the packets_RecvList, and the window_recv.
        self.last_acknowledge = dat.seq + len(dat.data)
        self.packets_RecvList.append(dat)
        self.__left_recv += 1
        self.__right_recv += 1
        self.window_recv = (self.__left_recv, self.__right_recv)
        # sends ACK of the packet
        ack = rudp_packet(ACK=True, seq=self.last_seq, acknowledge=self.last_acknowledge)
        self.sock.sendto(pickle.dumps(ack), client)
        return dat, client

    '''@send_3_dupAck
    Sends 3 dup_ACK messages in a case of packet - loss scenario, but a get of packets not in order.
    '''

    def send_3_dupAck(self, dest: (str, int)):
        self.send_packet(rudp_packet(ACK=True, seq=self.last_seq, acknowledge=self.last_acknowledge), dest)
        self.send_packet(rudp_packet(ACK=True, seq=self.last_seq, acknowledge=self.last_acknowledge), dest)
        self.send_packet(rudp_packet(ACK=True, seq=self.last_seq, acknowledge=self.last_acknowledge), dest)

    def set_sock(self, sock: socket.socket):
        self.sock = sock
