#include "server.h"
#include <iostream>
#include <memory>
#include <string>

namespace filesync {

FileSyncServiceImpl::FileSyncServiceImpl(DBManager& db) : db_(db) {}

grpc::Status FileSyncServiceImpl::UploadFile(grpc::ServerContext* context, grpc::ServerReader<FileChunk>* reader, UploadResponse* response) {
    FileChunk chunk;
    while (reader->Read(&chunk)) {
        // TODO: Store chunk
        std::cout << "Received chunk for file: " << chunk.file_name() << " index: " << chunk.chunk_index() << std::endl;
    }
    response->set_success(true);
    response->set_message("Upload successful (stub)");
    return grpc::Status::OK;
}

grpc::Status FileSyncServiceImpl::DownloadFile(grpc::ServerContext* context, const FileRequest* request, grpc::ServerWriter<FileChunk>* writer) {
    // TODO: Read file and stream chunks
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Not implemented yet");
}

grpc::Status FileSyncServiceImpl::PropagateMeta(grpc::ServerContext* context, const MetadataUpdate* request, PropagateResponse* response) {
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Not implemented yet");
}

grpc::Status FileSyncServiceImpl::FetchShard(grpc::ServerContext* context, const ShardRequest* request, ShardData* response) {
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Not implemented yet");
}

grpc::Status FileSyncServiceImpl::Heartbeat(grpc::ServerContext* context, const HeartbeatRequest* request, HeartbeatResponse* response) {
    response->set_alive(true);
    return grpc::Status::OK;
}

grpc::Status CRDTServiceImpl::ApplyCRDTUpdate(grpc::ServerContext* context, const CRDTUpdate* request, CRDTResponse* response) {
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Not implemented yet");
}

grpc::Status CRDTServiceImpl::GetCRDTState(grpc::ServerContext* context, const CRDTStateRequest* request, CRDTStateResponse* response) {
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Not implemented yet");
}

void RunServer(const std::string& server_address, const std::string& db_path) {
    DBManager db(db_path);
    if (!db.Init()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return;
    }

    FileSyncServiceImpl service(db);
    CRDTServiceImpl crdt_service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    builder.RegisterService(&crdt_service);
    
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

} // namespace filesync
