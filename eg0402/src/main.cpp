#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include "Graphs/SVFG.h"
#include "MSSA/SVFGBuilder.h"
#include "SVF-FE/LLVMModule.h"
#include "SVF-FE/LLVMUtil.h"
#include "SVF-FE/SVFIRBuilder.h"
#include "WPA/Andersen.h"

using namespace llvm;
using namespace SVF;

int main(int argc, char ** argv){
    assert(argc >= 2);
    string prefix(argv[1]);
    string fullName("./" + prefix + "/" + prefix + ".bc");
    /// SVF可以将多个bc文件生成SVF模块，这里只写一个bc文件
    vector<string> vec(1, fullName);
    auto *svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(vec);
    svfModule->buildSymbolTableInfo();
    /// 得到PAG的建造器，并且建造PAG
    /// PAG在新版本中改名为SVFIR
    SVFIRBuilder builder;
    auto * pag = builder.build(svfModule);
    /// 创建andersen指针分析, 创建内部就包含了analyze，因此无需显式调用
    auto * ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    /// 生成稀疏值流图
    SVFGBuilder svfBuilder(true);
    auto * svfg = svfBuilder.buildFullSVFG(ander);
    /// 输出MSSA
    svfg->getMSSA()->dumpMSSA();
    /// 应该释放内存，懒得释放了，小小泄漏一点没事 
    return 0;
}

