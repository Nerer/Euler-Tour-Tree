#include <cstdio>
#include <ctime>
#include <cassert>
#include <random>
#include <vector>
#include <set>
using namespace std;

const int MAXN = 10000;
const int MAXM = 100000;
const int RANGE = 500;

default_random_engine ran_engine;

struct vertex
{
	int depth;
	int parent, pa_edge;
	set<int> edges;
	long long sum;
};
struct edge
{
	int to;
	long long val;
};
struct Ttree
{
	int root;
	vector<int> points;
	Ttree() : root(-1) {}
	Ttree(int root, vector<int> points) : root(root), points(points) {}
};

int N, M;
vertex V[MAXN + 1];
edge E[4 * MAXN + 1];
vector<Ttree> forest;
int NE;

inline double plink()
{
	return double(forest.size() - 1) / (N - 1);
}
inline double pcut()
{
	return double(forest.size() - 1) / (2 * (N - 1));
}
inline int op_edge(int e)
{
	return ((e-1)^1)+1;
}
bool do_random(double p)
{
	uniform_real_distribution<double> real_rand(0.0, 1.0);
	return real_rand(ran_engine) < p;
}
int rand(int l, int r)
{
	uniform_int_distribution<int> int_rand(l, r);
	return int_rand(ran_engine);
}

void link(int u, int v, long long val)
{
	assert(NE + 2 <= 4 * MAXN);
	++NE;
	E[NE].to = v;
	E[NE].val = val;
	V[u].edges.insert(NE);
	++NE;
	E[NE].to = u;
	E[NE].val = val;
	V[v].edges.insert(NE);
}
void cut(int e)
{
	int e2 = ((e - 1) ^ 1) + 1;
	V[E[e2].to].edges.erase(e);
	V[E[e].to].edges.erase(e2);
	E[e].to = -1;
	E[e2].to = -1;
	E[e].val = 0;
	E[e2].val = 0;
}
void dfs(int idx, int fa, vector<int> &vset)
{
	vset.push_back(idx);
	if (fa == -1)
		V[idx].depth = 0, V[idx].parent = -1;
	else
		V[idx].depth = V[fa].depth + 1, V[idx].parent = fa;
	V[idx].sum = 0;
	for (int e : V[idx].edges)
	{
		if (E[e].to == fa)
		{
			V[idx].pa_edge = e;
			continue;
		}
		dfs(E[e].to, idx, vset);
		V[idx].sum += V[E[e].to].sum;
		V[idx].sum += E[e].val;
	}
}

int lca(int u, int v)
{
	if (V[u].depth < V[v].depth)
		return lca(v, u);
	while (V[u].depth > V[v].depth)
		u = V[u].parent;
	while (u != v)
	{
		u = V[u].parent;
		v = V[v].parent;
	}
	return u;
}
long long qpath(int u, int v)
{
	if (V[u].depth < V[v].depth)
		return qpath(v, u);
	long long ans = 0;
	while (V[u].depth > V[v].depth)
	{
		ans += E[V[u].pa_edge].val;
		u = V[u].parent;
	}
	while (u != v)
	{
		ans += E[V[u].pa_edge].val;
		u = V[u].parent;
		ans += E[V[v].pa_edge].val;
		v = V[v].parent;
	}
	return ans;
}
void mpath(int u, int v, long long delta)
{
	if (V[u].depth < V[v].depth)
		return mpath(v, u, delta);
	while (V[u].depth > V[v].depth)
	{
		E[V[u].pa_edge].val += delta;
		E[op_edge(V[u].pa_edge)].val += delta;
		u = V[u].parent;
	}
	while (u != v)
	{
		E[V[u].pa_edge].val += delta;
		E[op_edge(V[u].pa_edge)].val += delta;
		u = V[u].parent;
		E[V[v].pa_edge].val += delta;
		E[op_edge(V[v].pa_edge)].val += delta;
		v = V[v].parent;
	}
}
void msubtree(int idx, int fa, long long delta)
{
	for (int e : V[idx].edges)
	{
		if (E[e].to == fa)
			continue;
		E[e].val += delta;
		E[((e-1)^1)+1].val += delta;
		msubtree(E[e].to, idx, delta);
	}
}

int main()
{
	ran_engine.seed(time(0) + clock());
	FILE * fin = fopen("dtree.in", "w");
	FILE * fans = fopen("dtree.ans", "w");

	N = MAXN, M = MAXM;
	fprintf(fin, "%d %d\n", N, M);
	for (int i = 0; i < N; i++)
		forest.push_back(Ttree(i, { i }));
	for (int kase = 1; kase <= M; kase++)
	{
		if(kase % 5000 == 0 && M >= 10000)
			printf("%.3lf%%\n", kase * 100.0 / M);
		
		int tree = rand(0, forest.size() - 1);
		for (int i = 0; i < 10 && forest[tree].points.size() <= 1; i++)
			tree = rand(0, forest.size() - 1);

		if (forest[tree].points.size() <= 1 || do_random(plink()))
		{
			//link
			int subtree = rand(0, forest.size() - 1);
			tree = subtree;
			while (tree == subtree)
				tree = rand(0, forest.size() - 1);
			int parent = forest[tree].points.at(rand(0, forest[tree].points.size() - 1));
			int val = rand(0, 2 * RANGE);
			link(parent, forest[subtree].root, val);
			fprintf(fin, "1 %d %d %d\n", forest[subtree].root, parent, val);
			int old_size = forest[tree].points.size();
			forest[tree].points.clear();
			dfs(forest[tree].root, -1, forest[tree].points);
			assert(old_size + forest[subtree].points.size() == forest[tree].points.size());
			forest.erase(forest.begin() + subtree);
			continue;
		}
		
		vector<int> temp;
		dfs(forest[tree].root, -1, temp);
		
		if (do_random(pcut()))
		{
			//cut
			int top = forest[tree].points.at(rand(1, forest[tree].points.size() - 1));
			assert(top != forest[tree].root);
			cut(V[top].pa_edge);
			fprintf(fin, "2 %d\n", top);
			forest.push_back(Ttree(top, {}));
			int old_size = forest[tree].points.size();
			forest[tree].points.clear();
			dfs(forest[tree].root, -1, forest[tree].points);
			dfs(forest.back().root, -1, forest.back().points);
			assert(old_size == forest[tree].points.size() + forest.back().points.size());
			continue;
		}

		int op = rand(3, 7);
		if (op == 3)
		{
			//evert
			int top = forest[tree].points.at(rand(1, forest[tree].points.size() - 1));
			assert(top != forest[tree].root);
			fprintf(fin, "3 %d\n", top);
			int old_size = forest[tree].points.size();
			forest[tree].points.clear();
			forest[tree].root = top;
			dfs(forest[tree].root, -1, forest[tree].points);
			assert(old_size == forest[tree].points.size());
		}
		/*else if (op == 4)
		{
			//lca
			int u = forest[tree].points.at(rand(0, forest[tree].points.size() - 1));
			int v = forest[tree].points.at(rand(0, forest[tree].points.size() - 1));
			fprintf(fin, "4 %d %d\n", u, v);
			fprintf(fans, "%d\n", lca(u, v));
		}*/
		else if (op == 4)
		{
			//modify path
			int u = forest[tree].points.at(rand(0, forest[tree].points.size() - 1));
			int v = u;
			while (v == u)
				v = forest[tree].points.at(rand(0, forest[tree].points.size() - 1));
			int delta = rand(-RANGE, RANGE);
			mpath(u, v, delta);
			fprintf(fin, "4 %d %d %d\n", u, v, delta);
			vector<int> tmp;
			dfs(forest[tree].root, -1, tmp);
		}
		else if (op == 5)
		{
			//query path
			int u = forest[tree].points.at(rand(0, forest[tree].points.size() - 1));
			int v = u;
			while (v == u)
				v = forest[tree].points.at(rand(0, forest[tree].points.size() - 1));
			long long ans = qpath(u, v);
			fprintf(fin, "5 %d %d\n", u, v);
			//printf("LCA OF %d and %d is %d\n", u, v, lca(u, v));
			fprintf(fans, "%lld\n", ans);
		}
		else if (op == 6)
		{
			//modify subtree
			int u = forest[tree].points.at(rand(0, forest[tree].points.size() - 1));
			int delta = rand(-RANGE, RANGE);
			msubtree(u, V[u].parent, delta);
			fprintf(fin, "6 %d %d\n", u, delta);
			vector<int> tmp;
			dfs(forest[tree].root, -1, tmp);
		}
		else
		{
			//query subtree
			int u = forest[tree].points.at(rand(0, forest[tree].points.size() - 1));
			long long ans = V[u].sum;
			fprintf(fin, "7 %d\n", u);
			fprintf(fans, "%lld\n", ans);
		}
	}
	return 0;
}
