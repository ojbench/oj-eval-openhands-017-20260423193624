
#ifndef BPTREE_HPP
#define BPTREE_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include "utility.hpp"

namespace sjtu {

template <class Key, class Value, int M = 60, int L = 60>
class BPTree {
private:
    struct Node {
        bool is_leaf;
        int size;
        long parent;
        long next;
        Key keys[M];
        long children[M + 1]; // For internal nodes
        Value values[L];      // For leaf nodes

        Node() : is_leaf(false), size(0), parent(-1), next(-1) {
            for (int i = 0; i <= M; ++i) children[i] = -1;
        }
    };

    std::string filename;
    std::fstream file;
    long root_pos;
    long end_pos;

    void read_node(long pos, Node& node) {
        file.seekg(pos);
        file.read(reinterpret_cast<char*>(&node), sizeof(Node));
    }

    void write_node(long pos, const Node& node) {
        file.seekp(pos);
        file.write(reinterpret_cast<const char*>(&node), sizeof(Node));
    }

    long alloc_node() {
        long pos = end_pos;
        end_pos += sizeof(Node);
        return pos;
    }

    void update_header() {
        file.seekp(0);
        file.write(reinterpret_cast<char*>(&root_pos), sizeof(long));
        file.write(reinterpret_cast<char*>(&end_pos), sizeof(long));
    }

public:
    BPTree(const std::string& name) : filename(name) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) {
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
            root_pos = -1;
            end_pos = sizeof(long) * 2;
            update_header();
        } else {
            file.seekg(0);
            file.read(reinterpret_cast<char*>(&root_pos), sizeof(long));
            file.read(reinterpret_cast<char*>(&end_pos), sizeof(long));
        }
    }

    ~BPTree() {
        update_header();
        file.close();
    }

    void clear() {
        file.close();
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        root_pos = -1;
        end_pos = sizeof(long) * 2;
        update_header();
    }

    void insert(const Key& key, const Value& value) {
        if (root_pos == -1) {
            root_pos = alloc_node();
            Node root;
            root.is_leaf = true;
            root.size = 1;
            root.keys[0] = key;
            root.values[0] = value;
            write_node(root_pos, root);
            update_header();
            return;
        }

        long curr_pos = root_pos;
        Node curr;
        while (true) {
            read_node(curr_pos, curr);
            if (curr.is_leaf) break;
            int i = 0;
            while (i < curr.size && !(key < curr.keys[i])) i++;
            curr_pos = curr.children[i];
        }

        // Insert into leaf
        int i = 0;
        while (i < curr.size && curr.keys[i] < key) i++;
        // If key already exists, we might want to update or handle duplicates.
        // For this problem, we'll assume unique keys for now or handle duplicates externally.
        for (int j = curr.size; j > i; --j) {
            curr.keys[j] = curr.keys[j - 1];
            curr.values[j] = curr.values[j - 1];
        }
        curr.keys[i] = key;
        curr.values[i] = value;
        curr.size++;

        if (curr.size < L) {
            write_node(curr_pos, curr);
        } else {
            // Split leaf
            long next_pos = alloc_node();
            Node next_node;
            next_node.is_leaf = true;
            next_node.size = curr.size - curr.size / 2;
            curr.size /= 2;
            for (int j = 0; j < next_node.size; ++j) {
                next_node.keys[j] = curr.keys[curr.size + j];
                next_node.values[j] = curr.values[curr.size + j];
            }
            next_node.next = curr.next;
            curr.next = next_pos;
            next_node.parent = curr.parent;
            write_node(curr_pos, curr);
            write_node(next_pos, next_node);
            insert_into_parent(curr_pos, next_node.keys[0], next_pos);
        }
    }

    void insert_into_parent(long left_pos, const Key& key, long right_pos) {
        Node left;
        read_node(left_pos, left);
        if (left.parent == -1) {
            long new_root_pos = alloc_node();
            Node new_root;
            new_root.is_leaf = false;
            new_root.size = 1;
            new_root.keys[0] = key;
            new_root.children[0] = left_pos;
            new_root.children[1] = right_pos;
            write_node(new_root_pos, new_root);
            root_pos = new_root_pos;
            left.parent = root_pos;
            write_node(left_pos, left);
            Node right;
            read_node(right_pos, right);
            right.parent = root_pos;
            write_node(right_pos, right);
            update_header();
            return;
        }

        long parent_pos = left.parent;
        Node parent;
        read_node(parent_pos, parent);
        int i = 0;
        while (i < parent.size && parent.children[i] != left_pos) i++;
        for (int j = parent.size; j > i; --j) {
            parent.keys[j] = parent.keys[j - 1];
            parent.children[j + 1] = parent.children[j];
        }
        parent.keys[i] = key;
        parent.children[i + 1] = right_pos;
        parent.size++;

        if (parent.size < M) {
            write_node(parent_pos, parent);
            Node right;
            read_node(right_pos, right);
            right.parent = parent_pos;
            write_node(right_pos, right);
        } else {
            // Split internal node
            long next_pos = alloc_node();
            Node next_node;
            next_node.is_leaf = false;
            int mid = parent.size / 2;
            Key mid_key = parent.keys[mid];
            next_node.size = parent.size - mid - 1;
            parent.size = mid;
            for (int j = 0; j < next_node.size; ++j) {
                next_node.keys[j] = parent.keys[mid + 1 + j];
                next_node.children[j] = parent.children[mid + 1 + j];
            }
            next_node.children[next_node.size] = parent.children[parent.size + next_node.size + 1];
            next_node.parent = parent.parent;
            write_node(parent_pos, parent);
            write_node(next_pos, next_node);
            
            // Update children's parent pointer
            for (int j = 0; j <= next_node.size; ++j) {
                Node child;
                read_node(next_node.children[j], child);
                child.parent = next_pos;
                write_node(next_node.children[j], child);
            }
            
            Node right;
            read_node(right_pos, right);
            if (i < mid) right.parent = parent_pos;
            else right.parent = next_pos;
            write_node(right_pos, right);

            insert_into_parent(parent_pos, mid_key, next_pos);
        }
    }

    bool find(const Key& key, Value& value) {
        if (root_pos == -1) return false;
        long curr_pos = root_pos;
        Node curr;
        while (true) {
            read_node(curr_pos, curr);
            if (curr.is_leaf) break;
            int i = 0;
            while (i < curr.size && !(key < curr.keys[i])) i++;
            curr_pos = curr.children[i];
        }
        for (int i = 0; i < curr.size; ++i) {
            if (!(key < curr.keys[i]) && !(curr.keys[i] < key)) {
                value = curr.values[i];
                return true;
            }
        }
        return false;
    }

    void update(const Key& key, const Value& value) {
        if (root_pos == -1) return;
        long curr_pos = root_pos;
        Node curr;
        while (true) {
            read_node(curr_pos, curr);
            if (curr.is_leaf) break;
            int i = 0;
            while (i < curr.size && !(key < curr.keys[i])) i++;
            curr_pos = curr.children[i];
        }
        for (int i = 0; i < curr.size; ++i) {
            if (!(key < curr.keys[i]) && !(curr.keys[i] < key)) {
                curr.values[i] = value;
                write_node(curr_pos, curr);
                return;
            }
        }
    }

    void remove(const Key& key) {
        // Simplified remove: just mark as deleted or actually remove if easy.
        // For this problem, we might not need a full remove if we can just mark.
        // But let's implement a simple one if needed.
        // Actually, delete_train and refund_ticket might need it.
        // For now, let's just implement a "mark as deleted" in the value if possible,
        // or a simple removal from leaf without merging.
        if (root_pos == -1) return;
        long curr_pos = root_pos;
        Node curr;
        while (true) {
            read_node(curr_pos, curr);
            if (curr.is_leaf) break;
            int i = 0;
            while (i < curr.size && !(key < curr.keys[i])) i++;
            curr_pos = curr.children[i];
        }
        for (int i = 0; i < curr.size; ++i) {
            if (!(key < curr.keys[i]) && !(curr.keys[i] < key)) {
                for (int j = i; j < curr.size - 1; ++j) {
                    curr.keys[j] = curr.keys[j + 1];
                    curr.values[j] = curr.values[j + 1];
                }
                curr.size--;
                write_node(curr_pos, curr);
                return;
            }
        }
    }

    struct Iterator {
        BPTree* tree;
        long curr_pos;
        int index;
        Node curr_node;

        Iterator(BPTree* t, long p, int i) : tree(t), curr_pos(p), index(i) {
            if (curr_pos != -1) tree->read_node(curr_pos, curr_node);
        }

        bool isValid() const { return curr_pos != -1 && index < curr_node.size; }

        void next() {
            index++;
            if (index >= curr_node.size) {
                curr_pos = curr_node.next;
                index = 0;
                if (curr_pos != -1) tree->read_node(curr_pos, curr_node);
            }
        }

        Key key() const { return curr_node.keys[index]; }
        Value value() const { return curr_node.values[index]; }
    };

    Iterator lower_bound(const Key& key) {
        if (root_pos == -1) return Iterator(this, -1, 0);
        long curr_pos = root_pos;
        Node curr;
        while (true) {
            read_node(curr_pos, curr);
            if (curr.is_leaf) break;
            int i = 0;
            while (i < curr.size && !(key < curr.keys[i])) i++;
            curr_pos = curr.children[i];
        }
        int i = 0;
        while (i < curr.size && curr.keys[i] < key) i++;
        if (i < curr.size) return Iterator(this, curr_pos, i);
        return Iterator(this, curr.next, 0);
    }
};

} // namespace sjtu

#endif
