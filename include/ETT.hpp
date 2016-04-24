#ifndef SJTU_ETT_HPP
#define SJTU_ETT_HPP

#include "splay.hpp"
#include <cassert>
#include <utility>
#include <functional>

namespace sjtu
{
	struct edge_info
	{
		void split(edge_info &a, edge_info &b) {}
		void merge(const edge_info &a, const edge_info &b) {}
	};
	class ETT
	{
	protected:
		struct node;
	public:
		class edge : protected splay<node>::iterator
		{
			friend class ETT;
		public:
			edge() = default;
			edge_info & operator*()
			{
				if (!access().key_edge)
				{
					return access().iter->info;
				}
				return access().info;
			}
			edge_info * operator->()
			{
				return &(*(*this));
			}

		protected:
			edge(splay<node>::iterator &iter) : iterator(iter) {}
		};

		ETT() = default;
		ETT(const ETT &) = delete;
		ETT(ETT &&) = default;

		edge root()
		{
			return edge(etour.end());
		}
		edge insert(const edge_info &einfo, edge pos)
		{
			if (pos.access().iter.get_index() > pos.get_index())
				pos = pos.access().iter;
			auto iter1 = etour.insert(node(einfo), pos);
			auto iter2 = etour.insert(node(), pos);
			iter1->iter = iter2;
			iter2->iter = iter1;
			return iter1;
		}
		ETT & link(const edge_info &einfo, edge pos, ETT &other)
		{
			if (pos.access().iter.get_index() > pos.get_index())
				pos = pos.access().iter;
			etour.insert(node(einfo), pos);
			etour.merge(other.etour, pos);
			etour.insert(node(), pos);
			return *this;
		}
		ETT cut(edge pos)
		{
			if (pos == etour.end())
			{
				ETT ret(etour);
				etour.clear();
				return ret;
			}
			splay<node>::iterator iter_l, iter_r;
			get_iter(pos, iter_l, iter_r);
			auto tmp1 = etour.split(iter_l);
			auto tmp2 = tmp1.split(iter_r, splay<node>::after);
			etour.merge(tmp2);
			return ETT(tmp1);
		}
		ETT & modify_subtree(edge pos, std::function<void(edge_info &)> funcModify)
		{
			splay<node>::iterator iter_l, iter_r;
			if (pos == etour.end())
				iter_l = etour.begin(), iter_r = --etour.end();
			else
			{
				get_iter(pos, iter_l, iter_r);
				++iter_l;
				--iter_r;
			}
			auto tmp1 = etour.split(iter_l);
			auto tmp2 = tmp1.split(iter_r, splay<node>::after);
			funcModify(tmp1.begin()->info);
			etour.merge(tmp1).merge(tmp2);
		}
		edge_info query_subtree(edge pos)
		{
			splay<node>::iterator iter_l, iter_r;
			if (pos == etour.end())
				iter_l = etour.begin(), iter_r = --etour.end();
			else
			{
				get_iter(pos, iter_l, iter_r);
				++iter_l;
				--iter_r;
			}
			auto tmp1 = etour.split(iter_l);
			auto tmp2 = tmp1.split(iter_r, splay<node>::after);
			edge_info ret = tmp1.begin()->info;
			etour.merge(tmp1).merge(tmp2);
			return ret;
		}
		ETT & evert(edge pos)
		{
			if (pos == etour.end())
				return *this;
			splay<node>::iterator iter_l, iter_r;
			get_iter(pos, iter_l, iter_r);
			auto tmp1 = etour.split(iter_r);
			etour.merge(tmp1, etour.begin());
			return *this;
		}
	protected:
		ETT(splay<node> &et) : etour(std::move(et)) {}
		struct node
		{
			splay<node>::iterator iter;
			edge_info info;
			bool key_edge;
			node() : key_edge(false) {}
			node(const edge_info &einfo, bool key = true) : info(einfo), key_edge(key) {}
			void split(node &a, node &b) { info.split(a.info, b.info); }
			void merge(const node &a, const node &b) { info.merge(a.info, b.info); }
		};
		splay<node> etour;

	protected:
		static void get_iter(edge pos, splay<node>::iterator &iter_l, splay<node>::iterator &iter_r)
		{
			if (pos.access().iter.get_index() > pos.get_index())
				iter_l = pos, iter_r = pos.access().iter;
			else
				iter_l = pos.access().iter, iter_r = pos;
		}
	};
}
#endif