#include "crdt_manager.h"
// CRDT logic implementation
#include <algorithm>

namespace filesync {

CRDTManager::CRDTManager(std::string site_id) : site_id_(site_id), clock_(0) {}

std::list<RGANode>::iterator CRDTManager::FindNode(std::list<RGANode>& nodes, CharID id) {
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        if (it->id == id) return it;
    }
    return nodes.end();
}

void CRDTManager::ApplyInsert(const std::string& file_name, char content, CharID id, CharID origin_left) {
    auto& nodes = files_[file_name];
    
    // Check if already applied (idempotency)
    if (FindNode(nodes, id) != nodes.end()) return;

    // Find insertion point
    auto it = nodes.begin();
    
    // If origin_left is empty (start of file), start at beginning
    // Otherwise, find the origin_left node
    if (origin_left.clock != 0 || !origin_left.site_id.empty()) {
        auto left_it = FindNode(nodes, origin_left);
        if (left_it != nodes.end()) {
            it = std::next(left_it);
        }
    }

    // RGA Rule: Skip over nodes that have a greater ID (concurrently inserted to the right)
    // We keep skipping as long as the current node's origin_left is the same as ours, 
    // AND the current node's ID is greater than ours.
    // Actually, simplified RGA: Skip nodes that are to the right of our origin_left 
    // but have a higher priority (ID > our ID).
    
    // RGA Rule: Skip over nodes that have a greater ID (concurrently inserted to the right)
    // We keep skipping as long as the current node's origin_left is the same as ours, 
    // AND the current node's ID is greater than ours.
    // Actually, simplified RGA: Skip nodes that are to the right of our origin_left 
    // but have a higher priority (ID > our ID).
    
    // Let's try to be robust:
    it = nodes.begin();
    if (origin_left.clock != 0 || !origin_left.site_id.empty()) {
        auto left_it = FindNode(nodes, origin_left);
        if (left_it != nodes.end()) {
            it = std::next(left_it);
        }
    }
    
    while (it != nodes.end() && it->origin_left == origin_left && it->id < id) {
         ++it;
    }
    
    RGANode node;
    node.id = id;
    node.content = content;
    node.is_deleted = false;
    node.origin_left = origin_left;
    
    nodes.insert(it, node);
    
    // Update logic clock
    if (id.clock > clock_) clock_ = id.clock;
}

void CRDTManager::ApplyDelete(const std::string& file_name, CharID target_id) {
    auto& nodes = files_[file_name];
    auto it = FindNode(nodes, target_id);
    if (it != nodes.end()) {
        it->is_deleted = true;
    }
}

CRDTManager::LocalInsertOp CRDTManager::LocalInsert(const std::string& file_name, int index, char content) {
    auto& nodes = files_[file_name];
    clock_++;
    
    CharID id;
    id.site_id = site_id_;
    id.clock = clock_;
    
    CharID origin_left;
    origin_left.clock = 0; // Default (start of file)
    
    // Find the node at 'index' to determine origin_left
    // We need to skip deleted nodes to find the visual index
    int current_visual_index = 0;
    auto it = nodes.begin();
    auto prev_it = nodes.end();
    
    while (it != nodes.end()) {
        if (!it->is_deleted) {
            if (current_visual_index == index) {
                // Insert before this node. So origin_left is the node BEFORE 'it'.
                break;
            }
            current_visual_index++;
        }
        prev_it = it;
        ++it;
    }
    
    if (prev_it != nodes.end()) {
        origin_left = prev_it->id;
    } else if (it != nodes.begin()) {
         // If we are at end, prev_it should be the last node
         // Wait, the loop logic is a bit off for "append".
         // Let's simplify:
         // If index is 0, origin_left is empty/null.
         // If index > 0, find the (index-1)-th visible character. That is our origin_left.
    }
    
    // Correct logic for finding origin_left based on visual index:
    if (index > 0) {
        int visible_count = 0;
        for (auto& node : nodes) {
            if (!node.is_deleted) {
                visible_count++;
                if (visible_count == index) {
                    origin_left = node.id;
                    break;
                }
            }
        }
    }
    
    // Apply locally
    ApplyInsert(file_name, content, id, origin_left);
    
    return {content, id, origin_left};
}

std::string CRDTManager::GetText(const std::string& file_name) {
    std::string text;
    if (files_.find(file_name) == files_.end()) return "";
    
    for (const auto& node : files_[file_name]) {
        if (!node.is_deleted) {
            text += node.content;
        }
    }
    return text;
}

} // namespace filesync
