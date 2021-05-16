#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "helper_functions.h"
#include "file_functions.h"
#include "directory_functions.h"

#define PORT 8080

using namespace std;


char buffer[LENGTH];
const int MAX_CONNECTIONS = 3;
char* response_back = (char*) "Received by server";


void* thread_connect(void* ptr);


int main(int argc, char* argv[]) {
    // 2 fds, one for server and other for client-connections
    int server_fd, new_socket;

    // holds address info
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    clear_buffer(buffer, LENGTH);
    int opt = 1;
    
    // create socket fd (AF_INET - IPv4; SOCK_STREAM - TCP (reliable, connection oriented))
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Socket failed" << endl;
        exit(EXIT_FAILURE);
    }

    // set socket to port 8080 (prevents "address already in use")
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cout << "setsockopt failed" << endl;
        exit(EXIT_FAILURE);
    }

    // set address details
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;       // ip address of machine running
    address.sin_port = htons(PORT);             // port number in host -> network byte order

    // bind socket to address and port 8080
    if (bind(server_fd, (struct sockaddr*) &address, sizeof(address)) < 0) {
        std::cout << "Bind failed" << endl;
        exit(EXIT_FAILURE);
    }

    // wait for client to try and make a connection (up to 5 - MAX)
    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        std::cout << "Listen failed" << endl;
        exit(EXIT_FAILURE);
    }

    while (true) {
        // set new_socket to fd for connection between client and server
        if ((new_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0) {
            std::cout << "Accept failed" << endl;
            exit(EXIT_FAILURE);
        }

        pthread_t tid;
        pthread_create(&tid, nullptr, &thread_connect, &new_socket);
        pthread_detach(tid);        
    }

    return 0;
}


void* thread_connect(void* ptr) {
    const bool DEBUG = false;
    const bool SHOW_PID = false;
    bool formatted = false;

    // 2d array to hold parent directory and directory name (to be formatted with 'F' command)
    string directories[MAX_DIRS][2];
    int current_dir = 0;

    // maps to hold files of current dir (to be re-initialized with every directory change)
    map<string, int> files;

    // maps to hold dirs of current dir (to be re-initialized with every directory change)
    map<string, int> children;
    
    // open file (disk) using an fd
    const char* file = "directory_system.txt";
    int fd = open(file, O_RDWR);
    if (fd < 0) {
        std::cout << "Error: Can't open " << file << endl;
        pthread_exit(nullptr);
    }

    // get ptr to access virtual memory
    char* c_ptr = (char*) mmap(nullptr, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (c_ptr == MAP_FAILED) {
        std::cout << "Error: Mapping failed" << endl;
        pthread_exit(nullptr);
    }
    close(fd);

    // format filesystem (init map with data from disk)
    format_arrays(directories, MAX_DIRS, c_ptr, current_dir, files, children);

    // get new socket fd
    int socket_fd = *(int*) ptr;

    // loop while connection with a client persists
    while ((read(socket_fd, buffer, LENGTH)) > 0) {
        int response_size = get_message_size(buffer);
        std::cout << "Client message: " << buffer << endl;

        // grab first command (checking if directory command)
        int temp_start = 0;
        string cmd = read_from_buffer(buffer, temp_start);

        // flag to see if buffer contains response from dir cmds
        bool response = false;

        // make directory
        if (cmd == "mkdir") {
            mkdir(buffer, temp_start, directories, current_dir, children);
            response = true;
        }

        // change directory
        else if (cmd == "cd") {
            cd(buffer, temp_start, directories, current_dir, files, children, c_ptr);
            response = true;
        }

        // print working directory
        else if (cmd == "pwd") {
            pwd(current_dir, directories, buffer);
            response = true;
        }

        // remove directory
        else if (cmd == "rmdir") {
            rmdir(buffer, temp_start, children, c_ptr, directories, current_dir);
            response = true;
        }

        char c = toupper(buffer[0]);

        if (!response) {
            // get different commands
            switch (c) {
                // create file
                case 'C':
                    create_file(buffer, current_dir, files, c_ptr);
                    break;

                // delete file
                case 'D':
                    delete_file(buffer, files, c_ptr);
                    break;
                
                // list files
                case 'L':
                    list_files(buffer, files, children, c_ptr);
                    break;

                // read file
                case 'R':
                    read_file(buffer, files, c_ptr);
                    break;

                // write file
                case 'W':
                    write_file(buffer, current_dir, files, c_ptr);
                    break;
                    
                default:
                    clear_buffer(buffer, LENGTH);
                    buffer[0] = '0';
                    break;
            }
        }

        response_size = get_message_size(buffer);

        if (DEBUG) {
            std::cout << "printing buffer" << endl;
            print_buffer(buffer, response_size);
        }

        // send output to client
        if (send(socket_fd, buffer, response_size, 0) < 0) {
            std::cout << "Error: response failed to send" << endl;
        }
        clear_buffer(buffer, LENGTH);
        std::cout << endl;
    }

    // close mmap
    int err = munmap(c_ptr, DISK_SIZE);
    if (err != 0) {
        std::cout << "Error: Unmapping failed" << endl;
        pthread_exit(nullptr);
    }

    // open files.txt to overwrite
    ofstream output_files;
    output_files.open("directories.txt", ios::trunc);
    if (output_files.fail()) {
        std::cout << "Error: Can't open directories.txt" << endl;
        pthread_exit(nullptr);
    }

    // save 2d array to file
    for (int i = 0; i < MAX_DIRS; i++) {
        if (directories[i][0] == "\0") {
            output_files << "\0" << endl;
        } else {
            output_files << directories[i][0] << " " << directories[i][1] << endl;
        }
    }

    output_files.close();

    std::cout << "Exiting thread" << endl;
    pthread_exit(nullptr);
}
