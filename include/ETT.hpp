#ifndef SJTU_ETT_HPP
#define SJTU_ETT_HPP

#include "splay.hpp"
#include <cassert>
#include <utility>
#include <functional>
#include <unordered_set>

#include <iostream>

namespace sjtu
{
	/*struct edge_info
	{
		void split(edge_info &a, edge_info &b) {}
		void merge(const edge_info &a, const edge_info &b) {}
	};*/
	template<typename edge_info>
	class ETT
	{
	protected:
		struct node;
		typedef splay<node, ETT *> ett_splay;
	public:
		class edge : protected ett_splay::iterator
		{
			friend class ETT;
		public:
			edge() = default;

			ETT * get_ett()
			{
				return iterator::get_splay()->mdata;
			}
			bool is_accessible()
			{
				return iterator::is_accessible();
			}
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

			bool operator==(const edge &other) const
			{
				return iterator::operator==(other);
			}
			bool operator!=(const edge &other) const
			{
				return iterator::operator!=(other);
			}

		protected:
			edge(typename ett_splay::iterator &iter) : iterator(iter) {}
		};

		ETT()
		{
			etour.mdata = this;
		}
		ETT(const ETT &) = delete;
		ETT(ETT &&other) : etour(std::move(other.etour))
		{
			etour.mdata = this;
		}

		edge root()
		{
			return etour.end();
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
		edge link(const edge_info &einfo, edge pos, ETT &other)
		{
			if (pos != root() && pos.access().iter.get_index() > pos.get_index())
				pos = pos.access().iter;
			auto ret = etour.insert(node(einfo), pos);
			etour.merge(other.etour, pos);
			ret->iter = etour.insert(node(), pos);
			ret->iter->iter = ret;
			return ret;
		}
		ETT cut(edge pos)
		{
			if (pos == root())
			{
				ETT ret(etour);
				etour.clear();
				return ret;
			}
			typename ett_splay::iterator iter_l, iter_r;
			get_iter(pos, iter_l, iter_r);
			auto tmp1 = etour.split(iter_l);
			auto tmp2 = tmp1.split(iter_r, ett_splay::after);
			etour.merge(tmp2);
			
			assert(!tmp1.empty());
			tmp1.remove(tmp1.begin());
			tmp1.remove(--tmp1.end());
			return ETT(tmp1);
		}
		void modify_subtree(edge pos, std::function<void(edge_info &)> funcModify)
		{
			if (empty())
			{
				assert(pos == root());
				return;
			}

			typename ett_splay::iterator iter_l, iter_r;
			if (pos == root())
				iter_l = etour.begin(), iter_r = --root();
			else
			{
				get_iter(pos, iter_l, iter_r);
				++iter_l;
				if (iter_l == iter_r)
					return;
				--iter_r;
			}
			modify_range(iter_l, iter_r, funcModify);
		}
		edge_info query_subtree(edge pos)
		{
			if (empty())
			{
				assert(pos == root());
				return edge_info();
			}

			typename ett_splay::iterator iter_l, iter_r;
			if (pos == root())
				iter_l = etour.begin(), iter_r = --root();
			else
			{
				get_iter(pos, iter_l, iter_r);
				++iter_l;
				if (iter_l == iter_r)
					return edge_info();
				--iter_r;
			}
			return query_range(iter_l, iter_r);
		}
		void evert(edge pos)
		{
			if (pos == root())
				return;
			typename ett_splay::iterator iter_l, iter_r;
			get_iter(pos, iter_l, iter_r);
			auto tmp1 = etour.split(iter_r);
			etour.merge(tmp1, etour.begin());
		}

		void reverse(edge pos)
		{
			if (pos == root())
			{
				etour.reverse();
				return;
			}
			typename ett_splay::iterator iter_l, iter_r;
			get_iter(pos, iter_l, iter_r);
			auto tmp1 = etour.split(iter_r, ett_splay::after);
			auto tmp2 = etour.split(iter_l);
			tmp2.reverse();
			etour.merge(tmp2).merge(tmp1);
		}
		void reverse_subtree(edge pos, bool flag)
		{
			typename ett_splay::iterator l, r;
			if (flag)
			{
				if (pos == root())
				{
					if (etour.size() <= 2)
						return;
					l = etour.begin().access().iter, r = --root();
					++l;
					if (l == r || l == root())
						return;
				}
				else
				{
					get_iter(pos, l, r);
					++l;
					if (l == r)
						return;
					l = l.access().iter;
					++l;
					if (l == r)
						return;
					--r;
				}
			}
			else
			{
				if (pos == root())
				{
					if (etour.size() <= 2)
						return;
					l = etour.begin(), r = --root();
				}
				else
				{
					get_iter(pos, l, r);
					++l;
					if (l == r)
						return;
					--r;
				}
			}

			auto tmp1 = etour.split(r, ett_splay::after);
			auto tmp2 = etour.split(l);
			tmp2.reverse();
			etour.merge(tmp2).merge(tmp1);
		}
		void modify_range(edge l, edge r, std::function<void(edge_info &)> funcModify)
		{
			if (l == root())
				l = etour.begin();
			auto tmp1 = etour.split(r, ett_splay::after);
			auto tmp2 = etour.split(l);
			funcModify(tmp2.begin()->info);
			etour.merge(tmp2).merge(tmp1);
		}
		edge_info query_range(edge l, edge r)
		{
			if (l == root())
				l = etour.begin();
			auto tmp1 = etour.split(r, ett_splay::after);
			auto tmp2 = etour.split(l);
			edge_info ret = tmp2.begin()->info;
			etour.merge(tmp2).merge(tmp1);
			return ret;
		}
		void prefer_child(edge o, edge parent)
		{
			if (o == root())
				throw std::runtime_error("Cannot set root as preferred child");
			typename ett_splay::iterator l, r, pl, pr;
			get_iter(o, l, r);

			if (parent == root())
			{
				auto tmp1 = etour.split(r, ett_splay::after);
				auto tmp2 = etour.split(l);
				etour.merge(tmp1);
				etour.merge(tmp2, etour.begin());
			}
			else
			{
				get_iter(parent, pl, pr);

				if (l.get_index() <= pl.get_index())
					throw std::runtime_error("o is not a child of parent");
				auto tmp1 = etour.split(r, ett_splay::after);
				auto tmp2 = etour.split(l);
				etour.merge(tmp1);
				etour.merge(tmp2, pl, ett_splay::after);
			}
		}

		bool empty() const
		{
			return etour.empty();
		}

	protected:
		ETT(typename ett_splay &et) : etour(std::move(et))
		{
			etour.mdata = this;
		}
		struct node
		{
			ETT *ett;
			typename ett_splay::iterator iter;
			edge_info info;
			bool key_edge;
			node() : key_edge(false) {}
			node(const edge_info &einfo, bool key = true) : info(einfo), key_edge(key) {}
			explicit node(ETT *ett) : ett(ett) {}
			void split(node &a, node &b) { info.split(a.info, b.info); }
			void merge(const node &a, const node &b) { info.merge(a.info, b.info); }
		};
		ett_splay etour;

	protected:
		static void get_iter(edge pos, typename ett_splay::iterator &iter_l, typename ett_splay::iterator &iter_r)
		{
			if (pos.access().iter.get_index() > pos.get_index())
				iter_l = pos, iter_r = pos.access().iter;
			else
				iter_l = pos.access().iter, iter_r = pos;
		}

		/*void check_iter(typename ett_splay::iterator i)
		{
			assert(i == root() || i.access().iter->iter == i);
		}*/
	};
}
#endif