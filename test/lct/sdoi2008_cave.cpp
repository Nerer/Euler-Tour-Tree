#include "lct.hpp"
#include <cstdio>

using namespace std;
using namespace sjtu;

const int MAXN = 10000;

struct point_info
{
	int idx;
	point_info() = default;
	point_info(int idx) : idx(idx) {}
	void split(point_info &a, point_info &b) {}
	void merge(const point_info &a, const point_info &b) {}
};

LCT<point_info, int> lct;
LCT<point_info, int>::point pt[MAXN+1];
int N, M;

int main()
{
	freopen("cave.in", "r", stdin);
	freopen("cave.out", "w", stdout);
	scanf("%d %d", &N, &M);
	for (int i = 1; i <= N; i++)
		pt[i] = lct.insert(point_info(i));
	for (int i = 1; i <= M; i++)
	{
		char op[20];
		int u, v;
		scanf("%s %d %d", op, &u, &v);
		if (op[0] == 'Q')
		{
			printf(lct.get_root(pt[u]) == lct.get_root(pt[v]) ? "Yes\n" : "No\n");
		}
		else if (op[0] == 'C')
		{
			lct.link(pt[u], pt[v], 0);
		}
		else if (op[0] == 'D')
		{
			lct.cut(lct.get_lca(pt[u], pt[v]) == pt[u] ? pt[v] : pt[u]);
		}
	}
	return 0;
}