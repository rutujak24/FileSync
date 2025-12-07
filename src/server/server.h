#pragma once

#include <grpcpp/grpcpp.h>
#include "filesync.grpc.pb.h"
#include "crdt.grpc.pb.h"
#include "../db/db_manager.h"
#include "../common/crdt_manager.h"

namespace filesync {

class FileSyncServiceImpl final : public FileSyncService::Service {
public:
    FileSyncServiceImpl(DBManager& db);
    
    grpc::Status UploadFile(grpc::ServerContext* context, grpc::ServerReader<FileChunk>* reader, UploadResponse* response) override;
    grpc::Status DownloadFile(grpc::ServerContext* context, const FileRequest* request, grpc::ServerWriter<FileChunk>* writer) override;
    grpc::Status PropagateMeta(grpc::ServerContext* context, const MetadataUpdate* request, PropagateResponse* response) override;
    grpc::Status FetchShard(grpc::ServerContext* context, const ShardRequest* request, ShardData* response) override;
    grpc::Status Heartbeat(grpc::ServerContext* context, const HeartbeatRequest* request, HeartbeatResponse* response) override;
    grpc::Status ListFiles(grpc::ServerContext* context, const ListFilesRequest* request, FileListResponse* response) override;

private:
    DBManager& db_;
};

class CRDTServiceImpl final : public CRDTService::Service {
public:
    CRDTServiceImpl();
    grpc::Status ApplyCRDTUpdate(grpc::ServerContext* context, const CRDTOperation* request, CRDTResponse* response) override;
    grpc::Status GetCRDTState(grpc::ServerContext* context, const CRDTStateRequest* request, CRDTStateResponse* response) override;

private:
    CRDTManager crdt_manager_;
};

void RunServer(const std::string& server_address, const std::string& db_path);

} // namespace filesync
