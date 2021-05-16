#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080

using namespace std;

const int LENGTH = 1024;

void clear_buffer(char buffer[], int size);
void print_buffer(char buffer[], int size);


int main(int argc, char* argv[]) {
    const bool DEBUG = false;

    // fd for server-connection
    int sock_fd;

    // holds address info
    struct sockaddr_in serv_addr;

    char buffer[LENGTH];
    clear_buffer(buffer, LENGTH);
    string message = "";

    int valread;    

    // create socket fd (AF_INET - IPv4; SOCK_STREAM - TCP (reliable, connection oriented))
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Socket creation error" << endl;
        exit(EXIT_FAILURE);
    }

    // set address details
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);       // port number in host -> network byte order

    // convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cout << "Invalid address / address not supported" << endl;
        exit(EXIT_FAILURE);
    }

    // connect socket fd to server's address and port
    if (connect(sock_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection failed" << endl;
        exit(EXIT_FAILURE);
    }

    while (true) {
        cout << "Message to server: ";
        getline(cin, message);
        char* msg = &message[0];

        if (send(sock_fd, msg, strlen(msg), 0) < 0) {
            cout << "Failed to send" << endl;
        }

        valread = read(sock_fd, buffer, LENGTH);

        if (DEBUG) {
            cout << "valread: " << valread << endl;
        }

        if (valread < 0) {
            cout << "Read error" << endl;
        } else if (valread == 0) {
            cout << "EOF" << endl;
        } else {
            // below only works for string literal?
            // cout << "Server response: " << buffer << endl;
            
            cout << "Server response: " << endl;
            print_buffer(buffer, valread);
        }
        clear_buffer(buffer, LENGTH);
        cout << endl;
    }
    
    return 0;
}

void clear_buffer(char buffer[], int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = '\0';
    }
}

void print_buffer(char buffer[], int size) {
    for (int i = 0; i < size; i++) {
        cout << buffer[i];
    }
    cout << endl;
}