#ifndef SJTU_LCT_HPP
#define SJTU_LCT_HPP

#include "splay.hpp"
#include <unordered_set>
#include <functional>

namespace sjtu
{
	/*struct point_info
	{
		void split(point_info &a, point_info &b) {}
		void merge(const point_info &a, const point_info &b) {}
	};*/
	//Maintain a Forest
	template<typename point_info>
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

			bool operator==(const point &other) const
			{
				return tree == other.tree && iterator::operator==(other);
			}
			bool operator!=(const point &other) const
			{
				return tree != other.tree || iterator::operator==(other);
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
		void link(point u, point v)
		{
			if (get_root(u) == get_root(v))
				throw std::runtime_error("Cannot link two points in the same tree");
			evert(u);
			access(v);
			lct_splay * sp = u.get_splay();
			v.get_splay()->merge(*sp);
			assert(sp->empty());
			list_splay.erase(sp);
			delete sp;
		}
		void cut(point o)
		{
			if (get_root(o) == o)
				return;
			access(o);
			lct_splay * sp = o.get_splay();
			auto tmp = sp->split(o);
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
			o.get_splay()->reverse();
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
		point_info query_path(typename lct_splay::iterator u, typename lct_splay::iterator v)
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
			auto tmp = sp->split(u->parent);
			result.merge(u->info, tmp.begin()->info);
			sp->merge(tmp);
			return result;
		}
		void modify_path(typename lct_splay::iterator u, typename lct_splay::iterator v, std::function<void(point_info&)> funcModify)
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
				auto tmp = sp->split(u->parent);
				u.get_splay()->merge(tmp);
				funcModify(u.get_splay()->begin()->info);
				tmp = u.get_splay()->split(u->parent);
				sp->merge(tmp);
			}
		}
		
	protected:
		struct lct_node
		{
			point_info info;
			typename lct_splay::iterator parent;

			lct_node() = default;
			lct_node(const point_info &info) : info(info) {}
		};

		std::unordered_set<lct_splay *> list_splay;

	protected:
		void access(point o)
		{
			if (o.tree != this)
				throw std::runtime_error("Invalid Point");

			lct_splay * sp = o.get_splay();
			auto tmp = sp->split(o, lct_splay::after);
			if (!tmp.empty())
			{
				tmp.begin()->parent = o;
				list_splay.insert(new lct_splay(std::move(tmp)));
			}
			
			auto head = sp->begin();
			if (head->parent.is_accessible())
			{
				access(point(head->parent, this));
				head->parent.get_splay()->merge(*sp);
				head->parent = lct_splay::iterator();
				assert(sp->empty());
				list_splay.erase(sp);
				delete sp;
			}
		}
		
	};
}
#endif