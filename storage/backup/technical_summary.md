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
