#include <bits/stdc++.h>
using namespace std;

#include "Util/BasicTypes.h"
#include "Util/SVFUtil.h"
#include <Graphs/GenericGraph.h>
using namespace SVF;
using namespace llvm;

#include "LxBBGraph.h"

/**
 * 给定SVF模块，测试自生成的BB流程图
 * @param module: SVF模块
 * @param preifx: 文件夹的名字，同时也是ll文件和bc文件的文件本名
*/
void testBBs(const SVFModule *module, string const&prefix){
    string path = "./"+prefix+"/";
    ofstream out(path+"bb.txt");    

    /// 生成并输出BB图
    LxBBGraph* g = LxBBGraph::newInstance(module);
    g->dump(path+prefix+"_bb");
    delete g;
}

/**
 * @brief 给定SVF模块，生成指定函数的支配树和支配边界
 * @param module 
 * @param prefix 
 * @param Function 
 */
void testDomTree(const SVFModule * module, string const & prefix, const SVFFunction * f){
    string path = "./"+prefix+"/";

    /// 生成并输出BB图
    LxBBGraph * g = LxBBGraph::newInstance(module);
    g->dump(path+prefix+"_bb");

    DominatorTree dt;   // 定义支配树
    dt.recalculate(*f->getLLVMFun()); // 计算支配树

    /// 打印支配树
    dt.print(llvm::outs());

    /// 生成支配树的图
    LxBBGraph * tg = LxBBGraph::newInstance(dt, g);
    tg->dump(path+prefix+"_dt");

    /// 生成并显示支配边界
    DominanceFrontier df;
    df.analyze(dt);

    /// 打印支配边界
    llvm::outs() << "dt df: \n";
    df.print(llvm::outs());

    std::map<const llvm::BasicBlock *, int> mm;
    for(const auto & p : llvm::make_range(g->begin(), g->end())) mm.insert({p.second->getBB(), p.first});

    fprintf(stdout, "dt df with ID: \n");
    for(const auto & p : llvm::make_range(df.begin(), df.end())){
        auto bb = p.first;
        fprintf(stdout, "BB %d:", mm.find(bb)->second);
        for(const auto & b : p.second){
            fprintf(stdout, "%5d", mm.find(b)->second);
        }
        fprintf(stdout, "\n");
    }

    /// 生成逆支配树
    PostDominatorTree pdt;
    pdt.recalculate(*f->getLLVMFun()); 
    
    llvm::outs() << "pdt df: \n";
    pdt.print(llvm::outs());    
    
    LxBBGraph * ptg = LxBBGraph::newInstance(pdt, g);
    ptg->dump(path+prefix+"_pdt");



    delete g;
    delete tg;
    delete ptg;
}

/**
 * 首先编辑出一个.c文件，例如叫做x.c
 * 然后调用lxllvm.sh，格式为：lxllvm.sh x.c
 * 调用完成后，会建立一个名为x文件夹，内部有x.c, x.ll, x.bc, x.opt文件
 * 再调用本程序，格式为: ./main x 
 * 会进入x文件夹，分析相应的x.bc文件，输出文件也在x文件夹中
*/
int main(int argc, char **argv){
    assert(argc>=2);
    string prefix(argv[1]);
	string fullName("./"+prefix+"/"+prefix+".bc");
    /// SVF可以将多个bc文件生成SVF模块，这里只写一个bc文件
    vector<string> vec(1, fullName);
    SVFModule* svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(vec);

    /// 生成支配树，函数名固定为f
    for(auto * fun : llvm::make_range(svfModule->begin(), svfModule->end())){
        if(fun and fun->getName() == "f"){
            testDomTree(svfModule, prefix, fun);
        }
    }
    return 0;
}