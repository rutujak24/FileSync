#pragma once

#include <string>
#include <sqlite3.h>
#include <vector>

namespace filesync {

class DBManager {
public:
    DBManager(const std::string& db_path);
    ~DBManager();

    bool Init();
    bool Execute(const std::string& sql);
    
    // Future methods for metadata operations
    // bool AddFile(const std::string& name, const std::string& hash, int64_t size);
    // ...

private:
    std::string db_path_;
    sqlite3* db_;
};

} // namespace filesync
