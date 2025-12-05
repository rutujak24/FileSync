# FileSync
Distributed File Synchronization System

Goal: Sync files across multiple nodes with versioning, conflict resolution, and fault tolerance.

Architecture Overview

Client-Server Model:

Server Nodes: Store file replicas, manage metadata, handle sync requests.

Clients: Upload/download files.

Communication: gRPC or ZeroMQ (C++).

Persistence: Local file system + JSON/SQLite metadata.

Features:

File versioning and conflict resolution.

Automatic re-sync if server/client restarts.

Heartbeat/Node health check for fault tolerance.


### 10-Day Plan

Day 1 – System Design & Architecture

Define module diagram: Client, Server, Sync Manager, Metadata DB.

Choose communication protocol (gRPC/ZeroMQ).

Define file versioning schema.

Day 2 – Setup Project & Core Framework

Setup C++ project structure with CMake.

Implement basic Client ↔ Server connection.

Test simple file upload/download.

Day 3 – File Metadata Management

Implement metadata DB (JSON/SQLite).

Track file versions, timestamps, and last-modified.

Day 4 – Conflict Detection & Resolution

Detect conflicts when multiple clients modify the same file.

Implement resolution strategies: last-write-wins, manual merge logs.

Day 5 – Multi-Node Sync

Add multiple servers with replication.

Implement heartbeat mechanism for server availability.

Day 6 – File Transfer Optimization

Use chunked file transfer for large files.

Add optional compression.

Day 7 – Fault Tolerance & Recovery

Auto-resync after client/server crash.

Test server failover and replication.

Day 8 – Minimal Frontend

CLI or lightweight Qt/ImGui GUI to:

Show sync status, upload/download files, resolve conflicts.

Day 9 – Hosting & Testing

Setup Docker for server nodes.

Test distributed setup on multiple local or cloud VMs.

Day 10 – Polish & Documentation

Create GitHub repo with README, diagrams, usage guide.

Add logging, error handling, and example scripts.
