# FileSync++: A Distributed, Fault-Tolerant, CRDT-Enhanced, Erasure-Coded File Synchronization System

*FileSync++* is a high-performance, multi-node file synchronization platform designed in **C++** and built on **gRPC microservices**. The system enables reliable file uploads, downloads, metadata/version management, and cross-node propagation, similar to Google Drive or Dropbox — but with two advanced features that traditional file-sync products do not provide:

### **1. CRDT-Based Conflict-Free Merging for Text Files**

Unlike conventional cloud storage systems that resolve concurrent edits using simple last-write-wins (LWW), FileSync++ integrates a **Conflict-Free Replicated Data Type (CRDT)** engine for text documents. This enables **real-time, conflict-free merging** even when multiple clients edit the same file offline. Divergent file states automatically converge without data loss, enabling true collaborative editing.

### **2. Erasure Coding for Storage Efficiency and High Durability**

Instead of maintaining full replicas across storage nodes, FileSync++ supports **Reed–Solomon erasure coding (k data shards + m parity shards)**. Files are split into encoded fragments and distributed across nodes, providing **fault tolerance equivalent to replication** but with **significantly lower storage overhead**. Any subset of *k* fragments can reconstruct the full file, making the system highly durable even under node failures.

FileSync++ also provides:

* Multi-node eventual sync
* Server-to-server metadata/patch propagation
* Chunk-based file streaming
* Metadata tracking via SQLite
* Heartbeat-based failure detection
* Optional user-facing CLI or minimal web UI

This project demonstrates expertise in **distributed systems, C++ systems engineering, consensus-free replication, fault tolerance, CRDT theory, and storage coding**.

---

# 🌟 **Updated System Architecture (with CRDT + Erasure Coding)**

## **Core Components**

1. **Client**

   * Upload/download files via gRPC
   * Compute local diffs and hashes
   * CRDT-enabled editor for text files
   * Chunker + encoder for upload

2. **Server Node**

   * File chunk storage
   * Metadata store (SQLite)
   * Erasure coding module
   * CRDT merge engine (for text)
   * File reconstruction module
   * Server-to-server sync RPC handlers

3. **Sync Manager**

   * Watches metadata changes
   * Broadcasts updates to peer nodes
   * Manages background erasure coding sync jobs
   * Pulls missing shards

4. **Distributed Storage Layer**

   * Stores (k+m) encoded shards
   * Reconstructs files when needed
   * Integrates with metadata DB

5. **Cluster Manager**

   * Tracks server heartbeats
   * Detects node failures
   * Rebalances shards if node dies
