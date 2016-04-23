#include <cstdio>
#include <cstring>
#include <algorithm>
#include "splay.hpp"
using namespace std;
using namespace sjtu;

const int inf = 1000000000;

struct ninfo
{
	int num;
	int size;
	long long sum;
	long long presum, sufsum, maxsum;
	int maxval;
	bool same;
	int same_num;

	ninfo() : size(0), num(0), sum(0), presum(0), sufsum(0), maxsum(0), same(false), same_num(0), maxval(-inf) {}
	ninfo(int x) : num(x), size(1), sum(x), same(false), same_num(0), maxval(x)
	{
		if (x > 0)
			presum = sufsum = maxsum = x;
		else
			presum = sufsum = maxsum = 0;
	}
	void make_same(int x)
	{
		if (size == 0)
			return;
		num = x;
		maxval = x;
		sum = long long(x) * size;
		if (x > 0)
			presum = sufsum = maxsum = sum;
		else
			presum = sufsum = maxsum = 0;
		same = true;
		same_num = x;
	}

	void split(ninfo &a, ninfo &b)
	{
		if (same)
		{
			a.make_same(same_num);
			b.make_same(same_num);
			same = false;
			same_num = 0;
		}
	}
	void merge(const ninfo &a, const ninfo &b, bool ra, bool rb)
	{
		assert(!same);
		size = a.size + b.size + 1;
		maxval = std::max(num, std::max(a.maxval, b.maxval));
		sum = a.sum + b.sum + num;
		long long pa = a.presum, pb = b.presum, sa = a.sufsum, sb = b.sufsum;
		if (ra)
			std::swap(pa, sa);
		if (rb)
			std::swap(pb, sb);
		presum = std::max(pa, a.sum + num + pb);
		sufsum = std::max(sb, b.sum + num + sa);
		maxsum = std::max(sa + pb + num, std::max(a.maxsum, b.maxsum));
	}
};

splay<ninfo> sp;
int N, M;

/*void dump(splay<ninfo> &SP)
{
	printf("SIZE=%d, DATA: ", SP.size());
	for (auto i : SP)
		printf("%d ", i.num);
	printf("\n");
}*/

int main()
{
	freopen("sequence.in", "r", stdin);
	freopen("sequence.out", "w", stdout);
	scanf("%d %d", &N, &M);
	for (int i = 0; i < N; i++)
	{
		int num;
		scanf("%d", &num);
		sp.insert(ninfo(num));
	}
	//dump(sp);
	int size = N;
	for (int kase = 0; kase < M; kase++)
	{
		char op[20];
		scanf("%s", op);
		if (strcmp(op, "INSERT") == 0)
		{
			int pos, tot;
			scanf("%d %d", &pos, &tot);
			size += tot;
			auto iter = sp.find_node(pos);
			splay<ninfo> tmp;
			for (int i = 0; i < tot; i++)
			{
				int num;
				scanf("%d", &num);
				tmp.insert(ninfo(num));
			}
			sp.merge(tmp, iter);
		}
		else if (strcmp(op, "DELETE") == 0)
		{
			int pos, tot;
			scanf("%d %d", &pos, &tot);
			size -= tot;
			if (tot == 0)
				continue;
			auto iter = sp.find_node(pos - 1);
			auto iter_end = sp.find_node(pos + tot - 1);
			splay<ninfo> tmp = sp.split(iter_end);
			sp.split(iter);
			sp.merge(tmp);
		}
		else if (strcmp(op, "MAKE-SAME") == 0)
		{
			int pos, tot, c;
			scanf("%d %d %d", &pos, &tot, &c);
			if (tot == 0)
				continue;
			auto iter = sp.find_node(pos - 1);
			auto iter_end = sp.find_node(pos + tot - 1);
			splay<ninfo> tmp1 = sp.split(iter_end);
			splay<ninfo> tmp2 = sp.split(iter);
			tmp2.begin()->make_same(c);
			sp.merge(tmp2).merge(tmp1);
		}
		else if (strcmp(op, "REVERSE") == 0)
		{
			int pos, tot;
			scanf("%d %d", &pos, &tot);
			if (tot == 0)
				continue;
			auto iter = sp.find_node(pos - 1);
			auto iter_end = sp.find_node(pos + tot - 1);
			splay<ninfo> tmp1 = sp.split(iter_end);
			splay<ninfo> tmp2 = sp.split(iter);
			tmp2.reverse();
			sp.merge(tmp2).merge(tmp1);
		}
		else if (strcmp(op, "GET-SUM") == 0)
		{
			int pos, tot;
			scanf("%d %d", &pos, &tot);
			if (tot == 0)
			{
				printf("0\n");
				continue;
			}
			auto iter = sp.find_node(pos - 1);
			auto iter_end = sp.find_node(pos + tot - 1);
			splay<ninfo> tmp1 = sp.split(iter_end);
			splay<ninfo> tmp2 = sp.split(iter);
			//dump(tmp2);
			printf("%lld\n", tmp2.begin()->sum);
			sp.merge(tmp2).merge(tmp1);
		}
		else if (strcmp(op, "MAX-SUM") == 0)
		{
			long long ans = sp.begin()->maxsum;
			if (ans == 0)
				printf("%d\n", sp.begin()->maxval);
			else
				printf("%lld\n", ans);
		}
		assert(sp.size() == size);

		//dump(sp);
	}
	return 0;
}