#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
using namespace llvm;

/**
 * 输出LLVM函数，主要是输出其中BB的流程
*/
void dispLLVMFunction(const Function*function, ostream&os){
    os<<"-------LLVM Function-------"<<endl;
    os<<"functions name: "<<(string)function->getName()<<" ";
    os<<"arg count: "<<function->arg_size()<<endl;
    
    /// 获取所有BB的信息, 为所有BB编一个号
    unordered_map<const BasicBlock*, int> bb2id;
    for(const BasicBlock &bb: make_range(function->begin(), function->end())){
        bb2id.insert(make_pair(&bb, bb2id.size()+1));
    }    
    /// 输出BB的前驱和后继
    os<<"BB: "<<function->size()<<endl;
    for(const BasicBlock &bb: make_range(function->begin(), function->end())){
        os<<bb2id[&bb]<<":";
        for(const BasicBlock *pb: successors(&bb)){ // 注意这里迭代的是指针，参数也是指针
            os<<" "<<bb2id[pb];
        }
        os<<endl;
    }
}
/**
 * 输出LLVM模块，主要是输出所含函数
*/
void dispLLVMModule(const Module*module, ostream&os){
    os<<"==========LLVM Module========="<<endl;
    /// 输出模块名称
    os<<"module name: "<<(string)module->getName()<<endl;
    /// 输出模块中的函数
    os<<"Functions: "<<module->size()<<endl;
    for(const Function &func: module->functions()){ // 注意迭代的是引用而不是指针
        dispLLVMFunction(&func, os);
    }    
}

/**
 * 首先编辑一个c文件，例如x.c
 * 然后调用lxllvm.sh，格式为lxllvm.sh x.c
 * 会得到x文件夹，内有x.c,x.ll,x.bc等文件
 * 再调用本程序,./bin/main x
 * 会进入x文件夹，分析相应的bc文件 
*/
int main(int argc, char **argv){
    /// 判断并获取参数
    assert(argc>=2);
    string prefix(argv[1]);
    string fullName("./"+prefix+"/"+prefix+".bc");

    /// 从gc文件读取LLVM模块
    static LLVMContext MyGlobalContext;
    LLVMContext *context = &MyGlobalContext;
    SMDiagnostic err;
    unique_ptr<Module> uModule = parseIRFile(fullName, err, *context);
    Module *module = uModule.get();   

    /// 显示输出LLVM模块
    dispLLVMModule(module, cout);
    return 0;
}