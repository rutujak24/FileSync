# FileSync++: Distributed File Synchronization with CRDTs

**FileSync++** is a distributed file synchronization system built in **C++** using **gRPC**. It functions like a simple version of Dropbox or Google Drive, but with a powerful enhancement: **Real-time Collaborative Editing** using Conflict-Free Replicated Data Types (CRDTs).

## Key Features

### 1. Bidirectional File Sync
Automatically synchronizes files between the client and server.
-   **Smart Sync**: Only transfers files that are missing or changed.
-   **Efficient**: Uses SHA256 hashing to detect changes.

### 2. Real-Time Collaborative Editing (CRDT)
Supports conflict-free text editing. Multiple users can edit the same file, and the system ensures that everyone sees the same final result without merge conflicts.
-   **Algorithm**: Implements **RGA (Replicated Growable Array)**.
-   **Conflict-Free**: Mathematical guarantee of eventual consistency.

### 3. Fault Tolerance (Replication)
Ensures your data is safe even if a disk fails.
-   **Primary-Backup Replication**: Every uploaded file is saved to two separate storage locations (`storage/primary` and `storage/backup`).
-   **Automatic Failover**: If the primary file is lost, the server automatically retrieves it from the backup.

---

## Architecture

1.  **Client**:
    -   Command-line interface (CLI) for user interaction.
    -   Handles file scanning, hashing, and uploading/downloading.
    -   Manages local CRDT state for editing.

2.  **Server**:
    -   gRPC service handling file transfers and metadata.
    -   **SQLite Database**: Stores file metadata (names, hashes, sizes).
    -   **Storage Engine**: Manages dual-write replication.

---

## How to Build and Run

### Prerequisites
-   C++17 Compiler
-   CMake
-   gRPC & Protobuf
-   SQLite3
-   OpenSSL

### Build
```bash
mkdir build && cd build
cmake ..
make -j4
```

### Run Server
```bash
./filesync_server
```

### Run Client
```bash
# Interactive Mode (Recommended)
./filesync_client interactive

# Commands inside interactive mode:
> upload <file_path>
> download <file_name> <dest_path>
> sync
> edit <file_name> <index> <char>
> cat <file_name>
```

---

## Project Structure
-   `src/client/`: Client-side logic and CLI.
-   `src/server/`: Server-side logic and storage management.
-   `src/common/`: Shared utilities (CRDT manager, hashing).
-   `src/db/`: Database management (SQLite).
-   `protos/`: gRPC protocol definitions.
