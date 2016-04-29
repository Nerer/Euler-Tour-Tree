#include <cstdio>
#include "dynamic_tree.hpp"

using namespace std;
using namespace sjtu;

const int MAXN = 100000;

struct node
{
	long long num, sum;
	long long add_flag;
	int size;
	bool non_empty;

	node() : num(0), sum(0), add_flag(0), size(0), non_empty(false) {}
	node(int num) : num(num), sum(num), add_flag(0), size(1), non_empty(true) {}

	void set_add(long long delta)
	{
		sum += delta * size;
		add_flag += delta;
		if (non_empty)
			num += delta;
	}
	void split(node &a, node &b)
	{
		if (add_flag)
		{
			a.set_add(add_flag);
			b.set_add(add_flag);
			add_flag = 0;
		}
	}
	void merge(const node &a, const node &b)
	{
		assert(add_flag == 0);
		sum = a.sum + b.sum + num;
		size = a.size + b.size + non_empty;
	}

	void dump() {}
};

dynamic_tree<node>::iterator pt[MAXN + 1];
dynamic_tree<node> dtree;
int N, M;

int main()
{
	freopen("dtree.in", "r", stdin);
	freopen("dtree.out", "w", stdout);
	scanf("%d %d", &N, &M);
	for (int i = 0; i < N; i++)
		pt[i] = dtree.insert();
	for (int kase = 1; kase <= M; kase++)
	{
		if (kase % 5000 == 0 && M >= 10000)
			fprintf(stderr, "%.3lf%%\n", kase * 100.0 / M);

		int op, u, v, val;
		long long ans = 0;
		scanf("%d", &op);
		switch (op)
		{
		case 1: //link
			scanf("%d %d %d", &u, &v, &val);
			dtree.link(pt[u], pt[v], val);
			break;
		case 2:	//cut
			scanf("%d", &u);
			dtree.cut(pt[u]);
			break;
		case 3:	//evert
			scanf("%d", &u);
			dtree.evert(pt[u]);
			break;
		case 4:	//modify path
			scanf("%d %d %d", &u, &v, &val);
			dtree.modify_path(pt[u], pt[v], [val](node &e) { e.set_add(val); });
			break;
		case 5:	//query path
			scanf("%d %d", &u, &v);
			ans = dtree.query_path(pt[u], pt[v]).sum;
			printf("%lld\n", ans);
			break;
		case 6:	//modify subtree
			scanf("%d %d", &u, &val);
			dtree.modify_subtree(pt[u], [val](node &e) { e.set_add(val); });
			break;
		case 7:	//query subtree
			scanf("%d", &u);
			ans = dtree.query_subtree(pt[u]).sum;
			printf("%lld\n", ans);
		}
	}
	return 0;
}