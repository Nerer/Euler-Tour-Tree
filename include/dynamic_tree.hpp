#ifndef DIO
#define DIO
#include"splay.hpp"
#include<map>
#include<iostream>
namespace sjtu
{
	struct info
	{
		bool on;
		int id;
		int size;
		int num;
		int sum;
		int lazy;
		info() :on(0),id(0),size(0), sum(0), lazy(0), num(0) {}
		info(int x,bool _on):on(_on),id(x),size(_on),sum(0),lazy(0), num(0){}
		void add_lazy(int delta)
		{
			lazy += delta;
			sum += delta * size;
		}
		void merge(const info & a, const info & b)
		{
			size = a.size + b.size+1;
			sum = a.sum + b.sum + num;
		}
		void split(info & a, info & b)
		{
			if (lazy)
			{
				a.add_lazy(lazy);
				b.add_lazy(lazy);
				num += lazy;
				lazy = 0;
			}
		}
	};
	class ett;
	class lct
	{
	protected:
		ett* com;
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
		std::map<int, node*> mp;
	public:
		lct(): mp()
		{
			mp[0] = nullptr;
		}
		void combine(ett* _com)
		{
			com = _com;
		}
		~lct()
		{
			std::map<int, node*>::iterator iter = mp.begin();
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
			std::map<int, node*>::iterator iter = mp.begin();
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
		void access(node* u);
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
			change_root(x);
			access(y);
			y->splay();
			y->pushdown();
			y->ch[1] = x;
			if (x->fa->on) delete x->fa;
			x->fa = y;
			return;
		}
		node* operator[](int x)
		{
			std::map<int, node*>::iterator iter;
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
			access(x);
			access(y);
			x->splay();
			if (x->fa->id) return mp[x->fa->id];
			else return x;
		}
	};
	typedef splay<info>::iterator ptr;
	class ett
	{
		friend lct;
		lct* com;
		std::map<int, ptr> first;
		std::map<int, ptr> second;
	public:
		ett() :first(), second(),com(nullptr) {}
		void combine(lct* _com)
		{
			com = _com;
		}
		std::pair<ptr, ptr> operator[](int x)
		{
			std::map<int, ptr>::iterator iter1 = first.find(x), iter2 = second.find(x);
			//assert((iter1 == first.end()) == (iter2 == second.end()));
			if (iter1 == first.end() && iter2 == second.end())
			{
				splay<info> *tr = new splay<info>();
				tr->insert(info(x,1));
				tr->insert(info(x,0));
				first[x] = tr->begin();
				second[x] = --tr->end();
			}
			iter1 = first.find(x); iter2 = second.find(x);
			return std::pair<ptr, ptr>(iter1->second, iter2->second);
		}
	public:
		
		splay<info> tpcut(int x)
		{
			ptr pos1 = (*this)[x].first, pos2 = (*this)[x].second;
			splay<info>* temp = pos1.get_splay();
			splay<info> ret = temp->split(pos1, splay<info>::before);
			splay<info> t = ret.split(pos2, splay<info>::after);
			temp->merge(t);
			return ret;
		}
		splay<info>& tplink(int x, int y);
	public:
		void print(int x)
		{
			/*for (auto i : *first[x].get_splay())
				std::cout << i.sum << " ";
			std::cout << std::endl;*/
			return;
		}
		splay<info> cut(int x)
		{
			com->cut((*com)[x]);
			splay<info>* temp;
			ptr pos1 =(*this)[x].first, pos2 = (*this)[x].second;
			splay<info> ret = temp->split(pos1, splay<info>::before);
			splay<info> t = ret.split(pos2, splay<info>::after);
			temp->merge(t);
			return ret;
		}
		splay<info>& link(int x, int y)
		{
			com->link((*com)[x], (*com)[y]);
			splay<info> temp = tpcut(x);
			ptr pos = (*this)[y].first;
			first[y].get_splay()->merge(temp, pos, splay<info>::after);
			print(x);
			return *first[y].get_splay();
		}
		void chain(int x, int d)
		{
			com->access((*com)[x]);
			print(x);
			ptr pos = (*this)[x].first;
			splay<info>* p=pos.get_splay();
			splay<info>* temp = new splay<info>(p->split(pos, splay<info>::after));
			pos.access().add_lazy(d);
			temp->merge(*p, temp->begin());
			print(x);
			return;
		}
		void subtree(int x, int d)
		{
			ptr pos1 = (*this)[x].first, pos2 = (*this)[x].second;
			splay<info>* temp = pos1.get_splay();
			splay<info> ret = temp->split(pos1, splay<info>::before);
			splay<info> t = ret.split(pos2, splay<info>::after);
			pos2.get_splay();
			pos2.access().add_lazy(d);
			temp->merge(ret).merge(t);
			return;
		}
		int get_chain(int x)
		{
			int ret = 0;
			com->access((*com)[x]);
			print(x);
			ptr pos = (*this)[x].first;
			splay<info>* p = pos.get_splay();
			splay<info>* temp = new splay<info>(p->split(pos, splay<info>::after));
			ret = pos.access().sum;
			temp->merge(*p, temp->begin());
			return ret;
		}
		int get_subtree(int x)
		{
			int rett = 0;
			ptr pos1 = (*this)[x].first, pos2 = (*this)[x].second;
			splay<info>* temp = pos1.get_splay();
			splay<info> ret = temp->split(pos1, splay<info>::before);
			splay<info> t = ret.split(pos2, splay<info>::after);
			pos2.get_splay();
			rett= pos2.access().sum;
			temp->merge(ret).merge(t);
			return rett;
		}
	};
	splay<info>& ett::tplink(int x, int y)
	{
		splay<info> temp = tpcut(x);
		ptr pos = (*this)[y].first;
		first[y].get_splay()->merge(temp, pos, splay<info>::after);
		return *first[y].get_splay();
	}
	void lct::access(node* u)
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
			int x1 = u == nullptr ? 0 : u->id;
			int x2 = v == nullptr ? 0 : v->id;
			if (x1 && x2) com->tplink(x2, x1);
			v = u;
			u = mp[u->fa->id];
		}
		return;
	}
	class dynamic_tree
	{
		ett t1;
		lct t2;
	public:
		dynamic_tree() :t1(), t2()
		{
			t1.combine(&t2);
			t2.combine(&t1);
		}
		void link(int x, int y)
		{
			t1.link(x, y);
		}
		void cut(int x)
		{
			t1.cut(x);
		}
		void chain(int x,int d)
		{
			t1.chain(x,d);
		}
		void subtree(int x,int d)
		{
			t1.subtree(x, d);
		}
		int get_chain(int x)
		{
			return t1.get_chain(x);
		}
		int get_subtree(int x)
		{
			return t1.get_subtree(x);
		}
		int lca(int x, int y)
		{
			return t2.lca(t2[x], t2[y])->id;
		}
	};
}


#endif
