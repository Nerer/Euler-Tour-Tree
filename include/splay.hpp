#ifndef SJTU_SPLAY_HPP
#define SJTU_SPLAY_HPP

#include <cstddef>
#include <stdexcept>

namespace sjtu
{
	/*struct node_info_default
	{
		void split(node_info_default &a, node_info_default &b) {}
		void merge(const node_info_default &a, const node_info_default &b) {}
	};*/
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
			size_t get_index()
			{
				splay_node(now);
				tree = now->tree;
				return now->lchild ? now->lchild->size : 0;
			}

			node_info & access()
			{
				splay_node(now);
				tree = now->tree;
				return now->info;
			}
			node_info & operator*()
			{
				splay_node(now);
				tree = now->tree;
				return now->info;
			}
			node_info * operator->() noexcept
			{
				splay_node(now);
				tree = now->tree;
				return &now->info;
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
		protected:
			node *now;
			const splay *tree;

		protected:
			iterator(node *now, const splay *tree) : now(now), tree(tree) {}
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
		void merge(splay &other)
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
			other.root = nullptr;
			root->maintain();
		}
		void merge(splay &other, iterator iter, position pos = before)
		{
			splay tmp = split(iter, pos);
			merge(other);
			merge(tmp);
		}
		splay split(iterator iter, position pos = before)
		{
			iter.access();
			if (iter.tree != this)
				throw std::runtime_error("Invalid Iterator");
			root->pushdown();
			if (pos == before)
			{
				node *k = root->lchild;
				root->lchild = nullptr;
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
				return splay(k);
			}
		}
		void remove(iterator iter)
		{
			splay tmp = split(iter);
			merge(tmp.split(tmp.begin, after));
		}
		void insert(const node_info &info)
		{
			merge(splay(new node(info, this)));
		}
		void insert(const node_info &info, iterator iter, position pos = before)
		{
			splay tmp = split(iter, pos);
			merge(splay(new node(info, this)));
			merge(tmp);
		}
		iterator find_node(size_t index) const
		{
			return iterator(find_node(index, root), this);
		}
		void reverse()
		{
			if (!root)
				return;
			root->reverse ^= 1;
		}
		void clear()
		{
			clear_node(root);
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
				}
				info.split(lchild ? lchild->info : node_info(), rchild ? rchild->info : node_info());
			}
			void maintain()
			{
				size = 1;
				if (lchild)
					size += lchild->size;
				if (rchild)
					size += rchild->size;
				info.merge(lchild ? lchild->info : node_info(), rchild ? rchild->info : node_info());
			}
			void rotate_l()
			{
				pushdown();
				if (rchild)
					rchild->pushdown();
				node *&p = parent ? (this == parent->lchild ? parent->lchild : parent->rchild) : tree->root;
				node *k = rchild;
				rchild = k->lchild;
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
				pushdown();
				if (lchild)
					lchild->pushdown();
				node *&p = parent ? (this == parent->lchild ? parent->lchild : parent->rchild) : tree->root;
				node *k = lchild;
				lchild = k->rchild;
				k->rchild = this;
				k->parent = parent;
				k->tree = tree;
				tree = nullptr;
				maintain();
				k->maintain();
				p = k;
			}
		};
		node *root;
	protected:
		splay(node *root) : root(root)
		{
			root->parent = nullptr;
			root->tree = this;
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
					if(o->parent)
						o->parent->rotate_l();
				}
				else						//LR
				{
					p->rotate_l();
					if (o->parent)
						o->parent->rotate_r();
				}
			}
		}
		void clear_node(node *o)
		{
			if (!o)
				return;
			clear_node(o->lchild);
			clear_node(o->rchild);
			delete o;
		}
		static node * find_node(size_t idx, node *o)
		{
			if (!o)
				return nullptr;
			o->pushdown();
			if (o->lchild && o->lchild->size >= idx)
				return find_node(idx, o->lchild);
			if(o->lchild)
				idx -= o->lchild->size;
			if (idx == 0)
				return o;
			--idx;
			return find_node(idx, o->rchild);
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