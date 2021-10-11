#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include "Util/BasicTypes.h"
#include "Util/SCC.h"
#include "Util/SVFUtil.h"
#include <Graphs/GenericGraph.h>
using namespace SVF;
using namespace llvm;

#include "ValueMaintainAnalyzer.h"

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
    SymbolTableInfo *symbolTableInfo = SymbolTableInfo::Symbolnfo();

    string signAnaName("./"+prefix+"/"+prefix+"_valuemaintiananalysis.txt");
    ofstream out(signAnaName);
    ValueMaintainAnalyzer analyzer(svfModule, symbolTableInfo);
    analyzer.report(out);
    out.close();
    return 0;
}
