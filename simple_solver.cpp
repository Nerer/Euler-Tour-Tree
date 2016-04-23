#include <iostream>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <vector>

using namespace std;

class SplayTree{
private:
	class Node{
	public:
		Node* father, *ch[2];
		int key, value;
		Node() {
			father = NULL;
			ch[0] = ch[1] = NULL;
		}
		Node(const Node &other) {
			father = other.father;
			ch[0] = other.ch[0];
			ch[1] = other.ch[1];
			key = other.key;
			value = other.value;
		}
		Node(Node* fa, int id) {
			father = fa;
			ch[0] =  ch[1] = NULL;
			value = id;
			key = 0;
		}
	};
	vector<int>* edge;
	vector<Node*> pool;
	int vis[111111];
	void dfs(Node* from, Node* &a, Node *b) {
		if (b == NULL) {
			a = NULL;
			return;
		}
		a = new Node;
		a->father = from;
		a->key = b->key;
		a->value = b->value;
		dfs(a, a->ch[0], b->ch[0]);
		dfs(a, a->ch[0], b->ch[1]);
	}
	int isLeft(Node *x) {
		if (x->father == NULL) return 0;
		return x->father->ch[0] == x;
	}
	int isRight(Node *x) {
		if (x->father == NULL) return 0;
		return x->father->ch[1] == x;
	}

	void make_empty(Node *x) {
		if (x == NULL) {
			return;
		}
		make_empty(x->ch[0]);
		make_empty(x->ch[1]);
		delete x;
		x = NULL;
	}
	void dfs(int x) {
		//if (vis[x]) return;
		vis[x] = 1;
		tour.push_back(x);
		for (int i = 0; i < (int)edge[x].size(); i++) {
			dfs(edge[x][i]);
		}
		tour.push_back(-x);
	}

	void build(Node* fa, Node* &x, int l, int r) {
		if (l>r) return;
		int mid = l + r >> 1;
		x = new Node(fa, tour[mid]);
		pool.push_back(x);
		//cout << tour[mid] << "**" << endl;
		if (tour[mid] > 0) Left[tour[mid]] = x; else Right[-tour[mid]] = x;
		build(x, x->ch[0], l, mid - 1);
		build(x, x->ch[1], mid + 1, r);
	}
public:
	Node **Left, **Right;
	vector<int> tour;

	SplayTree() {
	}
	~SplayTree() {
		for (int i = 0; i < (int)pool.size(); i++) {
			delete pool[i];
		}
		pool.clear();
	}
	void init(int n) {
		edge = new vector<int>[n + 1];
		Left = new Node*[n + 1];
		Right = new Node*[n + 1];
		for (int i = 1; i <= n; i++) {
			edge[i].clear();
			vis[i] = 0;
		}
	}
	void add(int x, int y) {
		edge[x].push_back(y);
		vis[y] = 1;
	}
	void rotate(Node *x, int d) {
		Node *y = x->father;
		y->ch[!d] = x->ch[d];
		if (x->ch[d] != NULL) {
			x->ch[d]->father = y;
		}
		if (y->father != NULL) {
			y->father->ch[isRight(y)] = x;
		}
		x->father = y->father;
		y->father = x;
		x->ch[d] = y;
	}

	void splay(Node *x, Node* target) {
		if (x == NULL) return;
		for (; x->father != target; ) {
			if (x->father->father == target) {
				rotate(x, isLeft(x));
				return;
			}
			int whichx = isLeft(x);
			int whichfx = isLeft(x->father);
			if (whichx ^ whichfx) {
				rotate(x, whichx);
				rotate(x, whichfx);
			} else {
				rotate(x->father, whichfx);
				rotate(x, whichx);
			}
		}
	}

	int find(int x) {
		Node* y = Left[x];
		splay(y, NULL);
		if (y == NULL) {
			return 0;
		}
		for (; y->ch[0] != NULL; y = y->ch[0]);
		return y->value;
	}

	void move(int x, int y)	{
		if (x == y) {
			return;	
		}
		Node *l = Left[x], *r = Right[x];
		splay(l, NULL);
		splay(r, l);
		//puts("ok");
		Node *ll = l->ch[0];
		Node *rr = r->ch[1];
		l->ch[0] = NULL;
		r->ch[1] = NULL;
		Node *tmp = ll;
		//puts("ok2");
		for (; tmp != NULL && tmp->ch[1] != NULL; tmp = tmp->ch[1]);
		if (tmp != NULL) {
			tmp->ch[1] = rr;
		}
		//puts("ok3");
		if (ll != NULL) ll->father = NULL;
		if (rr != NULL) rr->father = tmp;
		if (!y) {
			splay(rr, NULL);
			return;
		}
		//puts("ok4");
		if (find(y) == x) {
			//cout << x << " " << y << endl;
			if (rr != NULL) rr->father = r;
			if (ll != NULL) ll->father = l;
			l->ch[0] = ll;
			r->ch[1] = rr;
			if (tmp != NULL) tmp->ch[1] = NULL;
			return;
		
		}
		Node* positionL = Left[y];
		splay(positionL, NULL);
		Node* next = positionL->ch[1];
		for (; next != NULL && next->ch[0] != NULL; next = next->ch[0]);
		splay(next, positionL);
		l->father = next;
		next->ch[0] = l;
	}
	void build(int n) {
		tour.clear();
		for (int i = 1; i <= n; i++) {
			if (!vis[i]) {
				dfs(i);
			}
		}
		/*for (int i = 0; i < 2 * n - 1; i++) {
			cout << tour[i] << " ";
		}*/
		//cout << endl;
		Node *root;
		for (int i = 0, j = 0, k = 0; i < tour.size(); i++) {
			k += tour[i] > 0 ? 1 : -1;
			if (!k) {
				build(NULL, root, j, i), j = i + 1;
			}
		}
		/*if (Left[2]->ch[0] == Left[1] && Left[2]->ch[1] == Right[2] && Right[2]->ch[1] == Right[1]) {
			cout << "ok" << endl;
		} else {
			cout << "fxxk" << endl;
		}*/
	}
};

void solve(int n) {
	static bool flag = 0;
	SplayTree solver;
	if (flag) {
		puts("");
	}
	flag = 1;
	solver.init(n);
	for (int i = 1; i <= n; i++) {
		int x;
		scanf("%d", &x);
		if (x != 0) {
			solver.add(x, i);
		}
	}
	solver.build(n);
/*	for (int i = 0; i < 2 * n; i++) {
	//	cout << i << " == " << (solver.Left[i] == NULL ? "boom" : "ok") << endl; 
		cout << solver.tour[i] << " ";
	}
	cout << endl;*/
	int m;
	scanf("%d", &m);
	for (int i = 1; i <= m; i++) {
		static char operation[11];
		getchar();
		scanf("%s", operation);
		if (operation[0] == 'Q') {
			int x;
			scanf("%d", &x);
			printf("%d\n", solver.find(x));
		} else {
			int x, y;
			scanf("%d%d", &x, &y);
			solver.move(x, y);
		}
	}

}
int main() {
	for (int n; scanf("%d", &n) == 1; solve(n));
	return 0;

}