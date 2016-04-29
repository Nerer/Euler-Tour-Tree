#ifndef SJTU_DYNAMIC_TREE_HPP
#define SJTU_DYNAMIC_TREE_HPP

#include "ETT.hpp"
#include "lct.hpp"
#include <list>

namespace sjtu
{
	/*struct edge_info
	{
		void split(edge_info &a, edge_info &b) {}
		void merge(const edge_info &a, const edge_info &b) {}
	};*/
	template<typename edge_info>
	class dynamic_tree
	{
	protected:
		struct tree;
		struct lct_info;
		struct ett_info;
		typedef LCT<lct_info, typename ETT<ett_info>::edge> Tlct;
	public:
		class iterator_edge
		{
			friend class dynamic_tree;
			friend class iterator;
		public:
			iterator_edge() = default;

			edge_info & operator*()
			{
				return *e;
			}
		protected:
			typename ETT<edge_info>::edge e;
			dynamic_tree *tree;

		protected:
			iterator_edge(typename ETT<edge_info>::edge e, dynamic_tree *tree) : e(e), tree(tree) {}
		};
		class iterator
		{
			friend class dynamic_tree;
		public:
			iterator() = default;

			/*iterator_edge upper()
			{
				return iterator_edge(tree->get_upper_edge(pt), tree);
			}*/

		protected:
			typename Tlct::point pt;
			dynamic_tree *tree;

		protected:
			iterator(typename Tlct::point pt, dynamic_tree *tree) : pt(pt), tree(tree) {}
			operator typename Tlct::point()
			{
				return pt;
			}
			ETT<ett_info> * ett()
			{
				auto e = upper_edge();
				if (e.is_accessible())
					return e.get_ett();
				else
					return pt->tree->ett;
			}
			typename ETT<ett_info>::edge upper_edge()
			{
				return tree->lct.get_upper_edge(pt);
			}
		};

	public:
		dynamic_tree() : lock_rev(false)
		{
			using namespace std::placeholders;
			lct.set_on_access(std::bind(
				&dynamic_tree<edge_info>::on_access, this, 
				_1, _2, _3, _4, _5));
		}
		iterator get_lca(iterator u, iterator v)
		{
			return iterator(lct.get_lca(u, v), this);
		}
		void evert(iterator o)
		{
			auto root = lct.get_root(o);
			if (root == o)
				return;
			assert(lct.get_parent(o).is_accessible());

			lct.access(o);
			lock_rev = true;
			lct.modify_path(lct.get_root(o), lct.get_parent(o), [](lct_info &info) { info.set_rev(); });
			lct.access(o);
			lock_rev = false;
			//The structure of the LCT should be exactly the same as the one before LOCK

			auto upper = o.upper_edge();
			if (upper.is_accessible())
			{
				auto subtree = o.ett();
				subtree->evert(upper);
				subtree->reverse(upper);
			}
			lct.evert(o);
			o.pt->tree = root->tree;
			root->tree = typename std::list<typename dynamic_tree::tree>::iterator();
		}
		void link(iterator u, iterator v, const edge_info &e)
		{
			if (lct.get_root(u) == lct.get_root(v))
				throw std::runtime_error("Cannot link two points in the same tree");

			tree &tr = *lct.get_root(u)->tree;

			evert(u);

			ETT<ett_info> *subtree = u.ett();
			ETT<ett_info> *tree = v.ett();

			ett_info edge(e, u, v);
			auto upper = v.upper_edge();
			auto iter_e = tree->link(edge, upper.is_accessible() ? upper : tree->root(), *subtree);

			u.pt->edges.push_back(iter_e);
			iter_e->iterU = --u.pt->edges.end();
			v.pt->edges.push_back(iter_e);
			iter_e->iterV = --v.pt->edges.end();

			lct.link(u.pt, v.pt, iter_e);

			assert(subtree->empty());
			assert(tr.ett == subtree);
			tree_list.erase(tr.iter);
			delete subtree;
		}
		void cut(iterator o)
		{
			if (lct.get_root(o) == o)
				return;
			assert(o.upper_edge().is_accessible());

			auto ett = o.ett();
			auto e = o.upper_edge();
			e->ptU->edges.erase(e->iterU);
			e->ptV->edges.erase(e->iterV);

			lct.cut(o);
			auto tmp = ett->cut(e);
			tree_list.push_back(tree());
			tree &tr = tree_list.back();
			tr.iter = --tree_list.end();
			tr.root = o.pt;
			tr.ett = new ETT<ett_info>(std::move(tmp));
			o.pt->tree = tr.iter;
		}
		edge_info query_subtree(iterator o)
		{
			auto e = o.upper_edge();
			auto st = o.ett();
			if (e.is_accessible())
				return st->query_subtree(e).metadata;
			else
				return st->query_subtree(st->root()).metadata;
		}
		void modify_subtree(iterator o, std::function<void(edge_info&)> funcModify)
		{
			auto e = o.upper_edge();
			auto st = o.ett();
			if (e.is_accessible())
				st->modify_subtree(e, [&funcModify](ett_info &info) { funcModify(info.metadata); });
			else
				st->modify_subtree(st->root(), [&funcModify](ett_info &info) { funcModify(info.metadata); });
		}
		edge_info query_path(iterator u, iterator v)
		{
			if (lct.get_root(u) != lct.get_root(v))
				throw std::runtime_error("No path between two points");
			auto root = lct.get_root(u);
			auto tree = u.ett();
			if (root != u)
				evert(u);
			edge_info ret = tree->query_range(tree->root(), v.upper_edge()).metadata;
			if (root != u)
				evert(iterator(root, this));
			return ret;
		}
		void modify_path(iterator u, iterator v, std::function<void(edge_info&)> funcModify)
		{
			if (lct.get_root(u) != lct.get_root(v))
				throw std::runtime_error("No path between two points");
			auto root = lct.get_root(u);
			auto tree = u.ett();
			if (root != u)
				evert(u);

			lct.access(v);

			tree->modify_range(tree->root(), v.upper_edge(), [&funcModify](ett_info &info) { funcModify(info.metadata); });

			if (root != u)
			{
				evert(iterator(root, this));
			}
		}
		iterator insert()
		{
			tree_list.push_back(tree());
			tree &tmp = tree_list.back();
			tmp.iter = --tree_list.end();
			tmp.root = lct.insert(lct_info(tmp.iter));
			tmp.ett = new ETT<ett_info>();
			return iterator(tmp.root, this);
		}
		iterator get_root(iterator o)
		{
			return iterator(lct.get_root(o), this);
		}

	protected:
		struct tree
		{
			typename Tlct::point root;
			ETT<ett_info> *ett;
			typename std::list<tree>::iterator iter;
		};
		struct lct_info
		{
			int dbg_idx;

			typename std::list<typename dynamic_tree::tree>::iterator tree;
			std::list<typename ETT<ett_info>::edge> edges;
			bool rev;
			bool rev_tag;
			lct_info() : rev(false), rev_tag(false) {}
			lct_info(typename std::list<typename dynamic_tree::tree>::iterator tree) : tree(tree)
			{
				static int idxn = 0;
				dbg_idx = idxn++;
			}
			void set_rev()
			{
				rev ^= 1;
				rev_tag ^= 1;
			}
			void split(lct_info &a, lct_info &b)
			{
				if (rev_tag)
				{
					a.set_rev(), b.set_rev();
					rev_tag = false;
				}
			}
			void merge(const lct_info &, const lct_info &) {}
		};
		struct ett_info
		{
			edge_info metadata;
			typename std::list<typename ETT<ett_info>::edge>::iterator iterU, iterV;
			typename Tlct::point ptU, ptV;
			ett_info() = default;
			ett_info(const edge_info &metadata, const typename Tlct::point ptU, const typename Tlct::point ptV) : metadata(metadata), ptU(ptU), ptV(ptV) {}
			void split(ett_info &a, ett_info &b)
			{
				metadata.split(a.metadata, b.metadata);
			}
			void merge(const ett_info &a, const ett_info &b)
			{
				metadata.merge(a.metadata, b.metadata);
			}
		};

	protected:
		std::list<tree> tree_list;
		Tlct lct;
		bool lock_rev;

	protected:
		//flag: true if parent has a preferred child
		void on_access(typename Tlct::point o, typename Tlct::point parent, 
			typename ETT<ett_info>::edge e, typename ETT<ett_info>::edge pe, bool flag)
		{
			if (!pe.is_accessible())
				pe = parent->tree->ett->root();
			if (parent->rev && !lock_rev)
			{
				pe.get_ett()->reverse_subtree(pe, flag);
				parent->rev = false;
			}
			if(o.is_accessible())
				pe.get_ett()->prefer_child(e, pe);

		}
	};
}

#endif