#include "client.h"
// Client implementation logic
#include "../common/utils.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <unordered_map>

namespace filesync {

FileSyncClient::FileSyncClient(std::shared_ptr<grpc::Channel> channel, std::string client_id)
    : stub_(FileSyncService::NewStub(channel)), crdt_stub_(CRDTService::NewStub(channel)), crdt_manager_(client_id) {}

void FileSyncClient::EditFile(const std::string& file_name, int index, char content) {
    // 1. Apply locally
    auto op = crdt_manager_.LocalInsert(file_name, index, content);
    
    // 2. Send to server
    CRDTOperation request;
    request.set_type(CRDTOperation::INSERT);
    request.set_file_name(file_name);
    request.set_site_id(op.id.site_id);
    request.set_clock(op.id.clock);
    request.set_content(std::string(1, content));
    request.set_origin_left_site(op.origin_left.site_id);
    request.set_origin_left_clock(op.origin_left.clock);
    
    CRDTResponse response;
    grpc::ClientContext context;
    
    grpc::Status status = crdt_stub_->ApplyCRDTUpdate(&context, request, &response);
    
    if (status.ok()) {
        std::cout << "Edit applied successfully." << std::endl;
    } else {
        std::cout << "Edit failed: " << status.error_message() << std::endl;
    }
}

void FileSyncClient::GetCRDTState(const std::string& file_name) {
    CRDTStateRequest request;
    request.set_file_name(file_name);
    
    CRDTStateResponse response;
    grpc::ClientContext context;
    
    grpc::Status status = crdt_stub_->GetCRDTState(&context, request, &response);
    
    if (status.ok()) {
        std::cout << "Current File Content: " << response.content() << std::endl;
    } else {
        std::cerr << "Failed to get CRDT state: " << status.error_message() << std::endl;
    }
}

void FileSyncClient::Sync() {
    std::cout << "Starting Sync..." << std::endl;
    
    // 1. Get Server File List
    ListFilesRequest request;
    FileListResponse response;
    grpc::ClientContext context;
    
    grpc::Status status = stub_->ListFiles(&context, request, &response);
    if (!status.ok()) {
        std::cerr << "Sync failed: Could not list server files (" << status.error_message() << ")" << std::endl;
        return;
    }
    
    std::unordered_map<std::string, std::string> server_files;
    for (const auto& file : response.files()) {
        server_files[file.file_name()] = file.file_hash();
    }
    
    // 2. Scan Local Directory
    std::unordered_map<std::string, std::string> local_files;
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().filename().string();
            // Skip hidden files, build dir, and storage dir
            if (name[0] == '.' || name == "build" || name == "storage" || name == "filesync.db") continue;
            
            local_files[name] = utils::CalculateSHA256(name);
        }
    }
    
    // 3. Download Missing/Changed Files from Server
    for (const auto& [name, hash] : server_files) {
        if (local_files.find(name) == local_files.end()) {
            std::cout << "[+] Downloading missing file: " << name << std::endl;
            DownloadFile(name, name);
        } else if (local_files[name] != hash) {
            std::cout << "[*] Updating changed file: " << name << std::endl;
            DownloadFile(name, name);
        }
    }
    
    // 4. Upload New Files to Server
    for (const auto& [name, hash] : local_files) {
        if (server_files.find(name) == server_files.end()) {
            std::cout << "[+] Uploading new file: " << name << std::endl;
            UploadFile(name);
        }
    }
    
    std::cout << "Sync Complete." << std::endl;
}

bool FileSyncClient::UploadFile(const std::string& file_path) {
    std::ifstream infile(file_path, std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }

    // Get file name from path
    std::string file_name = file_path.substr(file_path.find_last_of("/\\") + 1);

    grpc::ClientContext context;
    UploadResponse response;
    std::unique_ptr<grpc::ClientWriter<FileChunk>> writer(stub_->UploadFile(&context, &response));

    const size_t kChunkSize = 1024 * 1024; // 1MB
    std::vector<char> buffer(kChunkSize);
    int32_t chunk_index = 0;
    int64_t total_size = utils::GetFileSize(file_path);

    while (infile.read(buffer.data(), kChunkSize) || infile.gcount() > 0) {
        FileChunk chunk;
        chunk.set_file_name(file_name);
        chunk.set_chunk_index(chunk_index++);
        chunk.set_data(buffer.data(), infile.gcount());
        chunk.set_is_last_chunk(infile.eof());
        if (chunk_index == 1) {
            chunk.set_total_size(total_size);
        }
        
        if (!writer->Write(chunk)) {
            std::cerr << "Broken stream." << std::endl;
            return false;
        }
        
        if (infile.eof()) break;
    }
    
    writer->WritesDone();
    grpc::Status status = writer->Finish();
    
    if (status.ok()) {
        std::cout << "Upload successful: " << response.message() << std::endl;
        return true;
    } else {
        std::cout << "Upload failed: " << status.error_message() << std::endl;
        return false;
    }
}

bool FileSyncClient::DownloadFile(const std::string& file_name, const std::string& dest_path) {
    FileRequest request;
    request.set_file_name(file_name);

    grpc::ClientContext context;
    std::unique_ptr<grpc::ClientReader<FileChunk>> reader(stub_->DownloadFile(&context, request));

    std::ofstream outfile(dest_path, std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open destination file: " << dest_path << std::endl;
        return false;
    }

    FileChunk chunk;
    while (reader->Read(&chunk)) {
        outfile.write(chunk.data().c_str(), chunk.data().length());
        std::cout << "Received chunk " << chunk.chunk_index() << std::endl;
    }

    grpc::Status status = reader->Finish();
    if (status.ok()) {
        std::cout << "Download successful." << std::endl;
        return true;
    } else {
        std::cout << "Download failed: " << status.error_message() << std::endl;
        return false;
    }
}

} // namespace filesync
