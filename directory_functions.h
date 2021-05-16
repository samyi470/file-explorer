#ifndef DIRECTORY_FUNCTIONS_H
#define DIRECTORY_FUNCTIONS_H

#include "file_functions.h"

using namespace std;


void mkdir(char buffer[], int& count, string directories[][2], int current_dir, map<string, int>& children);
void cd(char buffer[], int& count, string directories[][2], int& current_dir, map<string, int>& files, map<string, int>& children, char* ptr);
void pwd(int current_dir, string directories[][2], char buffer[]);
void rmdir(char buffer[], int& count, map<string, int>& children, char* ptr, string directories[][2], int current_dir);
int get_next_dir_index(string directories[][2]);


void mkdir(char buffer[], int& count, string directories[][2], int current_dir, map<string, int>& children) {
    // get new directory name
    skip_whitespace(buffer, count);
    string new_dir = read_from_buffer(buffer, count);

    // if valid
    if (!new_dir.empty()) {
        // find next available spot in directories[][]
        int dir_row = get_next_dir_index(directories);

        // if no new spots are available
        if (dir_row == 0) {
            clear_buffer(buffer, LENGTH);
            string err_msg = "No Space";
            buffer[0] = '1';
            buffer[1] = ' ';
            int current_char = 2;

            write_to_buffer(buffer, err_msg, current_char);
        } else {
            // save to directories
            directories[dir_row][0] = to_string(current_dir);
            directories[dir_row][1] = new_dir;

            reinitialize_children(directories, current_dir, children);
            clear_buffer(buffer, LENGTH);
            buffer[0] = '0';
        }   
    }
}

void cd(char buffer[], int& count, string directories[][2], int& current_dir, map<string, int>& files, map<string, int>& children, char* ptr) {
    const bool DEBUG = false;
    
    // get new directory name
    skip_whitespace(buffer, count);
    string new_dir = read_from_buffer(buffer, count);

    if (DEBUG) {
        cout << "directory: " << new_dir << endl;
    }

    // check if dir exists
    map<string, int>::iterator it;
    it = children.find(new_dir);

    // if invalid dir
    if ((new_dir != ".." && it == children.end()) || (new_dir == ".." && directories[current_dir][1] == "root")) {
        std::cout << "Error: directory doesn't exist" << endl;
        clear_buffer(buffer, LENGTH);
        buffer[0] = '1';
    } else {
        if (new_dir == "..") {
            // set new directory to parent
            current_dir = stoi(directories[current_dir][0]);
        } else {
            // set current directory to new directory
            current_dir = children[new_dir];
        }

        // clear current file and directory maps
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

        clear_buffer(buffer, LENGTH);
        buffer[0] = '0';
    }
}

void pwd(int current_dir, string directories[][2], char buffer[]) {
    const bool DEBUG = false;

    // navigate through directories
    int dir = current_dir;
    string wd = directories[dir][1];

    if (wd != "root") {
        // get parent directory
        dir = stoi(directories[dir][0]);

        // while parent directory isn't root
        while (directories[dir][1] != "root") {
            if (DEBUG) {
                cout << "parent directory: " << directories[dir][1] << endl;
            }
            
            // get parent directory and add current directory to it
            wd = directories[dir][1] + "/" + wd;
            dir = stoi(directories[dir][0]);
        }

        wd = directories[dir][1] + "/" + wd;
    }

    if (DEBUG) {
        cout << "directories: " << wd << endl;
    }

    clear_buffer(buffer, LENGTH);
    int temp_count = 0;
    write_to_buffer(buffer, wd, temp_count);
}

void rmdir(char buffer[], int& count, map<string, int>& children, char* ptr, string directories[][2], int current_dir) {
    const bool DEBUG = false;
    
    // get name of directory to be moved
    skip_whitespace(buffer, count);
    string remove_dir = read_from_buffer(buffer, count);

    if (DEBUG) {
        cout << "checking if directory exists" << endl;
    }

    // check if dir exists
    map<string, int>::iterator it;
    it = children.find(remove_dir);

    if (it == children.end()) {
        std::cout << "Error: directory doesn't exist" << endl;
        clear_buffer(buffer, LENGTH);
        buffer[0] = '1';
    } else {
        // get all files in directory to be removed
        int dir = children[remove_dir];
        map<string, int> child_files;

        reinitialize_files(child_files, ptr, dir);

        if (DEBUG) {
            cout << "printing children files" << endl;
            print_map(child_files);
        }

        // loop through map and erase all files
        for (it = child_files.begin(); it != child_files.end(); it++) {
            int block = child_files[it->first];
            int block_start = get_start_index(block);
            int block_end = get_end_index(block);
            clear_block(ptr, block_start, block_end);
        }

        // clear both maps
        child_files.clear();

        // remove dir from directories
        directories[dir][0] = '\0';
        directories[dir][1] = '\0';

        reinitialize_children(directories, current_dir, children);

        clear_buffer(buffer, LENGTH);
        buffer[0] = '0';
    }
}

int get_next_dir_index(string directories[][2]) {
    for (int i = 1; i < MAX_DIRS; i++) {
        if (directories[i][0] == "\0") {
            return i;
        }
    }

    return 0;
}


#endif // DIRECTORY_FUNCTIONS_H