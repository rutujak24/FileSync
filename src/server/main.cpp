#include "server.h"

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50051");
    std::string db_path("filesync.db");
    
    filesync::RunServer(server_address, db_path);
    
    return 0;
}
