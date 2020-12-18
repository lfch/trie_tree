#include "trie_tree.h"

#include <string>
#include <iostream>
#include <unistd.h>

char usage[] = "trie_tree <infile> <outfile>\n";

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << usage;
		return -1;
	}

	std::string infile = argv[1];
	std::string outfile = argv[2];


	Trie_Tree tree;
	tree.build_tree(infile);

	sleep(10000);

	tree.dump_tree(outfile);

	return 0;
}
