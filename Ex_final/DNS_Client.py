import socket

server_address = ('localhost', 2001)

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    # Get user input for DNS query
    query = input("Enter a DNS query: ")

    # Send DNS query to server
    sock.sendto(query.encode(), server_address)

    # Receive DNS response from server
    data, address = sock.recvfrom(4096)
    response = data.decode().strip()
    print()
    # Print DNS response
    print(response)
