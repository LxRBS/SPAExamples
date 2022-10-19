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

static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
        llvm::cl::desc("<input bitcode>"), llvm::cl::init("-"));

int main(int argc, char ** argv){
    int arg_num = 0;
    char **arg_value = new char*[argc];
    std::vector<std::string> moduleNameVec;
    LLVMUtil::processArguments(argc, argv, arg_num, arg_value, moduleNameVec);
    cl::ParseCommandLineOptions(arg_num, arg_value, "Whole Program Points-to Analysis\n");

    SVFModule* svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);
    svfModule->buildSymbolTableInfo();   

    SVFIRBuilder builder;
    auto * pag = builder.build(svfModule);

    Andersen* ander = AndersenWaveDiff::createAndersenWaveDiff(pag);

    SVFGBuilder svfBuilder(true);
    auto * svfg = svfBuilder.buildFullSVFG(ander);
    return 0;
}

