#include "client.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "../common/utils.h"

namespace filesync {

FileSyncClient::FileSyncClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(FileSyncService::NewStub(channel)), crdt_stub_(CRDTService::NewStub(channel)) {}

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
