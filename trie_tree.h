#ifndef TRIE_TREE_H_
#define TRIE_TREE_H_

#include <string.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>

#define NODE_CAPACITY 4
#define MAX_LEN 1024
#define PREFIX_LEN 512

#define PREFIX_NUM  (3 * 1024 * 1024)
#define NODE_NUM (3 * 1024 * 1024)

#define NODE_SIZE 32

class Trie_Tree
{
public:
	// Intermediate string prefix for building tree.
	// Will be deleted after building done.
	struct Prefix
	{
		char *prefix_;
		int32_t len_;

		Prefix()
		{
			prefix_ = new char[PREFIX_LEN];
		}
		~Prefix()
		{
			delete[] prefix_;
		}
	};

	struct Tree_Node
	{
		char *data_;
		Tree_Node *parent_;
		std::vector<Tree_Node*> *children_;	
		Prefix *prefix_;

		Tree_Node()
		{
			data_ = new char[NODE_CAPACITY];
			memset(data_, 0x0, NODE_CAPACITY);
			parent_ = nullptr;
			children_ = nullptr;
			prefix_ = nullptr;
		}
		~Tree_Node()
		{
			delete[] data_;
		}
	};
	
	// An tree index for lookup, 
	// `id_` is the enity id that a string belongs to,
	// `path_len_` is the value string length.
	// `leaf_` is a pointer pointing to the leaf.
	using Tree_Index_Key = int64_t;
	struct Tree_Index_Value
	{
		int32_t height_;
		int32_t path_len_;
		Tree_Node *leaf_;
	};
	// using Tree_Index = std::unordered_map<Tree_Index_Key, Tree_Index_Value>;
	using Tree_Index = std::map<Tree_Index_Key, Tree_Index_Value>;
	using Empty_Index = std::unordered_set<Tree_Index_Key>;

public:
	Trie_Tree& ins()
	{
		static Trie_Tree ins;
		return ins;
	}

	Trie_Tree() = default;
	~Trie_Tree() = default;

	Trie_Tree(const Trie_Tree&) = delete;
	Trie_Tree(Trie_Tree&&) = delete;
	Trie_Tree&& operator=(const Trie_Tree&) = delete;
	Trie_Tree&& operator=(Trie_Tree&&) = delete;

	int init();
	int build_tree(const std::string& fname);
	int dump_tree(const std::string& fname);

private:
	int add(int64_t id, const char* arr, int len);
	int prune();
	Tree_Node* new_tree_node(const char *start, int len, Tree_Node *parent);
	void postorder_traverse(Tree_Node *root);
	void update_node_prefix(const char *start, int len, Tree_Node* cur);

	int split(char *src, const char *delim, char **dest);
	
	void update_index(Tree_Index_Key id, int32_t height, int32_t len, Tree_Node *leaf);

	Tree_Node root_;
	Tree_Index index_;
	Empty_Index empty_index_;

	using AuxMetaIndex = std::unordered_map<int, std::unordered_multimap<std::string, Tree_Node*>>;
	AuxMetaIndex aux_index_;
};

#endif
