#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <iostream>
#include <unistd.h>
#include <map>
#include <string.h>

using namespace std;


const int INFO_SIZE = 128;
const int CYLINDERS = 140;
const int SECTORS = 56;
const int BLOCKS = CYLINDERS * SECTORS;
const int DISK_SIZE = INFO_SIZE * BLOCKS;


void print_map(map<string, int>& some_map);
void print_directories(string directories[][2], int rows);
int get_block(int cylinder, int sector);
void write_to_buffer(char buffer[], string number, int &count);
bool is_whitespace(char character);
void skip_whitespace(char buffer[], int& count);
string read_from_buffer(char buffer[], int& count);
string read_all_from_buffer(char buffer[], int& count);
int get_start_index(int block);
int get_end_index(int block);
void read_from_disk(char buffer[], int& count, char* ptr, int start, int end);
void clear_block(char* ptr, int start, int end);
void write_to_disk(char buffer[], int& count, char* ptr, int start);
int get_message_size(char buffer[]);
void clear_buffer(char buffer[], int size);
void print_buffer(char buffer[], int size);


void print_map(map<string, int>& some_map) {
    map<string, int>::iterator it;
    for (it = some_map.begin(); it != some_map.end(); it++) {
        std::cout << it->first << " ";
    }
    std::cout << endl;
}

void print_directories(string directories[][2], int rows) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 2; j++) {
            if (directories[i][j] == "\0") {
                std::cout << "/0 ";
            } else {
                std::cout << directories[i][j] << " ";
            }
        }
        std::cout << endl;
    }
    std::cout << endl;
}

void write_to_buffer(char buffer[], string number, int& count) {
    for (int i = 0; i < number.size(); i++) {
        buffer[count++] = number.at(i);
    }
}

bool is_whitespace(char character) {
    return (character == ' ' || character == '\t' || character == '\n');
}

void skip_whitespace(char buffer[], int& count) {
    while (is_whitespace(buffer[count])) {
        count++;
    }
}

string read_from_buffer(char buffer[], int& count) {
    string number = "";

    // loop while character isn't whitespace and != '\0'
    for (; (!is_whitespace(buffer[count]) && buffer[count] != '\0'); count++) {
        number += buffer[count];
    }

    return number;
}

string read_all_from_buffer(char buffer[], int& count) {
    string data = "";

    // loop while character isn't '\0'
    for (; buffer[count] != '\0'; count++) {
        data += buffer[count];
    }

    return data;
}

int get_start_index(int block) {
    return block * 128;
}

int get_end_index(int block) {
    return (block + 1) * 128 - 1;
}

void read_from_disk(char buffer[], int& count, char* ptr, int start, int end) {
    for (int i = start; i <= end; i++) {
        // break loop if at end of stored data in block
        if (ptr[i] == '\0') {
            break;
        }

        buffer[count++] = ptr[i];
    }
}

void clear_block(char* ptr, int start, int end) {
    for (int i = start; i <= end; i++) {
        ptr[i] = '\0';
    }
}

void write_to_disk(char buffer[], int& count, char* ptr, int start) {    
    // loop while there are chars to write in buffer
    for (int i = start; buffer[count] != '\0'; count++) {
        ptr[i++] = buffer[count];
    }
}

int get_block(int cylinder, int sector) {
    // check if out of lower and upper bounds
    if (cylinder < 0 || cylinder >= CYLINDERS || sector < 0 || sector >= SECTORS) {
        return -1;
    }

    // simulate track-to-track time
    usleep(3 * cylinder);
    usleep(sector);

    return cylinder * SECTORS + sector;
}

int get_message_size(char buffer[]) {
    int i = 0;

    while (buffer[i] != '\0') {
        i++;
    }

    return (i + 1);
}

void clear_buffer(char buffer[], int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = '\0';
    }
}

void print_buffer(char buffer[], int size) {
    for (int i = 0; i < size; i++) {
        std::cout << buffer[i];
    }
    std::cout << endl;
}


#endif // HELPER_FUNCTIONS_H