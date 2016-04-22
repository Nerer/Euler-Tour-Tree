#include <iostream>
#include <alghorithm>
#include <cstring>
#include <cstdio>
#include <vector>

using namespace std;

class SplayTree{
private:
	class Node{
	public:
		Node* father, ch[2];
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
	vector<int> tour;
	int vis[111111];
	Node** Left, Right;
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
		if (vis[x]) return;
		tour.push_back(x);
		for (int i = 0; i < (int)edge[x].size(); i++) {
			dfs(edge[x][i]);
		}
		tour.push_back(-x);
	}

	void build(Node* fa, Node* &x, int l, int r) {
		if (l>r) return;
		int mid = l + r >> 1;
		x = new Node(fa, dfs[mid]);
		if (dfs[mid] > 0) Left[dfs[mid]] = x; else Right[-dfs[mid]] = x;
		build(x, x->ch[0], l, mid - 1);
		build(x, x->ch[1], mid + 1, r);
	}
public:
	SplayTree() {
		root = NULL;
	}
	SplayTree(const SplayTree &other) {
		dfs(root, other.root);
	}
	SplayTree &operator = (const SplayTree &other) {
		(*this).~SplayTree();
		dfs(root, other.root);
	}
	~SplayTree() {
		make_empty(root);
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
		for (; x->father != target; ) {
			if (x->father->father == target) {
				rotate(x, isLeft(x));
				return;
			}
			int whichx = isLeft(x);
			int whichfx = isLeft(x->father);
			if (whichx ^ whichfx) {
				rotate(x, whichx);
				rotate(x, whichz);
			} else {
				rotate(x->father, whichfx);
				rotate(x->father, whichfx);
			}
		}
	}
	int find(int x) {
		Node* y = node[Left[x]];
		splay(y, NULL);
		for (; y->ch[0] != NULL; y = y->ch[0]);
		return y->value;
	}
	void move(int x, int y)	{
		if (x == y) {
			return;	
		}
		Node* l = node[Left[x]], r = node[Right[x]];
		splay(l, 0);
		splay(r, l);
		Node *ll = l->ch[0];
		Node *rr = r->ch[1];
		l->ch[0] = NULL;
		r->ch[1] = NULL;
		Node *tmp = ll->ch[1];
		for (; tmp != NULL && tmp->ch[1] != NULL; tmp = tmp->ch[1]);
		if (tmp != NULL) {
			tmp->ch[1] = rr;
		}
		rr->father = tmp;
		if (!y) {
			splay(rr, NULL);
			return;
		}
		if (find(y) == x) {
			rr->father = r;
			ll->father = l;
			l->ch[0] = ll;
			r->ch[1] = rr;
			return;
		
		}
		Node* positionL = node[Left[y]];
		splay(positionL, NULL);
		Node* next = positionL->ch[1];
		for (; next != NULL && next->ch[0] != NULL; next = next->ch[0]);
		splay(next, positionL);
		l->father = next;
		next->ch[0] = l;
	}
	void build() {
		tour.clear();
		for (int i = 1; i <= n; i++) {
			dfs(i);
		}
		Node *root;
		for (int i = 0, j = 0, k = 0; i < dfs.size(); i++)
		{
			k += dfs[i]>0 ? 1 : -1;
			if (!k) build(0, root, j, i), j = i + 1;
		}
	}
}solver;

void solve() {
	solver.init(n);
	for (int i = 1; i <= n; i++) {
		int x;
		scanf("%d", &x);
		if (x != 0) {
			solver.add(x, i);
		}
	}
	solver.build();
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
	for (; scanf("%d", &n) == 1; solve());
	return 0;

}