#include "client.h"
#include <iostream>

int main(int argc, char** argv) {
    std::string target_str = "localhost:50051";
    filesync::FileSyncClient client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    
    if (argc > 1) {
        std::string command = argv[1];
        if (command == "upload" && argc > 2) {
            client.UploadFile(argv[2]);
        } else {
            std::cout << "Usage: ./filesync_client upload <file>" << std::endl;
        }
    } else {
        std::cout << "Usage: ./filesync_client <command> [args]" << std::endl;
    }
    
    return 0;
}
