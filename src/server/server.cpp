#include "server.h"
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include "../common/utils.h"

namespace filesync {

FileSyncServiceImpl::FileSyncServiceImpl(DBManager& db) : db_(db) {}

grpc::Status FileSyncServiceImpl::UploadFile(grpc::ServerContext* context, grpc::ServerReader<FileChunk>* reader, UploadResponse* response) {
    FileChunk chunk;
    std::ofstream outfile;
    std::string file_name;
    int64_t total_size = 0;
    bool first_chunk = true;

    while (reader->Read(&chunk)) {
        if (first_chunk) {
            file_name = chunk.file_name();
            // TODO: Use a specific storage directory
            outfile.open(file_name, std::ios::binary);
            if (!outfile.is_open()) {
                return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to open file for writing");
            }
            first_chunk = false;
        }
        outfile.write(chunk.data().c_str(), chunk.data().length());
        total_size += chunk.data().length();
        
        // Track chunk in DB (simplified for now, assuming local storage)
        db_.AddChunk(file_name, chunk.chunk_index(), 0, "localhost");
    }
    
    if (outfile.is_open()) {
        outfile.close();
    }

    // Calculate hash and update DB
    std::string hash = utils::CalculateSHA256(file_name);
    int64_t timestamp = std::time(nullptr);
    db_.AddFile(file_name, hash, total_size, timestamp);

    response->set_success(true);
    response->set_message("Upload successful");
    response->set_file_id(file_name); // Simple ID for now
    
    std::cout << "File uploaded: " << file_name << " Size: " << total_size << " Hash: " << hash << std::endl;
    return grpc::Status::OK;
}

grpc::Status FileSyncServiceImpl::DownloadFile(grpc::ServerContext* context, const FileRequest* request, grpc::ServerWriter<FileChunk>* writer) {
    std::string file_name = request->file_name();
    std::string hash;
    int64_t size, timestamp;
    
    if (!db_.GetFile(file_name, hash, size, timestamp)) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "File not found in metadata");
    }

    std::ifstream infile(file_name, std::ios::binary);
    if (!infile.is_open()) {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to open file for reading");
    }

    const size_t kChunkSize = 1024 * 1024; // 1MB chunks
    std::vector<char> buffer(kChunkSize);
    int32_t chunk_index = 0;

    while (infile.read(buffer.data(), kChunkSize) || infile.gcount() > 0) {
        FileChunk chunk;
        chunk.set_file_name(file_name);
        chunk.set_chunk_index(chunk_index++);
        chunk.set_data(buffer.data(), infile.gcount());
        chunk.set_is_last_chunk(infile.eof());
        if (chunk_index == 1) {
             chunk.set_total_size(size);
             chunk.set_file_hash(hash);
        }
        
        if (!writer->Write(chunk)) {
            return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to write chunk to stream");
        }
        
        if (infile.eof()) break;
    }

    return grpc::Status::OK;
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

CRDTServiceImpl::CRDTServiceImpl() : crdt_manager_("server") {}

grpc::Status CRDTServiceImpl::ApplyCRDTUpdate(grpc::ServerContext* context, const CRDTOperation* request, CRDTResponse* response) {
    std::string file_name = request->file_name();
    
    if (request->type() == CRDTOperation::INSERT) {
        CharID id = {request->site_id(), request->clock()};
        CharID origin_left = {request->origin_left_site(), request->origin_left_clock()};
        char content = request->content()[0];
        
        crdt_manager_.ApplyInsert(file_name, content, id, origin_left);
        std::cout << "Applied Insert: " << content << " from " << request->site_id() << std::endl;
    } else if (request->type() == CRDTOperation::DELETE) {
        CharID target_id = {request->target_site(), request->target_clock()};
        crdt_manager_.ApplyDelete(file_name, target_id);
        std::cout << "Applied Delete from " << request->site_id() << std::endl;
    }
    
    response->set_success(true);
    return grpc::Status::OK;
}

grpc::Status CRDTServiceImpl::GetCRDTState(grpc::ServerContext* context, const CRDTStateRequest* request, CRDTStateResponse* response) {
    std::string text = crdt_manager_.GetText(request->file_name());
    response->set_content(text);
    return grpc::Status::OK;
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
