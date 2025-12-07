#include "client.h"
#include <iostream>
#include <ctime>
#include <sstream>

int main(int argc, char** argv) {
    std::string target_str = "localhost:50051";
    // Generate a random client ID for CRDT
    std::srand(std::time(nullptr));
    std::string client_id = "client_" + std::to_string(std::rand());
    
    filesync::FileSyncClient client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()), client_id);
    
    if (argc > 1) {
        std::string command = argv[1];
        if (command == "upload" && argc > 2) {
            client.UploadFile(argv[2]);
        } else if (command == "download" && argc > 3) {
            client.DownloadFile(argv[2], argv[3]);
        } else if (command == "edit" && argc > 4) {
            // ./filesync_client edit <file> <index> <char>
            client.EditFile(argv[2], std::stoi(argv[3]), argv[4][0]);
        } else if (command == "cat" && argc > 2) {
            // ./filesync_client cat <file>
            client.GetCRDTState(argv[2]);
        } else if (command == "interactive") {
            std::cout << "Entering interactive mode. Commands: upload, download, edit, cat, exit" << std::endl;
            std::string line;
            while (std::cout << "> " && std::getline(std::cin, line)) {
                std::stringstream ss(line);
                std::string cmd;
                ss >> cmd;
                
                if (cmd == "exit") break;
                else if (cmd == "upload") {
                    std::string path;
                    if (ss >> path) client.UploadFile(path);
                } else if (cmd == "download") {
                    std::string name, path;
                    if (ss >> name >> path) client.DownloadFile(name, path);
                } else if (cmd == "edit") {
                    std::string name;
                    int idx;
                    char c;
                    if (ss >> name >> idx >> c) client.EditFile(name, idx, c);
                } else if (cmd == "cat") {
                    std::string name;
                    if (ss >> name) client.GetCRDTState(name);
                } else {
                    std::cout << "Unknown command" << std::endl;
                }
            }
        } else {
            std::cout << "Usage: " << std::endl;
            std::cout << "  ./filesync_client interactive" << std::endl;
            std::cout << "  ./filesync_client upload <file>" << std::endl;
            std::cout << "  ./filesync_client download <file_name> <dest_path>" << std::endl;
            std::cout << "  ./filesync_client edit <file_name> <index> <char>" << std::endl;
            std::cout << "  ./filesync_client cat <file_name>" << std::endl;
        }
    } else {
        std::cout << "Usage: ./filesync_client <command> [args]" << std::endl;
    }
    
    return 0;
}
