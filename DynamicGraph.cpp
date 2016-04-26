#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
using namespace std;

class SplayTree{};


class Forest
{
private:
    vector<int> firstArc;
    
    vector<bool> EMark, VMark;
    int arc1(int ei) { return ei;}
    int arc2(int ei) { return ei + firstArc.size() - 1;}
    
    struct Node
    {
        Node *left, *right, *parent;
        int size, priroity;
        char marks, markUnions;
        
        Node()
        {
            left = NULL, right = NULL, parent = NULL;
            size = 1, priority = 0, marks = 0, markUnions = 0;
        }
        
        Node *update()
        {
            int sz = 1;
            int mm = marks;
            if(left)
            {
                sz += left->size;
                mm |= left->mm;
            }
            if(right)
            {
                sz += right->size;
                mm |= right->mm;
            }
            size = sz, markUnions = mm;
            return this;
        }
        Node *linkl(Node *c)
        {
        }
        Node *linkr(Node *c)
        {
        }
        Node *linklr(Node *l, Node *r)
        {
        }
        Node *cut(Node *t)
        {
            if(t) t->parent = NULL;
            return t;
        }
        Node *findRoot(Node *t)
        {
            while(t->parent) t = t->parent;
            return t;
        }
        Node *findHead(Node *t)
        {
            while(t->left) t = t->left;
            return t;
        }
        void updatePath(Node *t)
        {
            while(t)
            {
                t->update();
                t = t->parent;
            }
        }
    }
    Node::SplayTree<Node> bst;
    vector<Node> nodes;
    
public:
    void init(int N)
    {
        int M = N - 1;
        firstArc.assign(N, -1);
        
        nodes.assign(M * 2, Node());
        for(int i = 0; i < M * 2; ++i) nodes[i].priority = bst.nextRand();//QAQAQ
        EMark.assign(M, 0);
        VMark.assign(N, 0);
    }
    void link(int ti, int u, int v)
    {
    }
    bool connected(int u, int v)
    {
        if(u == v) return 1;
        int a = firstArc[u], b = firstArc[v];
        if(a == -1 || b == -1) return 0;
        return Node::findRoot(&nodes[a]) == Node::findRoot(&nodes[b]);
    }
    void changeEMark(int ti, bool b)
    {
        EMark[ti] = b;
        Node *t = &nodes[ti];
        t->marks = b | (t->marks & (1 << 1));
        Node::updatePath(t);
    }
    void changeVMark(int v, bool b)
    {
        VMark[v] = b;
        int a = firstArc[v];
        if(a != -1)
        {
            Node *t = &nodes[a];
            t->marks = (t->marks & 1) | (b << 1);
            Node::updatePath(t);
        }
    }
}

class DynamicConnectivity
{
private:
    int E;
    vector< vector<int> > firstE;
    vector<int> nextE, prevE;
    
    vector<char> level;
    vector<Forest> forests;
    int LEVEL;
    
    vector<int> treeEdgeIndex;
    vector<int> treeEdgeMap;
    vector<int> freeList;
    
    vctor<bool> vis;
    
    int arc1(int ei) { return ei;}
    int arc2(int ei) { return ei + E;}
    int arcEdge(int i) { return (i >= E ?  i - E : i);}
    
    void addTreeEdge(int ei, int u, int v)
    {
        int lv = level[ei];
        
        int ti = freeList.back();
        freeList.pop_back();
        treeEdgeIndex[ei] = ti;
        treeEdgeMap[ti] = ei;
        
        forests[lv].changeEMark(ti, 1);
        
        for(int i = level[ei]; i >= 0; --i) forests[i].link(ti, u, v);
    }
    void addOrdinaryEdge(int ei, int u, int v)
    {
        int lv = level[ei];//level[ei] = 0, always
        
        {
            int nx, a;
            nx = firstE[lv][u];
            a = arc1(ei);
            firstE[lv][u] = a;
            nextE[a] = nx;
            prevE[a] = -1;
            if(nx != -1) prevE[nx] = a;
            if(nx == -1) forests[lv].changeVMark(u, 1);
            
            nx = firstE[lv][v];
            a = arc2(ei);
            firstE[lv][v] = a;
            nextE[a] = nx;
            prevE[a] = -1;
            if(nx != -1) prevE[nx] = a;
            if(nx == -1) forests[lv].changeVMark(v, 1);
        }
    }
public:
    void init(int N, int tot)
    {
        E = tot;
        level.resize(tot);
        level.assign(tot, -1);
        LEVEL = 1;
        while(1 << LEVEL <= N / 2) LEVEL++;
        forests.resize(LEVEL);
        for(int i = 0; i < LEVEL; ++i) forests[i].init(N);
        firstE.resize(LEVEL);
        for(int i = 0; i < LEVEL; ++i) firstE[i].assign(N, -1);
        nextE.assign(tot * 2, -2);
        prevE.assign(tot * 2, -2);
        
        arcHead.assign(tot * 2, -2);
        vis.assign(tot, 0);
        
        freeList.resize(N);
        for(int i = 0; i < N; ++i) freeList[i] = i;
    }
    bool addEdge(int ei, int u, int v)
    {
        int a1 = arc1(ei), a2 = arc2(ei);
        arcHead[a1] = u, arcHead[a2] = v;
        
        bool treeEdge = !forests[0].connected(u, v);
        level[ei] = 0;
        if(treeEdge) addTreeEdge(ei, u, v);
        else
        {
            treeEdgeIndex[ei] = -1;
            if(u != v) addOrdinaryEdge(ei, u, v);
        }
        
        return treeEdge;
    }
    bool deleteEdge(int ei)
    {
    }
}

int main()
{
    int N, M;
    scanf("%d%d", &N, &M);
    
    //add id to all edges(exist or not exist)
    vector<vector<int> > ind(N);
    int tot = 0;
    for(int i = 0; i < N; ++i)
    {
        ind[i].resize(N - i);
        for(int j = i + 1; j < N; ++j)
            ind[i][j] = tot++;
    }
    
    DynamicConnectivity dc;
    dc.init(N, tot);
    
    vector<bool> EdgeExist(tot);
    int parts = N;
    for(int m = 1; m <= M; ++m)
    {
        int S, T;
        scanf("%d%d", &S, &T);
        --S, --T;
        if(S == T) continue;
        if(S > T) swap(S, T);
        int ei = ind[S][T];
        if(!EdgeExist[ei])
        {
            parts -= dc.addEdge(ei, S, T);
            EdgeExist[ei] = true;
        }
        else
        {
            parts += dc.deleteEdge(ei);
            EdgeExist[ei] = false;
        }
        
        printf("Number of Components = %d\n", parts);
    }
    
    
    return 0;
}