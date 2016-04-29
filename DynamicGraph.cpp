#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
using namespace std;

template<typename Node>
class Treap
{
public:
    Node *join(Node *l, Node *r)
    {
        if(!l) return r;
        if(!r) return l;
        Node *t = NULL;
        
        int h;
        long long num = 0;
        for(h = 0; ; ++h)
        {
            if(h >= sizeof(num) * 8 - 2)//QAQAQ
            {
                t = join(l->right, r->left);
                num = num << 2 | 1;
                ++h;
                break;
            }
            num <<= 1;
            if(l->priority < r->prioirity)
            {
                Node *tmp = l->right;
                if(!tmp)
                {
                    t = r;
                    r = r->parent;
                    break;
                }
                l = tmp;
            }
            else
            {
                Node *tmp = r->left;
                if(!tmp)
                {
                    t = l;
                    l = l->parent;
                    break;
                }
                r = tmp;
                num |= 1;
            }
        }
        for(; h >= 0; --h)
        {
            if(!(num & 1))
            {
                Node *p = l->parent;
                t = l->linkr(t);
                l = p;
            }
            else
            {
                Node *p = r->parent;
                t = r->linkl(t);
                r = p;
            }
            num >>= 1;
        }
        
        return t;
    }
    //cut the line between t->left and t
    pair<Node*, Node*> split(Node* t)
    {
        Node *p;
        Node *l = t->left;
        Node *r = t;
        Node::cut(l);
        t->linkl(NULL);
        while(t->parent)
        {
            p = t->parent;
            t->parent = NULL;
            if(p->left == t) r = p->linkl(r);
            else l = p->linkr(l);
            t = p;
        }
        return pair(l, r);
    }

    pair<Node*, Node*> split(Node* t) {
        Node *p;
        Node *l = t->left;
        Node *r = t->right;
        Node::cut(l);
        t->linkl(NULL);
        while(t->parent)
        {
            p = t->parent;
            t->parent = NULL;
            if(p->left == t) r = p->linkl(r);
            else l = p->linkr(l);
            t = p;
        }
        return pair(l, r);
    }

    Node* comb(Node* h, Node* t)
    {
        if(!t) return h;
        Node* tmp = NULL;
        while(1)
        {
            if(h->priority < t->priority)
            {
                Node* p = t->parent;
                tmp = h->linkr(t);
                t = p;
                break;
            }
            Node* l = t->left;
            if(!l) {tmp = h; break;}
            t = l;
        }
        while(t)
        {
            tmp = t->linkl(tmp); t = t->parent;
        }
        return tmp;
    }
};



class Forest
{
public:
    vector<int> firstArc;
    vector<bool> EMark, VMark;

    int numV() const {
        return (int)firstArc.size();
    }
    int numE() const {
        return numV() - 1;
    }
    
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
            if(c) left = c, c->parent = this;
            return update();
        }
        Node *linkr(Node *c)
        {
            if(c) right = c, c->parent = this;
            return update();
        }
        Node *linklr(Node *l, Node *r)
        {
            if(l) left = l, l->parent = this;
            if(r) right = r, r->parent = this;
            return update();
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
    };

    Node::Treap<Node> bst;
    vector<Node> nodes;
    int getArc(Node *p) {
        return p - &nodes[0];
    }

    typedef Node::Treap<Node> mTreap;


    bool getEMark(int a) const {
        return a < numE() ? EMark[a] : false;
    }
    bool getVMark(int v) const {
        return VMark[v];
    }

    void updateMarks(int a, int v) {
        Node* t = &nodes[a];
        t->marks = getEMark(a) | (getVMark(v) << 1);
        Node::updatePath(t);
    }
    void firstArcChanged(int v, int a, int b) {
        if(a != -1) updateMarks(a, v);
        if(b != -1) updateMarks(b, v);
    }


    class Tree {
    public:
        const Node *root;
        Tree() { }
        Tree(const Node *root): root(root) { }
        bool operator==(const Tree &other) const { return ref == that.ref; }
        bool operator!=(const Tree &that) const { return ref != that.ref; }
        bool isIsolated() const { return root == NULL; }
    };

    Tree getRoot(int v) const {
        int a = firstArc[v];
        return Tree(a == -1 ? NULL : Node::findRoot(&nodes[a]));
    }

    int getSize(Tree t) {
        if (t.isIsolated()) return 1;
        return t.root->size() / 2 + 1;
    }
 
    
public:
    void init(int N)
    {
        int M = N - 1;
        firstArc.assign(N, -1);
        
        nodes.assign(M * 2, Node());
        for(int i = 0; i < M * 2; ++i) nodes[i].priority = rand();
        EMark.assign(M, 0);
        VMark.assign(N, 0);
    }
    void link(int ti, int u, int v)
    {
        int a1 = arc1(ti), a2 = arc2(ti);
        if(u > v) swap(a1, a2);
        int uu = firstArc[u], vv = firstArc[v];
        
        Node *l, *m, *r;
        if(uu != -1)
        {
            pair<Node*, Node*> p = bst.split(&node[uu]);
            m = bst.join(p.second, p.first);
        }
        else
        {
            m = NULL;
            firstArc[u] = a1;
            firstArcChanged(u, -1, a1);
        }
        if(vv != -1)
        {
            pair<Node*, Node*> p = bst.split(&node[vv]);
            l = p.first, r = p.second;
        }
        else
        {
            l = r = NULL;
            firstArc[v] = a2;
            firstArcChanged(v, -1, a2);
        }
        
        m = bst.comb(&nodes[a2], m);
        r = bst.comb(&nodes[a1], r);
        
        bst.join(bst.join(l, m), r);
    }

    void cut(int which, int u, int v) {
        if(u > v) {
            swap(u, v);
        }
 
        int a1 = arc1(which), a2 = arc2(which);
        pair<Node*, Node*> p = bst.split2(&nodes[a1]);
        int rsize = mTreap::size(p.second);
        pair<Node*, Node*> q = bst.split2(&nodes[a2]);


        Node *l, *m, *r;
        if(p.second == &nodes[a2] || mTreap::size(p.second) != rsize) { // 后继在右子树
            l = p.first, m = q.first, r = q.second;
        }else { //后继是父亲
            swap(u, v);
            swap(a1, a2);
            l = q.first, m = q.second, r = p.second;
        }
 
        if(firstArc[u] == a1) {
            int b;
            if(r != NULL) {
                b = getArc(Node::findHead(r));
            }else {
                b = (l == NULL ? -1 : getArc(Node::findHead(l)));
            }
            firstArc[u] = b;
            firstArcChanged(u, a1, b);
        }
        if(firstArc[v] == a2) {
            int b = (m == NULL ? -1 : getArc(Node::findHead(m)));
            firstArc[v] = b;
            firstArcChanged(v, a2, b);
        }
 
        bst.join(l, r);
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
};

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
    
    vector<bool> vis;

    vector<int> edges;
    
    int arc1(int ei) { return ei;}
    int arc2(int ei) { return ei + E;}
    int arcEdge(int i) { return (i >= E ?  i - E : i);}

    bool replace(int lv, int v, int w) {
        Forest &forest = forests[lv];
        Tree vRoot = forest.getRoot(v), wRoot = forest.getRoot(w);
 
        int vSize = forest.getSize(vRoot), wSize = forest.getSize(wRoot);
 
        int u; Tree uRoot; int uSize;
        if(vSize <= wSize)
            u = v, uRoot = vRoot, uSize = vSize;
        else
            u = w, uRoot = wRoot, uSize = wSize;
 
        int replacementEdge = -1;

        //随机取样一些边进行检查，可以提高效率        
        forceSearch(forest, uRoot, u, lv);
        if(replacementEdge != -1 && (int)edges.size() + 1 <= Samplings) {
            deleteOrdinaryEdge(replacementEdge);
            addTreeEdge(replacementEdge);
            for(int i = 0; i < (int)edges.size(); i ++) {
                vis[edges[i]] = false;
            }

            edges.clear();
            return true;
        }
        
        for(int i = 0; i < (int)edges.size(); i ++) {
            int e = edges[i];
            vis[e] = false;
            deleteOrdinaryEdge(e);
            Level[e]++;
            insertOrdinaryEdge(e);
        }
        visitedEdges.clear();
 
    

        //利用mark在树上进行跳跃，降低复杂度
        forest.searchEdges(uRoot);

        for(int i = 0; i < (int)edges.size(); i ++) {
            int t = edges[i];
            int e = treeEdgeMap[t];
            int v = arcHead[arc2(e)], w = arcHead[arc1(e)];
            int lv = Level[e];
 
            Level[e] = lv + 1;

            forests[lv].changeEMark(t, false);
            forests[lv+1].changeEMark(t, true);
            forests[lv+1].link(t, v, w);
        }
        edges.clear();
 
        if(replacementEdge != -1) {
            deleteNontreeEdge(replacementEdge);
            addTreeEdge(replacementEdge);
            return true;
        } else
        if(lv > 0) {
            return replace(lv-1, v, w);
        }else {
            return false;
        }
    }
 
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

    void deleteOrdinaryEdge(int e, int u, int v) {
        int lv = level[e];
        int t = arc1(e);
        int next = nextE[t], prev = prevE[t];
        nextE[t] = prevE[t] = -2;
 
        if(next != -1) prevE[next] = prev;
        if(prev != -1) nextE[prev] = next;
        else firstE[lv][u] = next;
 
        if(next == -1 && prev == -1) {
            forests[lv].changeVMark(v, false);
        }

        int t = arc2(e);
        next = nextE[t], prevE[next] = prev;
        nextE[t] = prevE[t] = -2;
        if (next != -1) prevE[next] = prev;
        if (prev != -1) nextE[prev] = next;
        else firestE[lv][v] = next;

        if (next == -1 && prev == -1) {
            forests[lv].changeVMark(u, false);
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
    bool deleteEdge(int e) {
        int a1 = arc1(e), a2 = arc2(e);
        int v = arcHead[a2], w = arcHead[a1]; 
        int lv = level[e];
        int which = treeEdgeIndex[e];
 
        bool splitted = false;
        if(which != -1) {
            treeEdgeMap[which] = -1;
            treeEdgeIndex[e] = -1;
            freeList.push_back(which);
            for(int i = 0; i <= lv; i ++)
                forests[i].cut(which, v, w);
 
            forests[lv].changeEMark(which, false);
            splitted = !replace(lv, v, w);
        }else {
            if(v != w) {
                deleteOrdinaryEdge(e, v, w);
            }
        }
        arcHead[a1] = arcHead[a2] = -1;
        level[ei] = -1;
        return splitted;
    }
};

int main()
{
    srand(time(0));
    
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