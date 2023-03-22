import socket

client_address = ('localhost', 68)
server_address = ('localhost', 67)

# Define DHCP message types
DHCP_DISCOVER = 1
DHCP_OFFER = 2
DHCP_REQUEST = 3
DHCP_ACK = 4

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(client_address)

# Build DHCP DISCOVER message
msg = ','.join([str(DHCP_DISCOVER), '00:11:22:33:44:55'])
sock.sendto(msg.encode(), server_address)

while True:
    data, server = sock.recvfrom(4096)

    # Parse DHCP OFFER message
    msg_type, ip_address, server_mac = data.decode().split(',')
    if int(msg_type) == DHCP_OFFER:
        # Build DHCP REQUEST message
        msg = ','.join([str(DHCP_REQUEST), ip_address, '00:11:22:33:44:55'])
        sock.sendto(msg.encode(), server_address)

    # Parse DHCP ACK message
    data, server = sock.recvfrom(4096)
    msg_type, ip_address, server_mac = data.decode().split(',')
    if int(msg_type) == DHCP_ACK:
        print('Assigned IP address:', ip_address)
        break
