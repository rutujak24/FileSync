#pragma once
// CRDT manager header

#include <string>
#include <vector>
#include <list>
#include <map>
#include <iostream>

namespace filesync {

// Unique ID for a character in RGA
struct CharID {
    std::string site_id;
    int32_t clock;

    bool operator==(const CharID& other) const {
        return site_id == other.site_id && clock == other.clock;
    }
    
    // Lexicographical comparison for RGA ordering
    bool operator<(const CharID& other) const {
        if (clock != other.clock) return clock < other.clock;
        return site_id < other.site_id;
    }
};

// A character in the text
struct RGANode {
    CharID id;
    char content;
    bool is_deleted;
    CharID origin_left; // The ID of the character to the left when this was inserted
};

class CRDTManager {
public:
    CRDTManager(std::string site_id);

    // Apply a remote operation
    void ApplyInsert(const std::string& file_name, char content, CharID id, CharID origin_left);
    void ApplyDelete(const std::string& file_name, CharID target_id);

    // Generate a local operation (simplified: insert at index)
    // Returns the operation details needed to send to peers
    struct LocalInsertOp {
        char content;
        CharID id;
        CharID origin_left;
    };
    LocalInsertOp LocalInsert(const std::string& file_name, int index, char content);

    // Get the current text content
    std::string GetText(const std::string& file_name);

private:
    std::string site_id_;
    int32_t clock_;
    
    // Map file_name -> List of RGA Nodes
    std::map<std::string, std::list<RGANode>> files_;
    
    // Helper to find a node by ID
    std::list<RGANode>::iterator FindNode(std::list<RGANode>& nodes, CharID id);
};

} // namespace filesync
