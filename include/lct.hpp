#ifndef SJTU_LCT_HPP
#define SJTU_LCT_HPP
#include<functional>
#include<map>
namespace sjtu
{
	class lctt
	{
	protected:
		class node
		{
		public:
			bool on;
			bool rev;
			node *fa;
			node* ch[2];
			int id;
			node(bool _on, int _id) :on(_on), rev(0), fa(), id(_id), ch() {}
			~node() {}
			void pushdown()
			{
				if (rev)
				{
					std::swap(ch[0], ch[1]);
					if (ch[0]) ch[0]->rev ^= 1;
					if (ch[1]) ch[1]->rev ^= 1;
					rev = 0;
				}
				return;
			}
			void rotate()
			{
				if (!fa->on) return;
				fa->pushdown();
				pushdown();
				bool f = this == fa->ch[1];
				bool o = f ^ 1;
				node* temp = fa->fa;
				if (temp->on)
					temp->ch[fa == temp->ch[1]] = this;
				fa->ch[f] = ch[o];
				if (ch[o]) ch[o]->fa = fa;
				ch[o] = fa;
				fa->fa = this;
				fa = temp;
				return;
			}
			//void reverse() { rev ^= 1; return; }
			void splay()
			{
				while (fa->on)
				{
					if (!fa->fa->on)
						rotate();
					else
					{
						bool f1 = this == fa->ch[1];
						bool f2 = fa == fa->fa->ch[1];
						if (f1 == f2)
						{
							fa->rotate();
							rotate();
						}
						else
						{
							rotate();
							rotate();
						}
					}
				}
				return;
			}
		public:
			int tid()
			{
				return id;
			}
		};
		std::map<int,node*> mp;
	public:
		lctt() :mp()
		{
			mp[0] = nullptr;
		}
		~lctt()
		{
			std::map<int,node*>::iterator iter = mp.begin();
			++iter;
			for (; iter != mp.end(); ++iter)
			{
				node* t = iter->second;
				if (t->fa)
				{
					if (!t->fa->on) delete t->fa;
					else
					{
						bool f = t == (t->fa->ch[1]);
						t->fa->ch[f] = nullptr;
					}
				}
				if (t->ch[0]) t->ch[0]->fa = nullptr;
				if (t->ch[1]) t->ch[1]->fa = nullptr;
				delete t;
			}
		}
		void clear()
		{
			std::map<int,node*>::iterator iter = mp.begin();
			++iter;
			for (; iter != mp.end(); ++iter)
			{
				node* t = iter->second;
				if (t->fa)
				{
					if (!t->fa->on) delete t->fa;
					else
					{
						bool f = t == (t->fa->ch[1]);
						t->fa->ch[f] = nullptr;
					}
				}
				if (t->ch[0]) t->ch[0]->fa = nullptr;
				if (t->ch[1]) t->ch[1]->fa = nullptr;
				delete t;
			}
			mp.clear();
		}
	public:
		void access(node* u)
		{
			node* v = nullptr;
			while (u)
			{
				u->splay();
				u->pushdown();
				if (u->ch[1])
					u->ch[1]->fa = new node(0, u->id);
				if (v)
				{
					if (!v->fa->on) delete v->fa;
					v->fa = u;
				}
				u->ch[1] = v;
				v = u;
				u = mp[u->fa->id];
			}
			return;
		}
	public:
		node* find_root(node* o)
		{
			access(o);
			o->splay();
			o->pushdown();
			while (o->ch[0])
			{
				o = o->ch[0];
				o->pushdown();
			}
			o->splay();
			return mp[o->id];
		}
		void cut(node* o)
		{
			access(o);
			o->splay();
			o->pushdown();
			if (o->ch[0]) o->ch[0]->fa = new node(0, 0);
			o->ch[0] = nullptr;
			return;
		}
		void change_root(node* o)
		{
			access(o);
			o->splay();
			o->rev ^= 1;
			return;
		}
		void link(node* x, node* y)
		{
			//change_root(x);
			access(y);
			y->splay();
			y->pushdown();
			y->ch[1] = x;
			if (!x->fa->on) delete x->fa;
			x->fa = y;
			return;
		}
		node* operator[](int x)
		{
			std::map<int,node*>::iterator iter;
			iter = mp.find(x);
			if (iter == mp.end())
			{
				node* ret;
				ret = mp[x] = new node(1, x);
				mp[x]->fa = new node(0, 0);
				return ret;
			}
			else return iter->second;
		}
		node* lca(node* x, node* y)
		{
			if (find_root(x) != find_root(y)) return nullptr;
			access(x);
			access(y);
			x->splay();
			if (x->fa->id) return mp[x->fa->id];
			else return x;
		}
	};
}
#endif