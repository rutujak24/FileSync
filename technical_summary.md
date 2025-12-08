# FileSync++: Technical Documentation (Day 1-3)

## 1. Project Overview
**FileSync++** is a distributed file synchronization system built in C++. It uses **gRPC** for communication, **SQLite** for metadata management, and a custom **CRDT (Conflict-Free Replicated Data Type)** engine for real-time collaborative text editing.

## 2. System Architecture
The system follows a **Client-Server** model (evolving into Multi-Node P2P).
-   **Communication**: gRPC (HTTP/2 + Protobuf).
-   **Storage**: Local disk for file blobs, SQLite for metadata.
-   **Security**: SHA256 hashing for data integrity.
-   **Concurrency**: RGA (Replicated Growable Array) for text merging.

## 3. Implementation Details

### Day 1: Foundation & Protocols
We established the communication contracts using Protocol Buffers.
-   **`protos/filesync.proto`**: Defines core RPCs.
    -   `UploadFile`: Streaming RPC for sending file chunks.
    -   `DownloadFile`: Streaming RPC for receiving file chunks.
-   **`protos/crdt.proto`**: Defines collaborative editing RPCs.
    -   `ApplyCRDTUpdate`: Sends character insertions/deletions.
    -   `GetCRDTState`: Retrieves current document state.

### Day 2: Core File Transfer
We implemented robust file upload and download capabilities.
-   **Chunking Strategy**: Files are split into **1MB chunks** (`kChunkSize = 1024 * 1024`). This prevents memory overload for large files.
-   **Data Integrity**:
    -   **SHA256 Hashing**: Implemented in `src/common/utils.cpp` using OpenSSL. Every uploaded file is hashed, and the hash is stored in the DB.
-   **Metadata Storage**:
    -   **SQLite Schema** (`src/db/db_manager.cpp`):
        -   `files`: Stores `name`, `hash`, `size`, `timestamp`.
        -   `chunks`: Maps file chunks to storage nodes (preparation for distributed storage).

### Day 4: Simple Replication & Failover
We implemented a fault-tolerant storage layer using **Simple Replication**.
-   **Dual-Write Strategy**:
    -   `UploadFile` writes the incoming stream to both `storage/primary/` and `storage/backup/`.
    -   This ensures that we always have two copies of every file.
-   **Failover Mechanism**:
    -   `DownloadFile` attempts to read from `storage/primary/` first.
    -   If the primary file is missing or corrupted, it catches the error and seamlessly switches to reading from `storage/backup/`.
    -   This provides **High Availability** against single-disk failures.

## 4. Key Source Files
-   **`src/server/server.cpp`**: The brain of the operation. Handles gRPC requests, writes files to disk (primary & backup), and updates the SQLite DB.
-   **`src/client/client.cpp`**: The user interface. Reads local files, streams them to the server, and handles user input for editing.
-   **`src/common/crdt_manager.cpp`**: The algorithmic core. Contains the logic for `ApplyInsert` and `ApplyDelete` using RGA rules.
-   **`src/db/db_manager.cpp`**: The persistence layer. Wraps SQLite C API for safe SQL execution.

## 5. Demo Guide (How to Verify)

### Test 1: File Upload/Download & Failover
1.  **Start Server**: `./build/filesync_server`
2.  **Upload**: `./build/filesync_client upload test_data.bin`
    -   *Check*: File exists in `storage/primary/` AND `storage/backup/`.
3.  **Simulate Failure**: `rm storage/primary/test_data.bin`
4.  **Download**: `./build/filesync_client download test_data.bin restored.bin`
    -   *Check*: Server logs "Recovered ... from Backup". Download succeeds.

### Test 2: Collaborative Editing (CRDT)
1.  **Start Server**: `./build/filesync_server`
2.  **Start Client**: `./build/filesync_client interactive`
3.  **Simulate Typing**:
    ```bash
    > edit notes.txt 0 H
    > edit notes.txt 1 i
    > cat notes.txt
    ```
    -   *Result*: `Hi`
4.  **Concurrent Edit Simulation** (requires 2 terminals):
    -   Client A: `edit shared.txt 0 A`
    -   Client B: `edit shared.txt 0 B`
    -   *Result*: Both will converge to `AB` or `BA` depending on ID sorting, but **both will be identical**.

## 6. Detailed Architecture & Code Path Analysis

### Control Flow & Code Path

The system uses **gRPC** for communication between Client and Server. The main entry point for the server is `src/server/main.cpp` (which calls `RunServer` in `server.cpp`).

#### **File Upload Flow (`UploadFile`)**
1.  **Client** sends a stream of `FileChunk`s.
2.  **Server** (`FileSyncServiceImpl::UploadFile` in `src/server/server.cpp`):
    -   Receives the first chunk to get the filename.
    -   Opens **two** file streams: one to `storage/primary/` and one to `storage/backup/`.
    -   Writes incoming data to **both** locations synchronously.
    -   Updates **SQLite** (`DBManager::AddChunk`) to track chunk metadata.
    -   Once finished, calculates the SHA256 hash of the primary file and updates the file metadata in SQLite (`DBManager::AddFile`).

#### **File Download Flow (`DownloadFile`)**
1.  **Client** requests a file by name.
2.  **Server** (`FileSyncServiceImpl::DownloadFile`):
    -   Checks SQLite for file existence.
    -   **Primary Path**: Attempts to open `storage/primary/<filename>`.
    -   **Failover Path**: If primary fails, it logs a warning and attempts to open `storage/backup/<filename>`.
    -   Reads the file in 1MB chunks and streams them back to the client.

### CRDT & Realtime Editing Support

The project uses **RGA (Replicated Growable Array)** to support realtime collaborative editing. This is implemented in `src/common/crdt_manager.h` and `.cpp`.

-   **Data Structure**: `std::list<RGANode>`. Each node represents a character and contains:
    -   `CharID`: A unique ID composed of `{site_id, clock}`.
    -   `origin_left`: The ID of the character that was to the immediate left when this character was inserted.
    -   `is_deleted`: A tombstone flag.

-   **Insertion Logic (`ApplyInsert`)**:
    1.  Uses `origin_left` to find the intended insertion position.
    2.  **Conflict Resolution**: If multiple users insert at the same position (same `origin_left`), it uses the `CharID` to break ties. It skips over nodes that have a higher priority (lexicographically larger `CharID`) to ensure all clients converge to the same order.

-   **Deletion Logic (`ApplyDelete`)**:
    -   Does **not** remove the node from the list.
    -   Sets `is_deleted = true` (Tombstone). This ensures that future concurrent operations can still reference this node as an `origin_left`.

-   **Realtime Support**:
    -   The `CRDTServiceImpl` exposes `ApplyCRDTUpdate` via gRPC.
    -   When a user types, the client sends an `INSERT` operation.
    -   The server applies it to its in-memory `CRDTManager` and acknowledges success.

### Fault Tolerance (Primary/Backup)

Fault tolerance is achieved through **Synchronous Replication** and **Read-Repair/Failover**.

-   **Replication**: In `UploadFile`, every chunk is written to `storage/backup/` immediately after being written to `storage/primary/`.
    ```cpp
    // src/server/server.cpp:46
    if (outfile_backup.is_open()) {
        outfile_backup.write(chunk.data().c_str(), chunk.data().length());
    }
    ```
-   **Failover**: In `DownloadFile`, if the primary file cannot be opened, the system automatically switches to the backup.
    ```cpp
    // src/server/server.cpp:89
    if (!infile.is_open()) {
        // ... Log warning ...
        file_path = "storage/backup/" + file_name;
        infile.open(file_path, std::ios::binary);
    }
    ```

### High-Level (HLD) & Low-Level Design (LLD)

#### **HLD (High-Level Design)**
-   **Architecture**: Client-Server using gRPC.
-   **Storage Layer**:
    -   **Metadata**: SQLite Database (`filesync.db`) stores file names, hashes, sizes, and chunk mapping.
    -   **Blob Storage**: Local filesystem with a Primary/Backup folder structure for redundancy.
-   **Communication**: gRPC with Protobuf for structured data exchange.

#### **LLD (Low-Level Design)**
-   **`FileSyncServiceImpl`**: Implements the `FileSyncService` gRPC interface. Handles file I/O and interacts with `DBManager`.
-   **`CRDTServiceImpl`**: Implements the `CRDTService` gRPC interface. Wraps `CRDTManager`.
-   **`CRDTManager`**: Pure C++ logic for RGA algorithm. Manages the linked list of characters and handles `ApplyInsert`/`ApplyDelete`.
-   **`DBManager`**: Abstraction over `sqlite3`. Handles SQL queries for metadata.

### CMake & OpenSSL Usage

-   **CMake (`CMakeLists.txt`)**:
    -   **Build System**: Configures the project, finds dependencies (`find_package`), and defines executables.
    -   **Code Generation**: Uses `protoc` and `grpc_cpp_plugin` to generate C++ code from `.proto` files automatically.
    -   **Linking**: Links the `OpenSSL::SSL` and `OpenSSL::Crypto` libraries to the server and client executables.

-   **OpenSSL**:
    -   **Hashing**: Used in `src/common/utils.cpp` inside `CalculateSHA256`. It uses `<openssl/sha.h>` to compute the SHA256 hash of uploaded files for integrity verification.
    -   **Transport Security**: While the current server code uses `grpc::InsecureServerCredentials()`, the OpenSSL libraries are linked, allowing for easy upgrade to `grpc::SslServerCredentials()` for encrypted transport.
