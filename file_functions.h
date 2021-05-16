#ifndef FILE_FUNCTIONS_H
#define FILE_FUNCTIONS_H

#include <fstream>
#include "helper_functions.h"

using namespace std;


const int LENGTH = 1024;
const int MAX_DIRS = 20;


void reinitialize_files(map<string, int>& files, char* ptr, int current_dir);
void reinitialize_children(string directories[][2], int current_dir, map<string, int>& children);
void format_arrays(string directories[][2], int rows, char* ptr, int current_dir, map<string, int>& files, map<string, int>& children);
void create_file(char buffer[], int current_dir, map<string, int>& files, char* ptr);
void delete_file(char buffer[], map<string, int>& files, char* ptr);
void list_files(char buffer[], map<string, int>& files, map<string, int>& children, char* ptr);
void list_file_size(char buffer[], int& count, map<string, int>::iterator it, char* ptr);
void read_file(char buffer[], map<string, int> files, char* ptr);
void write_file(char buffer[], int current_dir, map<string, int> files, char* ptr);


void reinitialize_files(map<string, int>& files, char* ptr, int current_dir) {
    files.clear();

    // loop through all blocks and get files if part of curr dir
    for (int i = 0; i < BLOCKS; i++) {
        int block_start = get_start_index(i);

        if (ptr[block_start] != '\0') {
            int block_end = get_end_index(i);
            int temp_start = 0;
            char temp_buffer[INFO_SIZE];
            clear_buffer(temp_buffer, INFO_SIZE);
            read_from_disk(temp_buffer, temp_start, ptr, block_start, block_end);

            // get file name and dir
            temp_start = 0;
            string filename = read_from_buffer(temp_buffer, temp_start);
            skip_whitespace(temp_buffer, temp_start);
            string dir = read_from_buffer(temp_buffer, temp_start);

            int f_dir = stoi(dir);
            if (f_dir == current_dir) {
                files.insert(pair<string, int>(filename, i));
            }
        }
    }
}

void reinitialize_children(string directories[][2], int current_dir, map<string, int>& children) {
    // clear children maps
    children.clear();

    // reinitialize map for children
    for (int i = 1; i < MAX_DIRS; i++) {
        if (directories[i][0] == to_string(current_dir)) {
            children.insert(pair<string, int>(directories[i][1], i));
        }
    }
}

void format_arrays(string directories[][2], int rows, char* ptr, int current_dir, map<string, int>& files, map<string, int>& children) {
    const bool DEBUG = false;

    if (DEBUG) {
        std::cout << "formatting" << endl;
    }

    ifstream input_dirs;
    string file_line = "";

    // open file to initialize 2d array
    input_dirs.open("directories.txt");
    if (input_dirs.fail()) {
        std::cout<< "Error: can't open directories.txt" << endl;
    }

    int row = 0;

    // initialize 2d array for directories line-by-line
    while (getline(input_dirs, file_line)) {
        int index = 0;
        char* line_ptr = &file_line[0];
        string parent_index = "";
        string dir_name = "";

        // if not an empty row
        if (file_line[0] != '\0') {
            // grab parent index
            parent_index = read_from_buffer(line_ptr, index);
            skip_whitespace(line_ptr, index);
            dir_name = read_from_buffer(line_ptr, index);

            directories[row][0] = parent_index;
            directories[row][1] = dir_name;
        }

        row++;
    }

    reinitialize_files(files, ptr, current_dir);
    reinitialize_children(directories, current_dir, children);

    if (DEBUG) {
        std::cout << "current dir: " << current_dir << endl;

        std::cout << "printing directories" << endl;
        print_directories(directories, MAX_DIRS);

        std::cout << "print files belonging to dir" << endl;
        print_map(files);

        std::cout << "print dirs belonging to dir" << endl;
        print_map(children);
    }
}

void create_file(char buffer[], int current_dir, map<string, int>& files, char* ptr) {
    const bool DEBUG = false;
    
    // get file name from buffer
    int current_char = 1;
    skip_whitespace(buffer, current_char);
    string new_filename = read_from_buffer(buffer, current_char);

    if (DEBUG) {
        std::cout << "new file name: " << new_filename << endl;
        std::cout << "checking if exists" << endl;
    }

    // check if filename exists
    map<string, int>::iterator it;
    it = files.find(new_filename);

    // if filename exists
    if (it != files.end()) {
        std::cout << "Error: file name exists" << endl;
        clear_buffer(buffer, LENGTH);
        buffer[0] = '1';
        return;
    }

    int empty_block = 0;
    int block_start = 0;
    int block_end = 0;


    // get first available block
    for (int i = 0; i < BLOCKS; i++) {
        int start = get_start_index(i);

        if (ptr[start] == '\0') {
            block_start = start;
            block_end = get_end_index(i);
            break;
        }

        empty_block++;
    }

    if (DEBUG) {
        std::cout << "empty block: " << empty_block << endl;
    }

    // if out of space
    if (empty_block == BLOCKS) {
        std::cout << "Error: out of space" << endl;
        clear_buffer(buffer, LENGTH);
        buffer[0] = '2';
        return;
    }

    // add directory row to buffer
    string directory = to_string(current_dir);
    buffer[current_char++] = ' ';
    for (int i = 0; i < directory.size(); i++) {
        buffer[current_char++] = directory.at(i);
    }

    // add file length to buffer
    buffer[current_char++] = ' ';
    buffer[current_char++] = '0';

    int buffer_start = 1;
    skip_whitespace(buffer, buffer_start);
    write_to_disk(buffer, buffer_start, ptr, block_start);

    // update maps
    files.insert(pair<string, int>(new_filename, empty_block));

    if (DEBUG) {
        std::cout << "command (incl file length 0)" << endl;
        print_buffer(buffer, current_char);
    }

    clear_buffer(buffer, LENGTH);
    buffer[0] = '0';
}

void delete_file(char buffer[], map<string, int>& files, char* ptr) {
    const bool DEBUG = false;
    
    // get filename to delete
    int current_char = 1;
    skip_whitespace(buffer, current_char);
    string to_delete = read_from_buffer(buffer, current_char);

    if (DEBUG) {
        std::cout << "to delete: " << to_delete << endl;
        std::cout << "checking fi exists" << endl;
    }

    // check if filename exists
    map<string, int>::iterator it;
    it = files.find(to_delete);

    // if filename not found
    if (it == files.end()) {
        std::cout << "Error: filename not found" << endl;
        clear_buffer(buffer, LENGTH);
        buffer[0] = '1';
        return;
    }

    int block = files[to_delete];
    int block_start = get_start_index(block);
    int block_end = get_end_index(block);

    if (DEBUG) {
        std::cout << "block to delete: " << files[to_delete] << endl;
        std::cout << "start: " << block_start << endl;
        std::cout << "end: " << block_end << endl;
    }

    // clear data from disk
    clear_block(ptr, block_start, block_end);

    // remove from maps
    files.erase(to_delete);

    clear_buffer(buffer, LENGTH);
    buffer[0] = '0';
}

void list_files(char buffer[], map<string, int>& files, map<string, int>& children, char* ptr) {
    const bool DEBUG = false;

    // get if more info is requested
    int info = 0;
    int current_char = 1;
    skip_whitespace(buffer, current_char);
    string more_info = read_from_buffer(buffer, current_char);
    
    // attempt conversion from string to int
    try {
        info = stoi(more_info);
    } catch (exception &err) {
        std::cout << "Error: stoi conversion" << endl;
        clear_buffer(buffer, LENGTH);
        buffer[0] = '0';
        return;
    }

    // clear entire buffer
    clear_buffer(buffer, LENGTH);
    int write_char = 0;

    // write file names to buffer
    map<string, int>::iterator it;
    for (it = files.begin(); it != files.end(); it++) {
        string filename = it->first;
        write_to_buffer(buffer, filename, write_char);

        // add file size if requested
        if (info == 1) {
            list_file_size(buffer, write_char, it, ptr);
        }

        // add '\n'
        buffer[write_char++] = '\n';
    }

    // write directory names to buffer
    for (it = children.begin(); it != children.end(); it++) {
        string dirname = it->first;
        write_to_buffer(buffer, dirname, write_char);
        buffer[write_char++] = '\n';
    }
}

void list_file_size(char buffer[], int& count, map<string, int>::iterator it, char* ptr) {
    buffer[count++] = ' ';

    // get data length for block
    int block = it->second;
    int block_start = get_start_index(block);
    int block_end = get_end_index(block);

    // store block in temp_buffer
    char temp_buffer[INFO_SIZE];
    clear_buffer(temp_buffer, INFO_SIZE);
    int temp_start = 0;
    read_from_disk(temp_buffer, temp_start, ptr, block_start, block_end);

    // skip file name and directory row
    temp_start = 0;
    read_from_buffer(temp_buffer, temp_start);
    skip_whitespace(temp_buffer, temp_start);
    read_from_buffer(temp_buffer, temp_start);
    skip_whitespace(temp_buffer, temp_start);
    string file_length = read_from_buffer(temp_buffer, temp_start);
    int f_length = stoi(file_length);

    // add size to buffer
    write_to_buffer(buffer, file_length, count);
}

void read_file(char buffer[], map<string, int> files, char* ptr) {
    const bool DEBUG = false;

    // get file name
    int current_char = 1;
    skip_whitespace(buffer, current_char);
    string file_name = read_from_buffer(buffer, current_char);

    if (DEBUG) {
        std::cout << "to read: " << file_name << endl;
        std::cout << "checking if exists" << endl;
    }
    
    // check if file name exists
    map<string, int>::iterator it;
    it = files.find(file_name);

    // if file name not found
    if (it == files.end()) {
        std::cout << "Error: file not found" << endl;
        clear_buffer(buffer, LENGTH);
        buffer[0] = '1';
        return;
    }

    int block = files[file_name];
    int block_start = get_start_index(block);
    int block_end = get_end_index(block);

    if (DEBUG) {
        std::cout << "block to read: " << files[file_name] << endl;
        std::cout << "start: " << block_start << endl;
        std::cout << "end: " << block_end << endl;
    }

    // store block in temp_buffer
    int temp_start = 0;
    char temp_buffer[INFO_SIZE];
    clear_buffer(temp_buffer, INFO_SIZE);
    read_from_disk(temp_buffer, temp_start, ptr, block_start, block_end);

    // skip file name and directory row
    temp_start = 0;
    read_from_buffer(temp_buffer, temp_start);
    skip_whitespace(temp_buffer, temp_start);
    read_from_buffer(temp_buffer, temp_start);
    skip_whitespace(temp_buffer, temp_start);

    // get file length
    string file_length = read_from_buffer(temp_buffer, temp_start);
    string file_content = "";
    int f_length = stoi(file_length);

    // if file has content, get all content
    if (f_length > 0) {
        skip_whitespace(temp_buffer, temp_start);
        file_content = read_all_from_buffer(temp_buffer, temp_start);
    }

    if (DEBUG) {
        std::cout << "file length: " << file_length << endl;
        std::cout << "file content: " << file_content << endl;
        std::cout << "temp buffer: " << endl;
        print_buffer(temp_buffer, temp_start);
    }

    // write file content to buffer
    clear_buffer(buffer, LENGTH);
    buffer[0] = '0';
    buffer[1] = ' ';
    temp_start = 2;
    write_to_buffer(buffer, file_length, temp_start);

    // if file has content, write all content
    if (f_length > 0) {
        buffer[temp_start++] = ' ';
        write_to_buffer(buffer, file_content, temp_start);
    }
}

void write_file(char buffer[], int current_dir, map<string, int> files, char* ptr) {
    const bool DEBUG = false;

    // get file name
    int current_char = 1;
    skip_whitespace(buffer, current_char);
    string filename = read_from_buffer(buffer, current_char);

    if (DEBUG) {
        std::cout << "writing to: " << filename << endl;
        std::cout << "checking if exists" << endl;
    }

    // check if file name exists
    map<string, int>::iterator it;
    it = files.find(filename);

    // if filename not found
    if (it == files.end()) {
        std::cout << "Error: file not found" << endl;
        clear_buffer(buffer, LENGTH);
        buffer[0] = '1';
        return;
    }

    // get file length
    skip_whitespace(buffer, current_char);
    string file_length = read_from_buffer(buffer, current_char);
    int f_length = 0;

    // attempt conversion from string to int
    try {
        f_length = stoi(file_length);
    } catch (exception& err) {
        std::cout << "Error: stoi conversion" << endl;
        clear_buffer(buffer, LENGTH);
        buffer[0] = '2';
        return;
    }

    string file_content = "";
    skip_whitespace(buffer, current_char);
    file_content = read_all_from_buffer(buffer, current_char);

    // get block
    int block = files[filename];
    int block_start = get_start_index(block);
    int block_end = get_end_index(block);
    clear_block(ptr, block_start, block_end);

    if (DEBUG) {
        std::cout << "block to delete: " << files[filename] << endl;
        std::cout << "start: " << block_start << endl;
        std::cout << "end: " << block_end << endl;
        print_buffer(buffer, current_char);
    }

    // clear buffer and rewrite with filename dir length data
    clear_buffer(buffer, LENGTH);
    int temp_start = 0;
    write_to_buffer(buffer, filename, temp_start);
    buffer[temp_start++] = ' ';
    write_to_buffer(buffer, to_string(current_dir), temp_start);
    buffer[temp_start++] = ' ';
    write_to_buffer(buffer, file_length, temp_start);

    // if file has content, write all content
    if (f_length > 0) {
        buffer[temp_start++] = ' ';
        write_to_buffer(buffer, file_content, temp_start);
    }

    temp_start = 0;
    write_to_disk(buffer, temp_start, ptr, block_start);

    clear_buffer(buffer, LENGTH);
    buffer[0] = '0';
}


#endif // FILE_FUNCTIONS_H