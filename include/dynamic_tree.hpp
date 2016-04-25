#ifndef SJTU_DYNAMIC_TREE_HPP
#define SJTU_DYNAMIC_TREE_HPP

#include "ETT.hpp"
#include "lct.hpp"

namespace sjtu
{
	struct edge_info
	{
		void split(edge_info &a, edge_info &b) {}
		void merge(const edge_info &a, const edge_info &b) {}
	};
	class dynamic_tree : protected lct, ETT<edge_info>
	{
	public:

	};
}

#endif