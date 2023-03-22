import socket
import random

server_address = ('localhost', 67)

# Define DHCP message types
DHCP_DISCOVER = 1
DHCP_OFFER = 2
DHCP_REQUEST = 3
DHCP_ACK = 4

# Define IP address pool
ip_pool = ['192.168.0.' + str(i) for i in range(1, 255)]

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the server address
sock.bind(server_address)

print('DHCP server listening on {}:{}'.format(*server_address))

while True:
    data, address = sock.recvfrom(4096)

    # Parse DHCP DISCOVER message
    msg_type, client_mac = data.decode().split(',')
    if int(msg_type) == DHCP_DISCOVER:
        # Generate a random IP address from the pool
        ip_address = random.choice(ip_pool)

        # Build DHCP OFFER message
        msg = ','.join([str(DHCP_OFFER), ip_address, client_mac])
        sock.sendto(msg.encode(), address)
        print('Sent DHCP OFFER message to client')

        # Wait for DHCP REQUEST message
        data, address = sock.recvfrom(4096)
        msg_type, ip_address, client_mac = data.decode().split(',')
        if int(msg_type) == DHCP_REQUEST:
            # Assign the requested IP address to the client
            if ip_address in ip_pool:
                ip_pool.remove(ip_address)

                # Build DHCP ACK message
                msg = ','.join([str(DHCP_ACK), ip_address, client_mac])
                sock.sendto(msg.encode(), address)
                print('Sent DHCP ACK message to client')
