#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
    struct Node {
        T data;
        Node *left;
        Node *right;
        int npl;  // null path length
        
        Node(const T &value) : data(value), left(nullptr), right(nullptr), npl(0) {}
    };
    
    Node *root;
    size_t current_size;
    Compare comp;
    
    // Helper functions
    int npl(Node *node) const {
        return node ? node->npl : -1;
    }
    
    Node* merge_nodes(Node *h1, Node *h2) {
        if (!h1) return h2;
        if (!h2) return h1;
        
        // Ensure h1 has the larger (or equal) root
        if (comp(h1->data, h2->data)) {
            Node *temp = h1;
            h1 = h2;
            h2 = temp;
        }
        
        // Merge h1's right subtree with h2
        h1->right = merge_nodes(h1->right, h2);
        
        // Maintain leftist heap property: left subtree npl >= right subtree npl
        if (npl(h1->left) < npl(h1->right)) {
            Node *temp = h1->left;
            h1->left = h1->right;
            h1->right = temp;
        }
        
        // Update npl
        h1->npl = (h1->right ? h1->right->npl : 0) + 1;
        
        return h1;
    }
    
    Node* copy_node(Node *node) {
        if (!node) return nullptr;
        
        Node *new_node = nullptr;
        try {
            new_node = new Node(node->data);
            new_node->left = copy_node(node->left);
            new_node->right = copy_node(node->right);
            new_node->npl = node->npl;
        } catch (...) {
            delete_node(new_node);
            throw;
        }
        
        return new_node;
    }
    
    void delete_node(Node *node) {
        if (!node) return;
        delete_node(node->left);
        delete_node(node->right);
        delete node;
    }
    
public:
    /**
     * @brief default constructor
     */
    priority_queue() : root(nullptr), current_size(0) {}

    /**
     * @brief copy constructor
     * @param other the priority_queue to be copied
     */
    priority_queue(const priority_queue &other) : root(nullptr), current_size(0), comp(other.comp) {
        root = copy_node(other.root);
        current_size = other.current_size;
    }

    /**
     * @brief deconstructor
     */
    ~priority_queue() {
        delete_node(root);
    }

    /**
     * @brief Assignment operator
     * @param other the priority_queue to be assigned from
     * @return a reference to this priority_queue after assignment
     */
    priority_queue &operator=(const priority_queue &other) {
        if (this == &other) return *this;
        
        // Create a copy first for exception safety
        Node *new_root = copy_node(other.root);
        size_t new_size = other.current_size;
        Compare new_comp = other.comp;
        
        // If copy succeeded, delete old data and assign new data
        delete_node(root);
        root = new_root;
        current_size = new_size;
        comp = new_comp;
        
        return *this;
    }

    /**
     * @brief get the top element of the priority queue.
     * @return a reference of the top element.
     * @throws container_is_empty if empty() returns true
     */
    const T & top() const {
        if (empty()) {
            throw container_is_empty();
        }
        return root->data;
    }

    /**
     * @brief push new element to the priority queue.
     * @param e the element to be pushed
     */
    void push(const T &e) {
        Node *new_node = nullptr;
        try {
            new_node = new Node(e);
        } catch (...) {
            throw;
        }
        
        Node *new_root = nullptr;
        try {
            new_root = merge_nodes(root, new_node);
        } catch (...) {
            delete new_node;
            throw;
        }
        
        root = new_root;
        current_size++;
    }

    /**
     * @brief delete the top element from the priority queue.
     * @throws container_is_empty if empty() returns true
     */
    void pop() {
        if (empty()) {
            throw container_is_empty();
        }
        
        Node *left_child = root->left;
        Node *right_child = root->right;
        Node *old_root = root;
        
        try {
            root = merge_nodes(left_child, right_child);
        } catch (...) {
            // Restore original state if merge fails
            root = old_root;
            throw;
        }
        
        delete old_root;
        current_size--;
    }

    /**
     * @brief return the number of elements in the priority queue.
     * @return the number of elements.
     */
    size_t size() const {
        return current_size;
    }

    /**
     * @brief check if the container is empty.
     * @return true if it is empty, false otherwise.
     */
    bool empty() const {
        return current_size == 0;
    }

    /**
     * @brief merge another priority_queue into this one.
     * The other priority_queue will be cleared after merging.
     * The complexity is at most O(logn).
     * @param other the priority_queue to be merged.
     */
    void merge(priority_queue &other) {
        if (this == &other) return;
        
        // Store original states for exception safety
        Node *original_root = root;
        Node *original_other_root = other.root;
        size_t original_size = current_size;
        size_t original_other_size = other.current_size;
        
        try {
            root = merge_nodes(root, other.root);
            current_size += other.current_size;
            other.root = nullptr;
            other.current_size = 0;
        } catch (...) {
            // Restore both queues to original states
            root = original_root;
            other.root = original_other_root;
            current_size = original_size;
            other.current_size = original_other_size;
            throw runtime_error();
        }
    }
};

}

#endif