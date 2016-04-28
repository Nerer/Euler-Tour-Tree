#ifndef SJTU_LCT_HPP
#define SJTU_LCT_HPP

#include "splay.hpp"
#include <unordered_set>
#include <functional>
#include <stack>

namespace sjtu
{
	/*struct point_info
	{
		void split(point_info &a, point_info &b) {}
		void merge(const point_info &a, const point_info &b) {}
	};
	typedef int edge_key;*/
	//Maintain a Forest
	template<typename point_info, typename edge_key>
	class LCT
	{
	protected:
		struct lct_node;
		typedef splay<lct_node> lct_splay;
	public:
		class point : protected lct_splay::iterator
		{
			friend class LCT;
		public:
			point() = default;
			point_info & operator*()
			{
				return this->access().info;
			}
			point_info * operator->()
			{
				return &this->access().info;
			}
			bool is_accessible()
			{
				return iterator::is_accessible();
			}

			bool operator==(const point &other) const
			{
				return tree == other.tree && iterator::operator==(other);
			}
			bool operator!=(const point &other) const
			{
				return tree != other.tree || iterator::operator!=(other);
			}
		protected:
			LCT *tree;
		protected:
			point(typename lct_splay::iterator &iter, LCT *tree) : iterator(iter), tree(tree) {}
		};
	public:
		LCT() = default;

		~LCT()
		{
			for (lct_splay * i : list_splay)
				delete i;
		}

		point insert(const point_info &pinfo)
		{
			lct_splay * sp = new lct_splay();
			sp->insert(lct_node(pinfo));
			list_splay.insert(sp);
			return point(sp->begin(), this);
		}
		void link(point u, point v, const edge_key &e)
		{
			if (get_root(u) == get_root(v))
				throw std::runtime_error("Cannot link two points in the same tree");
			evert(u);
			access(v);
			u.access().upper = e;
			u.access().parent = v;
			access(u);
		}
		void cut(point o)
		{
			if (get_root(o) == o)
				return;
			access(o);
			lct_splay * sp = o.get_splay();
			auto tmp = sp->split(o);
			o.access().upper = edge_key();
			o.access().parent = typename lct_splay::iterator();
			if (!tmp.empty())
				list_splay.insert(new lct_splay(std::move(tmp)));
			if (sp->empty())
			{
				list_splay.erase(sp);
				delete sp;
			}
		}
		void evert(point o)
		{
			access(o);
			o.get_splay()->begin()->reverse_edge();
			o.get_splay()->reverse();
			o.access().upper = edge_key();
			o.access().parent = typename lct_splay::iterator();
		}
		point get_root(point o)
		{
			access(o);
			return point(o.get_splay()->begin(), this);
		}
		point get_lca(point u, point v)
		{
			access(u);
			access(v);
			if (u.get_splay() == v.get_splay())
				return u;
			return point(((lct_splay::iterator)u)->parent, this);
		}
		point_info query_path(point u, point v)
		{
			access(u);
			access(v);
			if (u.get_splay() == v.get_splay())
			{
				//root ---> u ---> v
				lct_splay * sp = u.get_splay();
				auto tmp = sp->split(u);
				point_info result = tmp.begin()->info;
				sp->merge(tmp);
				return result;
			}
			point_info result;
			lct_splay * sp = v.get_splay();
			auto lca = u.get_splay()->begin()->parent;
			auto tmp = sp->split(lca);
			result.merge(u.access().info, tmp.begin()->info);
			sp->merge(tmp);
			return result;
		}
		void modify_path(point u, point v, std::function<void(point_info&)> funcModify)
		{
			access(u);
			access(v);
			if (u.get_splay() == v.get_splay())
			{
				lct_splay * sp = u.get_splay();
				auto tmp = sp->split(u);
				funcModify(tmp.begin()->info);
				sp->merge(tmp);
			}
			else
			{
				lct_splay * sp = v.get_splay();
				auto lca = u.get_splay()->begin()->parent;
				auto tmp = sp->split(lca);
				u.get_splay()->merge(tmp);
				funcModify(u.get_splay()->begin()->info);
				tmp = u.get_splay()->split(lca);
				sp->merge(tmp);
			}
		}
		edge_key get_upper_edge(point o)
		{
			access(o);
			if(o.get_splay()->begin() != o)
				return o.access().upper;
			return edge_key();
		}
		point get_parent(point o)
		{
			if (o.get_splay()->begin() != o)
				return point(--o, this);
			return point(o.access().parent, this);
		}

		void set_on_access(std::function<void(point, point, edge_key, edge_key, bool)> func)
		{
			on_access = func;
		}

		void access(point o)
		{
			if (o.tree != this)
				throw std::runtime_error("Invalid Point");

			std::stack<typename lct_splay::iterator> stack;
			for (
				typename lct_splay::iterator i = o; 
				i.is_accessible(); 
				i = i.get_splay()->begin()->parent)
			{
				stack.push(i);
			}

			while (stack.size() >= 2)
			{
				auto i = stack.top();
				stack.pop();
				auto child_head = stack.top().get_splay()->begin();

				lct_splay *sp = i.get_splay(), *spchild = child_head.get_splay();

				if(on_access)
					on_access(point(child_head, this), point(i, this), child_head->upper, i->upper, i != --sp->end());


				auto tmp = sp->split(i, lct_splay::after);
				if (!tmp.empty())
				{
					tmp.begin()->parent = i;
					list_splay.insert(new lct_splay(std::move(tmp)));
				}

				(--sp->end())->lower = child_head->upper;
				sp->merge(*spchild);
				child_head->parent = lct_splay::iterator();

				assert(spchild->empty());
				list_splay.erase(spchild);
				delete spchild;
			}

			if(on_access)
				on_access(point(), o, edge_key(), o.access().upper, lct_splay::iterator(o) != --o.get_splay()->end());

			auto tmp = o.get_splay()->split(o, lct_splay::after);
			if (!tmp.empty())
			{
				tmp.begin()->parent = o;
				list_splay.insert(new lct_splay(std::move(tmp)));
			}
		}
		
	protected:
		struct lct_node
		{
			point_info info;
			typename lct_splay::iterator parent;
			edge_key upper, lower;
			bool edge_rev;

			lct_node() : edge_rev(false) {}
			lct_node(const point_info &info) : info(info), edge_rev(false) {}
			void reverse_edge()
			{
				edge_rev = true;
				std::swap(upper, lower);
			}
			void split(lct_node &a, lct_node &b)
			{
				if (edge_rev)
				{
					a.reverse_edge();
					b.reverse_edge();
					edge_rev = false;
				}
				info.split(a.info, b.info);
			}
			void merge(const lct_node &a, const lct_node &b)
			{
				info.merge(a.info, b.info);
			}
		};

		std::unordered_set<lct_splay *> list_splay;
		std::function<void(point, point, edge_key, edge_key, bool)> on_access;
	};
}
#endif