import socket
server_address = ('localhost', 2001)
# Define DNS records
dns_records = {

    'www.youtube.com': '208.65.153.238',
    'www.google.com': '8.8.8.8',
    'www.example.com': '192.168.0.2',
    'mail.example.com': '192.168.0.3'
}
# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Bind the socket to the server address
sock.bind(server_address)
# Create a dictionary to store cached DNS records
cache = {}

print('DNS server listening ')

while True:
    data, address = sock.recvfrom(4096)
    # Parse DNS query
    query = data.decode().strip()
    print('the client need the IP of:', query)

    if query in cache:
        # Return cached DNS record if available
        response = cache[query]
    elif query in dns_records:
        # Build DNS response
        response = dns_records[query]
        # Cache the DNS record
        cache[query] = response
    else:
        # Return an error message if the DNS record is not found
        response = "DNS record not found"
    print('the client will get:', response)
    # Send the DNS response to the client
    sock.sendto(response.encode(), address)
