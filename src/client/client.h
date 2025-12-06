#pragma once

#include <grpcpp/grpcpp.h>
#include "filesync.grpc.pb.h"
#include "crdt.grpc.pb.h"

namespace filesync {

class FileSyncClient {
public:
    FileSyncClient(std::shared_ptr<grpc::Channel> channel);

    bool UploadFile(const std::string& file_path);
    bool DownloadFile(const std::string& file_name, const std::string& dest_path);
    
    // Future methods
    // bool Sync();

private:
    std::unique_ptr<FileSyncService::Stub> stub_;
    std::unique_ptr<CRDTService::Stub> crdt_stub_;
};

} // namespace filesync
