#ifndef DIO
#define DIO
#include"splay.hpp"
#include<map>
#include"lct.hpp"
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
			size = a.size + b.size+on;
			if (!on) num = 0;
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
		node* path_root(node *o)
		{
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
		void link(node* x, node* y);
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
			if (find_root(x) != find_root(y)) return nullptr;
			access(x);
			access(y);
			x->splay();
			if (x->fa->id) return mp[x->fa->id];
			return x;
		}
	};
	typedef splay<info>::iterator ptr;
	class ett
	{
		friend lct;
		lct* com;
		lctt* str;
		std::map<int, ptr> first;
		std::map<int, ptr> second;
		int root;
	public:
		ett(int x) :first(), second(),com(nullptr) ,root(x){}
		void combine(lct* _com,lctt* _str)
		{
			com = _com;
			str = _str;
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
		void print(splay<info>* tr)
		{
			for (auto i : *tr)
				std::cout << i.id << " ";
			std::cout << std::endl;
			return;
		}
		splay<info>& tpcut(int x)
		{
			ptr pos1 = (*this)[x].first, pos2 = (*this)[x].second;
			splay<info>* temp = pos1.get_splay();
			splay<info>* ret = new splay<info>(temp->split(pos1, splay<info>::before));
			splay<info>* t = new splay<info>(ret->split(pos2, splay<info>::after));
			temp->merge(*t);
			return *ret;
		}
		splay<info>& tplink(int x, int y,int f);
	public:
		/*splay<info>& cut(int x)
		{
			com->cut((*com)[x]);
			ptr pos1 =(*this)[x].first, pos2 = (*this)[x].second;
			splay<info>* temp = pos1.get_splay();
			splay<info> ret = temp->split(pos1, splay<info>::before);
			splay<info> t = ret.split(pos2, splay<info>::after);
			temp->merge(t);
			return tpcut(x);
		}*/
		splay<info>& link(int x, int y)
		{
			int l=com->lca((*com)[root], (*com)[x])?com->lca((*com)[root], (*com)[x])->tid():0;
			int r = (*this)[x].first.get_splay()->begin().access().id;
			if (x == root) throw"nishabia";
			else
				if (l != x)
				{
					splay<info> *ret = new splay<info>(std::move(tplink(x, y, 0)));
					com->link((*com)[x], (*com)[y]);
					str->link((*str)[x], (*str)[y]);
					return *ret;
				}
				else
				{
					int t = com->path_root((*com)[root])->tid();
					com->change_root((*com)[t]);
					splay<info> *ret = new splay<info>(std::move(tplink(x, y, t)));
					root = t;
					tplink(x, y, 0);
					com->link((*com)[x], (*com)[y]);
					str->link((*str)[x], (*str)[y]);
					return *ret;
				}
		}
		int dlink(int x,int y, int d)
		{
			int ret = 0;
			//com->access((*com)[x]);
			ptr pos1 = (*this)[x].first;
			ptr pos2 = (*this)[y].first;
			if (pos1.get_index() > pos2.get_index()) std::swap(pos1, pos2);
			splay<info>* p=pos1.get_splay();
			splay<info>* temp = new splay<info>(p->split(pos1, splay<info>::before));
			splay<info>* t = new splay<info>(temp->split(pos2, splay<info>::after));
			if(d) pos1.access().add_lazy(d);
			else ret = pos1.access().sum;
			p->merge(*temp).merge(*t);
			return ret;
		}
		void chain(int x, int y,int d)
		{
			str->access((*str)[x]);
			str->access((*str)[y]);
			int l = com->lca((*com)[x], (*com)[y])->tid();
			if (l == x) dlink(x, y, d);
			else
			{
				dlink(l, y, d);
				dlink(com->path_root((*com)[x])->tid(), x, d);
			}
			return;
		}
		void point(int x, int d)
		{
			ptr pos = (*this)[x].first;
			pos.access().num += d;
			return;
		}
		int get_point(int x)
		{
			ptr pos = (*this)[x].first;
			return pos.access().num;
		}
		int zzk(int x, int d)
		{
			int p = 0;
			ptr pos1 = (*this)[x].first, pos2 = (*this)[x].second;
			splay<info>* temp = pos1.get_splay();
			splay<info>* ret = new splay<info>(temp->split(pos1, splay<info>::before));
			splay<info>* t = new splay<info>(ret->split(pos2, splay<info>::after));
			pos2.get_splay();
			if(d) pos2.access().add_lazy(d);
			else p = pos2.access().sum;
			temp->merge(*ret).merge(*t);
			return p;
		}
		void subtree(int x, int d)
		{
			int l = com->lca((*com)[root],(*com)[x])->tid();
			int y = (*this)[x].first.get_splay()->begin().access().id;
			if (x == root)
				zzk(y, d);
			else
			if (l != x)
				zzk(x, d);
			else
			{
				int t = com->path_root((*com)[root])->tid();
				zzk(y, d);
				zzk(t, -d);
			}
			return;
		}
		int get_chain(int x,int y)
		{
			int ret = 0;
			str->access((*str)[x]);
			str->access((*str)[y]);
			int l = com->lca((*com)[x], (*com)[y])->tid();
			if (l == x) ret=dlink(x, y, 0);
			else
			{
				ret=dlink(l, y, 0);
				ret+=dlink(com->path_root((*com)[x])->tid(), x, 0);
			}
			return ret;
		}
		int get_subtree(int x)
		{
			int ret = 0;
			int l = com->lca((*com)[root], (*com)[x])->tid();
			int y = (*this)[x].first.get_splay()->begin().access().id;
			if (x == root)
				ret = zzk(y, 0);
			else
			if (l != x)
				ret=zzk(x, 0);
			else
			{
				int t = com->path_root((*com)[root])->tid();
				ret=zzk(y, 0);
				ret-=zzk(t, 0);
			}
			return ret;
		}
		void change_root(int x)
		{
			str->change_root((*str)[x]);
			root = x;
		}
		void pt()
		{
			print((*this)[root].first.get_splay());
		}
	};
	splay<info>& ett::tplink(int x, int y,int f)
	{
		
		if (!f)
		{
			splay<info> temp(std::move(tpcut(x)));
			ptr pos = (*this)[y].first;
			pos.get_splay()->merge(temp, pos, splay<info>::after);
			return *pos.get_splay();
		}
		else
		{
			splay<info> temp(std::move(tpcut(f)));
			ptr pos = (*this)[f].first;
			ptr p = (*this)[x].first;
			temp.merge(*p.get_splay(), pos, splay<info>::after);
			return *new splay<info>(std::move(temp));
		}
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
			int x1 = u == nullptr ? 0 : path_root(u)->id;
			int x2 = v == nullptr ? 0 : v->id;
			if (x1 && x2) com->tplink(x2, x1,0);
			v = u;
			u = mp[u->fa->id];
		}
		return;
	}
	void lct::link(node* x, node* y)
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
	class dynamic_tree
	{
		ett t1;
		lct t2;
		lctt t3;
	public:
		dynamic_tree(int x) :t1(x), t2()
		{
			t1.combine(&t2,&t3);
			t2.combine(&t1);
		}
		//将x父亲换为y，没有删边操作
		void link(int x, int y)
		{
			t1.link(x, y);
		}
		void chain(int x,int y,int d)
		{
			t1.chain(x,y,d);
		}
		void subtree(int x,int d)
		{
			t1.subtree(x, d);
		}
		int get_chain(int x,int y)
		{
			return t1.get_chain(x,y);
		}
		int get_subtree(int x)
		{
			return t1.get_subtree(x);
		}
		void change_root(int x)
		{
			t1.change_root(x);
		}
		int lca(int x, int y)
		{
			return t3.lca(t3[x], t3[y])->id;
		}
		void print()
		{
			t1.pt();
		}
	};
}


#endif
