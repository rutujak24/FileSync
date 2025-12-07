#pragma once

#include <grpcpp/grpcpp.h>
#include "filesync.grpc.pb.h"
#include "crdt.grpc.pb.h"

#include "../common/crdt_manager.h"

namespace filesync {

class FileSyncClient {
public:
    FileSyncClient(std::shared_ptr<grpc::Channel> channel, std::string client_id);

    bool UploadFile(const std::string& file_path);
    bool DownloadFile(const std::string& file_name, const std::string& dest_path);
    
    // CRDT Operations
    bool EditFile(const std::string& file_name, int index, char content);
    bool GetCRDTState(const std::string& file_name);

private:
    std::unique_ptr<FileSyncService::Stub> stub_;
    std::unique_ptr<CRDTService::Stub> crdt_stub_;
    CRDTManager crdt_manager_;
};

} // namespace filesync
