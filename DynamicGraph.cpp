#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cassert>
using namespace std;

template<typename Node>
class Treap
{
public:
    static int size(Node* t) { return t == NULL ? 0 : t->size; }

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
            if(l->priority < r->priority)
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
                num |= 1;
                if(!tmp)
                {
                    t = l;
                    l = l->parent;
                    break;
                }
                r = tmp;
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
        return make_pair(l, r);
    }

    pair<Node*, Node*> split2(Node* t) {
        Node *p;
        Node *l = t->left;
        Node *r = t->right;
        Node::cut(l);
        Node::cut(r);
        t->linklr(NULL, NULL);
        while(t->parent)
        {
            p = t->parent;
            t->parent = NULL;
            if(p->left == t) r = p->linkl(r);
            else l = p->linkr(l);
            t = p;
        }
        return make_pair(l, r);
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

    int arc1(int ei) const { return ei; }
    int arc2(int ei) const { return ei + (numV() - 1); }
    int numV() const {
        return (int)firstArc.size();
    }
    int numE() const {
        return numV() - 1;
    }

    struct Node
    {
        Node *left, *right, *parent;
        int size, priority;
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
                mm |= left->marks;
            }
            if(right)
            {
                sz += right->size;
                mm |= right->marks;
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
        static Node *cut(Node *t)
        {
            if(t) t->parent = NULL;
            return t;
        }
        static const Node *findRoot(const Node *t)
        {
            while(t->parent) t = t->parent;
            return t;
        }
        static Node *findHead(Node *t)
        {
            while(t->left) t = t->left;
            return t;
        }
        static void updatePath(Node *t)
        {
            while(t)
            {
                t->update();
                t = t->parent;
            }
        }
    };

    Treap<Node> bst;
    vector<Node> nodes;
    int getArc(const Node *p) const {
        return p - &nodes[0];
    }

    typedef Treap<Node> mTreap;


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
        bool operator==(const Tree &other) const { return root == other.root; }
        bool operator!=(const Tree &other) const { return root != other.root; }
        bool isIsolated() const { return root == NULL; }
    };

    Tree getRoot(int v) const {
        int a = firstArc[v];
        return Tree(a == -1 ? NULL : Node::findRoot(&nodes[a]));
    }

    int getSize(Tree t) {
        if (t.isIsolated()) return 1;
        return (t.root)->size / 2 + 1;
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
        if(u > v) swap(u, v);
        int uu = firstArc[u], vv = firstArc[v];
        Node *l, *m, *r;
        if(uu != -1)
        {
            pair<Node*, Node*> p = bst.split(&nodes[uu]);
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
            pair<Node*, Node*> p = bst.split(&nodes[vv]);
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

    template<typename Type>
    bool searchEdges(Tree tree, Type toDo) const {
        return searchMark<0,Type>(tree, toDo);
    }
    template<typename Type>
    bool searchVertex(Tree tree, Type toDo) const {
        return searchMark<1,Type>(tree, toDo);
    }

    template<int Mark, typename Type>
    bool searchMark(Tree tree, Type toDo) const {
        if(tree.isIsolated()) return true;
        const Node *t = tree.root;
        if(t->markUnions >> Mark & 1)
            return checkTree<Mark,Type>(t, toDo);
        else
            return true;
    }
 
    template<int Mark, typename Type>
    bool checkTree(const Node *t, Type toDo) const {
        const Node *l = t->left, *r = t->right;
        if(l && (l->markUnions >> Mark & 1))
            if(!checkTree<Mark,Type>(l, toDo)) return false;
        if(t->marks >> Mark & 1)
            if(!toDo(getArc(t))) return false;
        if(r && (r->markUnions >> Mark & 1))
            if(!checkTree<Mark,Type>(r, toDo)) return false;
        return true;
    }
};

class DynamicConnectivity
{

private:
    typedef Forest::Tree Tree;
    int E;
    int NV;
    vector< vector<int> > firstE;
    vector<int> nextE, prevE;
    
    vector<char> level;
    vector<Forest> forests;
    int LEVEL;
    vector<int> from;
    vector<int> treeEdgeIndex;
    vector<int> treeEdgeMap;
    vector<int> freeList;
    
    vector<bool> vis;

    vector<int> edges;
    
    int arc1(int ei) { return ei;}
    int arc2(int ei) { return ei + numE();}
    int numV() const {
        return NV;
    }
    int numE() const {
        return level.size();
    }

    int arcEdge(int i) { return (i >= numE() ?  i - numE() : i);}
    bool replace(int lv, int v, int w) {
        Forest &forest = forests[lv];
        Tree vRoot = forest.getRoot(v), wRoot = forest.getRoot(w);
 
        int vSize = forest.getSize(vRoot), wSize = forest.getSize(wRoot);
 
        int u; Tree uRoot; int uSize;
        if(vSize <= wSize)
            u = v, uRoot = vRoot, uSize = vSize;
        else
            u = w, uRoot = wRoot, uSize = wSize;
 
        int replacement = -1;
        const int Samplings = LEVEL * 10;
        //随机取样一些边进行检查，可以提高效率        
        searchIncident(forest, uRoot, u, lv, FindReplacement(uRoot, &replacement));
        if(replacement != -1 && (int)edges.size() + 1 <= Samplings) {
            deleteOrdinaryEdge(replacement);
            addTreeEdge(replacement);
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
            level[e]++;
            addOrdinaryEdge(e);
        }
        edges.clear();
 
    

        //利用mark在树上进行跳跃，降低复杂度
        forest.searchEdges(uRoot, SearchTreeEdges(this));

        for(int i = 0; i < (int)edges.size(); i ++) {
            int t = edges[i];
            int e = treeEdgeMap[t];
            int v = from[arc2(e)], w = from[arc1(e)];
            int lv = level[e];
 
            level[e] = lv + 1;

            forests[lv].changeEMark(t, false);
            forests[lv + 1].changeEMark(t, true);
            forests[lv + 1].link(t, v, w);
        }
        edges.clear();
 
        if(replacement != -1) {
            deleteOrdinaryEdge(replacement);
            addTreeEdge(replacement);
            return true;
        } else
        if(lv > 0) {
            return replace(lv - 1, v, w);
        }else {
            return false;
        }
    }

    struct SearchTreeEdges {
        DynamicConnectivity *thisp;
        SearchTreeEdges(DynamicConnectivity *thisp): thisp(thisp) { }
 
        inline bool operator()(int a) {
            thisp->searchTreeEdges(a);
            return true;
        }
    };
    void searchTreeEdges(int t) {
        edges.push_back(t);
    }
 
    template<typename Type>
    bool searchIncident(Forest &forest, Tree t, int u, int lv, Type toDo) {
        if(t.isIsolated())
            return searchByVertex<Type>(lv, u, toDo);
        else
            return forest.searchVertex(t, SearchIncident<Type>(this, lv, toDo));
    }
 
    template<typename Type>
    struct SearchIncident {
        DynamicConnectivity *thisp;
        int lv;
        Type toDo;
 
        SearchIncident(DynamicConnectivity *thisp, int lv, Type toDo):
            thisp(thisp), lv(lv), toDo(toDo) { }
 
        inline bool operator()(int tii) const {
            return thisp->searchByArc(tii, lv, toDo);
        }
    };
 
    template<typename Type>
    bool searchByArc(int tii, int lv, Type toDo) {
        bool dir = tii >= numV() - 1;
        int ti = dir ? tii - (numV() - 1) : tii;
        int ei = treeEdgeMap[ti];
        int v = from[arc2(ei)], w = from[arc1(ei)];
        int u = !(dir != (v > w)) ? v : w; //由边的编号得到出点
        return searchByVertex(lv, u, toDo);
    }
    template<typename Type>
    bool searchByVertex(int lv, int u, Type toDo) {
       for (int now = firstE[lv][u]; now != -1; now = nextE[now]) {
            if (!toDo(this, now)) {
                return false;
            }
        }
        return true;
    }
 
    struct FindReplacement {
        Tree uRoot;
        int *replacement;
        FindReplacement(Tree uRoot, int *replacement):
            uRoot(uRoot), replacement(replacement) {}
 
        inline bool operator()(DynamicConnectivity *thisp, int a) const {
            return thisp->findReplacement(a, uRoot, replacement);
        }
    };

    bool findReplacement(int a, Tree uRoot, int *replacement) {
        int ei = arcEdge(a);
        if(vis[ei]) return true;
 
        int lv = level[ei];
        Tree hRoot = forests[lv].getRoot(from[a]);
        
        if(hRoot.isIsolated() || hRoot != uRoot) {
            *replacement = ei;
            return false;
        }
        vis[ei] = true;
        edges.push_back(ei);
        return true;
    }
 
    void addTreeEdge(int ei)
    {
        int lv = level[ei];
        int u = from[arc2(ei)], v = from[arc1(ei)];
        int ti = freeList.back();
        freeList.pop_back();
        treeEdgeIndex[ei] = ti;
        treeEdgeMap[ti] = ei;
        forests[lv].changeEMark(ti, 1);
        for(int i = 0; i <= lv; ++i) forests[i].link(ti, u, v);
    }
    void addOrdinaryEdge(int ei)
    {
        int u = from[arc2(ei)], v = from[arc1(ei)];
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

    void deleteOrdinaryEdge(int e) {
        int u = from[arc2(e)], v = from[arc1(e)];
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

        t = arc2(e);
        next = nextE[t], prevE[next] = prev;
        nextE[t] = prevE[t] = -2;
        if (next != -1) prevE[next] = prev;
        if (prev != -1) nextE[prev] = next;
        else firstE[lv][v] = next;

        if (next == -1 && prev == -1) {
            forests[lv].changeVMark(u, false);
        }
    }
public:
    void init(int N, int tot)
    {
        NV = N;
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
        
        treeEdgeIndex.assign(E, -1);
        treeEdgeMap.assign(N - 1, -1);
 
        from.assign(tot * 2, -1);
        vis.assign(tot, 0);
        
        freeList.resize(N - 1);
        for(int i = 0; i < N - 1; ++i) freeList[i] = i;
    }
    bool addEdge(int ei, int u, int v)
    {
        int a1 = arc1(ei), a2 = arc2(ei);
        from[a1] = v, from[a2] = u;
        
        bool treeEdge = !forests[0].connected(u, v);
        level[ei] = 0;
        if(treeEdge) addTreeEdge(ei);
        else
        {
            treeEdgeIndex[ei] = -1;
            if(u != v) addOrdinaryEdge(ei);
        }
        
        return treeEdge;
    }
    bool deleteEdge(int e) {
        int a1 = arc1(e), a2 = arc2(e);
        int v = from[a2], w = from[a1]; 
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
                deleteOrdinaryEdge(e);
            }
        }
        from[a1] = from[a2] = -1;
        level[e] = -1;
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
        ind[i].resize(i);
        for(int j = 0; j < i; ++j)
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
        if(S < T) swap(S, T);
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