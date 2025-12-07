#include "client.h"
#include <iostream>
#include <fstream>

namespace filesync {

FileSyncClient::FileSyncClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(FileSyncService::NewStub(channel)), crdt_stub_(CRDTService::NewStub(channel)) {}

bool FileSyncClient::UploadFile(const std::string& file_path) {
    grpc::ClientContext context;
    UploadResponse response;
    std::unique_ptr<grpc::ClientWriter<FileChunk>> writer(stub_->UploadFile(&context, &response));

    // Stub: Send one fake chunk
    FileChunk chunk;
    chunk.set_file_name("test.txt");
    chunk.set_chunk_index(0);
    chunk.set_data("Hello World");
    chunk.set_is_last_chunk(true);
    
    if (!writer->Write(chunk)) {
        return false;
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
    return false; // Not implemented
}

} // namespace filesync
