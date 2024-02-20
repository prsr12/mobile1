# mobile1
Project1 for mobile computing

This Project is a simple client-server application which sends a file of upto 100 mb from the client to the server.

The server application uses the port number and path to where the file should be saved as Command line arguments.
./server (port) (path)

The client uses the server's IP, port number and the file to be transferred as the Command line argument
./client (Server IP)(port)( file name)

Features of the server - 
The server has a stay alive function, it keeps accepting new connection, until manually shutdown or it has 30 seconds of inactivity.
The server
