#include "trie_tree.h"

#include <fstream>
#include <string.h>
#include <stdio.h>
#include <string>
#include <utility>
#include <functional>
#include <sys/time.h>

#include <iostream>

int Trie_Tree::split(char *src, const char *delim, char **dest)
{
        if (src == nullptr || strlen(src) == 0) return 0;
        if (delim == nullptr || strlen(delim) == 0) return 0;
        if (dest == nullptr) return 0;

        int count = 0;
        char *next = nullptr;
        char *saveptr = nullptr;;
        next = (char *)strtok_r(src, delim, &saveptr);
        while(next) {
                *dest++ = next;
                ++count;
                next = strtok_r(nullptr, delim, &saveptr);
        }

        return count;
}

int Trie_Tree::build_tree(const std::string& fname)
{
	std::ifstream ifs(fname);
	std::string line;
	char str[1024];
	int64_t id = -1;
	char *res[2];
	int64_t line_counter = 0;
	
	struct timeval start;
	gettimeofday(&start, nullptr);

	while (std::getline(ifs, line))
	{
		++line_counter;
		int nsplit = split(const_cast<char*>(line.c_str()), ",", res);
		id = strtol(res[0], nullptr, 10);
		if (nsplit == 1)
		{
			this->empty_index_.insert(id);
			continue;
		}
		int ret = this->add(id, res[1], strlen(res[1]));
		if (ret != 0) return -1;
	}

	if (this->prune() != 0) return -1;

	struct timeval end;
	gettimeofday(&end, nullptr);

	int64_t cost = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;

	std::cout << "line num = " << line_counter << std::endl;
	std::cout << "tree build cost = " << cost << "ms\n";

	//if (this->dump_tree() != 0) return -1;

	return 0;
}

int Trie_Tree::add(int64_t id, const char *arr, int len)
{
	int counter = 0;
	int height = 1;
	Tree_Node *parent = &root_;
	const char *start = arr;

	int field_len = NODE_CAPACITY;
	if (len < NODE_CAPACITY) field_len = len;

	bool add_new_node = false;
	bool match_totally = false;

	while(*start != '\0')
	{
		counter += field_len;

		std::string cur_field(start, field_len);

		auto height_it = aux_index_.find(height);
		if (height_it == aux_index_.end())
		{
			std::unordered_multimap<std::string, Tree_Node*> tmp_map;
			aux_index_.emplace(std::make_pair(height, tmp_map));
		}

		auto & map = aux_index_[height];
		auto range = map.equal_range(cur_field);

		if (range.first == map.end() && range.second == map.end())
		{
			add_new_node = true;
		}
		else
		{
			bool match_partially = false;

			for (auto it = range.first; it != range.second; ++it)
			{
				Trie_Tree::Prefix *prefix = it->second->prefix_;

				// better use calculated len instead of prefix->len_
				bool equal = (memcmp(prefix->prefix_, arr, prefix->len_) == 0);
				if (equal == false)
				{
					// if not equal, go to next matched substr
					auto tmp_it = it;
					if (++tmp_it == range.second) match_partially = false;
					continue;
				}
				else
				{
					// if equal, go to next level.
					parent = it->second;
					match_partially = true;
					if (counter == len) match_totally = true;
					break;
				}
			}

			add_new_node = !match_partially;
		}

		Tree_Node * cur = nullptr;
		if (add_new_node)
		{
			cur = new_tree_node(start, field_len, parent);
			auto & m = aux_index_[height];
			m.emplace(std::make_pair(cur_field, cur));
			parent = cur;
		}

		if (match_totally) cur = parent;
		
		start += field_len;
		
		if (len - counter >= NODE_CAPACITY) field_len = NODE_CAPACITY;	
		else field_len = len - counter;

		if (counter == len) this->update_index(id, height, len, cur);

		++height;
	}

	return 0;
}

// delete children linkage
// clear aux index 
int Trie_Tree::prune()
{
	this->aux_index_.clear();
	this->postorder_traverse(&root_);
	return 0;
}

void Trie_Tree::postorder_traverse(Tree_Node *root)
{
	if (!root) return;
	if (root->children_)
	{
		for (auto & child : *root->children_)
		{
			if (child) postorder_traverse(child);
		}
	}
	delete root->children_;
	root->children_ = nullptr;
	delete root->prefix_;
	root->prefix_ = nullptr;
}

int Trie_Tree::dump_tree(const std::string& fname)
{
	std::ofstream ofs(fname);

	for (const auto & it : this->index_)
	{
		char res[it.second.path_len_ + 1];
		memset(res, 0x0, sizeof(res));
		
		Tree_Node *cur = it.second.leaf_;
		int32_t level = it.second.height_;

		while (level != 0)
		{
			int32_t len = NODE_CAPACITY;

			if (level == it.second.height_)
			{
				len = it.second.path_len_ - (level - 1) * NODE_CAPACITY;
			}
			if (strlen(cur->data_) != 0)
			{
				memcpy(res + (level - 1) * NODE_CAPACITY, cur->data_, len);
			}

			--level;
			cur = cur->parent_;
		}

		res[it.second.path_len_] = '\0';

		ofs << it.first << "," << res << std::endl;
	}

	for (const auto & it : this->empty_index_)
	{
		ofs << it << "," << std::endl;
	}

	return 0;	
}
	
Trie_Tree::Tree_Node* Trie_Tree::new_tree_node(
		const char *start, int len, Tree_Node *parent)
{
	Tree_Node *node = new Tree_Node();
	memcpy(node->data_, start, len);
	node->parent_ = parent;
	 if (!parent->children_) parent->children_ = new std::vector<Tree_Node*>();
	parent->children_->push_back(node);

	// set current node data prefix.
	this->update_node_prefix(start, len, node);

	return node;
}

void Trie_Tree::update_node_prefix(const char *start, int len, Tree_Node* cur)
{
	Trie_Tree::Prefix *prefix = new Trie_Tree::Prefix();
	cur->prefix_ = prefix;
	Trie_Tree::Prefix *parent_prefix = cur->parent_->prefix_;
	if (parent_prefix)
	{
		memcpy(prefix->prefix_, parent_prefix->prefix_, parent_prefix->len_);
		memcpy(prefix->prefix_ + parent_prefix->len_, start, len);
	}
	else 
		memcpy(prefix->prefix_, start, len);

	if (parent_prefix)
		prefix->len_ = parent_prefix->len_ + len;
	else
		prefix->len_ = len;
}

inline void Trie_Tree::update_index(Tree_Index_Key id,
		int32_t height, int32_t len, Tree_Node *leaf)
{
	Tree_Index_Value value = {height, len, leaf};	
	this->index_.insert(std::make_pair(std::ref(id), std::ref(value)));
}
