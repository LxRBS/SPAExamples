#include <bits/stdc++.h>
using namespace std;

#include "LxGraph.h"
using namespace SVF;

#include <llvm/Support/GenericDomTree.h>
using namespace llvm;

int main(){
    /// 生成图
    auto * g = LxGraph::getInstanceOne();
    /// 支配树
    using DomTree = DominatorTreeBase<LxNode, false>;
    DomTree dt;
    dt.reset();
    dt.recalculate(*g);
    auto * root = dt.getRoot();
    cout << "root = " << root->getId() << endl;
    return 0;
}