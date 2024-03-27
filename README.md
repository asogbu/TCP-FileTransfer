# TCP-FileTransfer

## Computer Networks: Semester Project #1

A simple client-server application that transfers a file over a TCP connection. See [project description](https://nd-cse-30264.github.io/Project-Description/).

### Design

#### Client

The high level structure of the client is as follows:

- Process command-line arguments (port to listen, directory where to save files).
- Set up TCP socket to listen at desired port.
- Wait for a connection.
- Recieve a chunk of file, write it to file, and repeat until connection is closed by client (or timeout).
- Go back to wait for the next connection.
- If terminating signal receive, quit.

#### Server

The high level structure of the server is as follows:

- Process command-line arguments (host, port, file to send).
- Connect to server using a TCP connection.
- Read a chunk of file, send it, and repeat until whole file sent.
- Close connection.

### Problem solving

The main problem I ran into was in passing Test 15:

> Server aborts connection (a file should be created, containing only ERROR string) when it does not receive data from client for more than 10 seconds.

First, I wrote my own slow client to test with the server to see if I would get the expected behavior. The behavior and output seemed to be correct. Then, I tried changing the implementation of the server timeout from (current) `setsockopt(..., SO_RCVTIMEO, ...)` to instead set the socket as nonblocking and monitor it with `select`. This didn't change the output either. Finally, I tried analyzing and modifying the code from the autograder, and I was able to see that the grader was getting several 0 bytes before getting to the ERROR string. Thus, I figured I had forgot to rewind the filestream between truncating the file and writing ERROR to it. Problem solved.

### Resources

The function `socket_dial` present in client.cpp is a modified version of code written for [Homework 9](https://www3.nd.edu/~pbui/teaching/cse.20289.sp22/homework09.html) of [Systems Programming](https://www3.nd.edu/~pbui/teaching/cse.20289.sp22/).

Similarly, the function `socket_listen` present in socket.cpp is based on `socket_dial`.

Man pages for the involved functions were heavily referenced and occasional consulting of [Stack Overflow](https://stackoverflow.com) posts.

## Author

Andrés Santiago Orozco Gorjón \
NDID: 902078552
