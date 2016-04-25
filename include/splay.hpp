#ifndef SJTU_SPLAY_HPP
#define SJTU_SPLAY_HPP

#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <queue>
#include <type_traits>

namespace sjtu
{
	/**
	 *	Write node_info::merge to merge the metadata if necesssary:
	 *		void node_info::merge(const node_info &a, const node_info &b [, bool rev_a, bool rev_b]);
	 *			* a, b:				the node info to be merged
	 *			* rev_a, rev_b:		[optional] if the interval of a (or b) is reversed
	 *
	 *	Write node_info::split to split the metadata if necessary:
	 *		void node_info::split(node_info &a, node_info &b [, bool rev_a, bool rev_b]);
	 *	
	 *	NOTE: If there is no metadata to be maintained, just simply skip the functions above.
	 *		For example, you can use 
	 *			splay<int> var;
	 *		Or
	 *			struct info
	 *			{
	 *				//...some data
	 *			};
	 *			splay<info> var;
	 **/
	template<typename node_info>
	class splay
	{
	protected:
		struct node;
	public:
		enum position
		{
			before, after
		};
		class iterator
		{
			friend class splay;
		public:
			iterator() : now(nullptr), tree(nullptr) {}

			size_t get_index()
			{
				splay_node(now);
				tree = now->tree;
				return now->lchild ? now->lchild->size : 0;
			}

			bool is_accessible() const 
			{
				return now;
			}

			splay<node_info> * get_splay()
			{
				splay_node(now);
				return tree = now->tree;
			}

			node_info & access()
			{
				splay_node(now);
				now->pushdown();
				tree = now->tree;
				return now->info;
			}
			node_info & operator*()
			{
				return access();
			}
			node_info * operator->() noexcept
			{
				return &access();
			}

			iterator operator++(int) 
			{
				iterator iter = *this;
				++(*this);
				return iter;
			}
			iterator & operator++()
			{
				if (!now)
					throw std::out_of_range("No Next Element");
				splay_node(now);
				tree = now->tree;
				node *o = now->rchild;
				if (!o)
				{
					now = nullptr;
					return *this;
				}
				while (o->pushdown(), o->lchild)
					o = o->lchild;
				now = o;
				return *this;
			}
			iterator operator--(int)
			{
				iterator iter = *this;
				--(*this);
				return iter;
			}
			iterator & operator--()
			{
				if (!now)
				{
					if (!tree)
						throw std::runtime_error("Invalid Iterator");
					node *o = tree->root;
					if (!o)
						throw std::out_of_range("No Previous Element");
					while (o->pushdown(), o->rchild)
						o = o->rchild;
					now = o;
					return *this;
				}
				splay_node(now);
				tree = now->tree;
				node *o = now->lchild;
				if (!o)
					throw std::out_of_range("No Previous Element");
				while (o->pushdown(), o->rchild)
					o = o->rchild;
				now = o;
				return *this;
			}

			bool operator==(const iterator &other) const
			{
				if (now)
					return now == other.now;
				return !other.now && tree == other.tree;
			}
			bool operator!=(const iterator &other) const
			{
				return !(*this == other);
			}
		protected:
			node *now;
			splay *tree;

		protected:
			iterator(node *now, splay *tree) : now(now), tree(tree) {}
		};
	public:
		splay() : root(nullptr)
		{
		}
		splay(splay &&other)
		{
			root = other.root;
			root->tree = this;
			other.root = nullptr;
		}
		splay(const splay &other) = delete;
		~splay()
		{
			clear();
		}

		splay & operator=(splay &&other)
		{
			if (&other == this)
				return *this;
			clear();
			root = other.root;
			root->tree = this;
			other.root = nullptr;
			return *this;
		}
		void clone(const splay &other)
		{
			clear();
			root = clone_node(other.root);
			root->parent = nullptr;
			root->tree = this;
		}
		iterator begin()
		{
			if (!root)
				return iterator(nullptr, this);
			node *o = root;
			while (o->pushdown(), o->lchild)
				o = o->lchild;
			return iterator(o, this);
		}
		iterator end()
		{
			return iterator(nullptr, this);
		}
		splay & merge(splay &other)
		{
			if (!root)
			{
				root = other.root;
				root->tree = this;
				other.root = nullptr;
			}
			(--end()).access();
			root->pushdown();
			root->rchild = other.root;
			if (root->rchild)
			{
				root->rchild->parent = root;
				root->rchild->tree = nullptr;
			}
			other.root = nullptr;
			root->maintain();
			return *this;
		}
		splay & merge(splay &other, iterator iter, position pos = before)
		{
			splay tmp = split(iter, pos);
			merge(other);
			merge(tmp);
			return *this;
		}
		splay split(iterator iter, position pos = before)
		{
			if (iter.now)
				iter.access();
			else
				return splay();
			if (iter.tree != this)
				throw std::runtime_error("Invalid Iterator");
			root->pushdown();
			if (pos == before)
			{
				node *k = root->lchild;
				root->lchild = nullptr;
				root->maintain();
				splay result(root);
				if (k)
				{
					k->parent = nullptr;
					k->tree = this;
				}
				root = k;
				return result;
			}
			else
			{
				node *k = root->rchild;
				if (k)
					k->parent = nullptr;
				root->rchild = nullptr;
				root->maintain();
				return splay(k);
			}
		}
		splay & remove(iterator iter)
		{
			splay tmp = split(iter);
			merge(tmp.split(tmp.begin, after));
			return *this;
		}
		iterator insert(const node_info &info)
		{
			splay tmp(new node(info, this));
			merge(tmp);
			return --end();
		}
		iterator insert(const node_info &info, iterator iter, position pos = before)
		{
			splay tmp = split(iter, pos);
			splay tmp2(new node(info, this));
			merge(tmp2);
			merge(tmp);
			return pos == before ? --iter : ++iter;
		}
		iterator find_node(size_t index)
		{
			node *o = find_node(index, root);
			if (o)
			{
				splay_node(o);
				assert(iterator(o, this).get_index() == index);
			}
			return iterator(o, this);
		}
		splay & reverse()
		{
			if (!root)
				return *this;
			root->reverse ^= 1;
			return *this;
		}
		splay & clear()
		{
			if (!root)
				return *this;
			std::queue<node *> Q;
			Q.push(root);
			while (!Q.empty())
			{
				node *o = Q.front();
				Q.pop();
				if (o->lchild)
					Q.push(o->lchild);
				if (o->rchild)
					Q.push(o->rchild);
				delete o;
			}
			return *this;
		}
		size_t size() const
		{
			if (!root)
				return 0;
			return root->size;
		}
		bool empty() const
		{
			return !root;
		}

		node_info & operator[](size_t index)
		{
			node *o = find_node(index, root);
			if (!o)
				throw std::out_of_range("Element doesn't Exist");
			return o->info;
		}
	protected:
		struct node
		{
			node *parent;
			splay *tree;
			node *lchild, *rchild;
			size_t size;
			bool reverse;
			node_info info;

			node(node_info info, splay *tree) : 
				info(info), tree(tree),
				parent(nullptr), lchild(nullptr), rchild(nullptr),
				size(1), reverse(false) 
			{
			}

			void pushdown()
			{
				if (reverse)
				{
					std::swap(lchild, rchild);
					if (lchild)
						lchild->reverse ^= 1;
					if (rchild)
						rchild->reverse ^= 1;
					reverse = false;
				}
				split::type::invoke(this);
				merge::type::invoke(this);
			}
			void maintain()
			{
				size = 1;
				if (lchild)
					size += lchild->size;
				if (rchild)
					size += rchild->size;
				merge::type::invoke(this);
			}
			void rotate_l()
			{
				assert(parent || (tree && tree->root));
				pushdown();
				if (rchild)
					rchild->pushdown();
				node *&p = parent ? (this == parent->lchild ? parent->lchild : parent->rchild) : tree->root;
				node *k = rchild;
				rchild = k->lchild;
				if (rchild)
					rchild->parent = this;
				k->lchild = this;
				k->parent = parent;
				k->tree = tree;
				tree = nullptr;
				parent = k;
				maintain();
				k->maintain();
				p = k;
			}
			void rotate_r()
			{
				assert(parent || (tree && tree->root));
				pushdown();
				if (lchild)
					lchild->pushdown();
				node *&p = parent ? (this == parent->lchild ? parent->lchild : parent->rchild) : tree->root;
				node *k = lchild;
				lchild = k->rchild;
				if (lchild)
					lchild->parent = this;
				k->rchild = this;
				k->parent = parent;
				k->tree = tree;
				tree = nullptr;
				parent = k;
				maintain();
				k->maintain();
				p = k;
			}

		protected:

			/********************************
			 * Detect node_info::split(...) and node_info::merge(...)
			 ********************************/
			class split
			{
				template<typename T>
				struct have_split
				{
					//TRICK: sizeof(T) must be at least 1
					static_assert(sizeof(T) == 0,
						"Invalid Definition of Member Function node_info::split()\n"
						"Candidates are:\n"
						"void node_info::split(node_info &, node_info &)\n"
						"void node_info::split(node_info &, node_info &, bool, bool)\n");
				};
				template<typename RetType, typename ClassName>
				struct have_split<RetType(ClassName::*)(ClassName &, ClassName &)>
				{
					static void invoke(node *o)
					{
						node_info empty1, empty2;
						o->info.split(o->lchild ? o->lchild->info : empty1, o->rchild ? o->rchild->info : empty2);
					}
				};
				template<typename RetType, typename ClassName>
				struct have_split<RetType(ClassName::*)(ClassName &, ClassName &, bool, bool)>
				{
					static void invoke(node *o)
					{
						node_info empty1, empty2;
						o->info.split(
							o->lchild ? o->lchild->info : empty1, o->rchild ? o->rchild->info : empty2, 
							o->lchild && o->lchild->reverse, o->rchild && o->rchild->reverse);
					}
				};
				struct no_split
				{
					static void invoke(node *o) {}
				};
				template<typename T>
				static have_split<decltype(&T::split)> detector(decltype(&T::split) *);
				template<typename T>
				static no_split detector(...);

			public:
				typedef decltype(detector<node_info>(nullptr)) type;
			};
			class merge
			{
				template<typename T>
				struct have_merge
				{
					static_assert(sizeof(T) == 0,
						"Invalid Definition of node_info::merge()\n"
						"Candidates are:\n"
						"void node_info::merge(const node_info &, const node_info &)\n"
						"void node_info::merge(const node_info &, const node_info &, bool, bool)");
				};
				template<typename RetType, typename ClassName>
				struct have_merge<RetType(ClassName::*)(const ClassName &, const ClassName &)>
				{
					static void invoke(node *o)
					{
						o->info.merge(o->lchild ? o->lchild->info : node_info(), o->rchild ? o->rchild->info : node_info());
					}
				};
				template<typename RetType, typename ClassName>
				struct have_merge<RetType(ClassName::*)(const ClassName &, const ClassName &, bool, bool)>
				{
					static void invoke(node *o)
					{
						o->info.merge(
							o->lchild ? o->lchild->info : node_info(), o->rchild ? o->rchild->info : node_info(),
							o->lchild && o->lchild->reverse, o->rchild && o->rchild->reverse);
					}
				};
				struct no_merge
				{
					static void invoke(node *o) {}
				};
				template<typename T>
				static have_merge<decltype(&T::merge)> detector(decltype(&T::merge) *);
				template<typename T>
				static no_merge detector(...);

			public:
				typedef decltype(detector<node_info>(nullptr)) type;
			};
			/********************************/

		};
		node *root;
	protected:
		splay(node *root) : root(root)
		{
			if (root)
			{
				root->parent = nullptr;
				root->tree = this;
			}
		}
		static void splay_node(node *o)
		{
			node *p;
			while (p = o->parent)
			{
				if (p->parent)
					p->parent->pushdown();
				p->pushdown();
				if (p->parent && o == p->lchild && p == p->parent->lchild)		//LL
				{
					p->parent->rotate_r();
					p->rotate_r();
				}
				else if (p->parent && o == p->rchild && p == p->parent->rchild)	//RR
				{
					p->parent->rotate_l();
					p->rotate_l();
				}
				else if (o == p->lchild)		//RL
				{
					p->rotate_r();
					if (o->parent)
					{
						assert(o == o->parent->rchild);
						o->parent->rotate_l();
					}
				}
				else						//LR
				{
					assert(o == p->rchild);
					p->rotate_l();
					if (o->parent)
					{
						assert(o == o->parent->lchild);
						o->parent->rotate_r();
					}
				}
			}
		}
		static node * find_node(size_t idx, node *o)
		{
			while (o)
			{
				o->pushdown();
				if (o->lchild && o->lchild->size > idx)
				{
					o = o->lchild;
					continue;
				}
				if (o->lchild)
					idx -= o->lchild->size;
				if (idx == 0)
					return o;
				--idx;
				o = o->rchild;
			}
			return nullptr;
		}
		static node * clone_node(node *o)
		{
			if (!o)
				return nullptr;
			node *k = new node(*o);
			k->lchild = clone_node(o->lchild);
			k->lchild->parent = o;
			k->rchild = clone_node(o->rchild);
			k->rchild->parent = o;
			return k;
		}
	};
}

#endif