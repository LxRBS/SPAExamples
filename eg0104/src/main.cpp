#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include "Util/BasicTypes.h"
#include "Util/SCC.h"
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
    testBBs(svfModule, prefix);
    return 0;
}