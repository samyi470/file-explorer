#include <iostream>
#include <fstream>

using namespace std;


void init_disk(string file_name);


int main() {
    // BasicDiskStorageServer
    init_disk("test.txt");


    // FileSystemServer
    ofstream input;
    input.open("files.txt");
    if (input.fail()) {
        cout << "Error: Can't open files.txt" << endl;
    }
    input.close();

    init_disk("file_system.txt");


    // DirectoryStructureServer
    string directories[20][2];
    
    // initialize
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 2; j++) {
            directories[i][j] = "\0";
        }
    }

    directories[0][0] = "null";
    directories[0][1] = "root";

    // save to file
    ofstream output_files;
    output_files.open("directories.txt", ios::trunc);
    if (output_files.fail()) {
        cout << "Error: Can't open directories.txt" << endl;
    }

    for (int i = 0; i < 20; i++) {
        if (directories[i][0] == "\0") {
            output_files << "\0" << endl;
        } else {
            output_files << directories[i][0] << " " << directories[i][1] << endl;
        }
    }

    output_files.close();

    init_disk("directory_system.txt");
}


void init_disk(string file_name) {
    ofstream input;
    input.open(file_name);
    int disk_size = 128 * 140 * 56;
    for (int i = 0; i < disk_size; i++) {
        input << '\0';
    }

    input.close();
}