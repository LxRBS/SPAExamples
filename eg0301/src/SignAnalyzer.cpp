#include <bits/stdc++.h>
#include "SignAnalyzer.h"

using namespace std;
using namespace llvm;
using namespace SVF;

void SignAnalyzer::init(){
    /// 首先为所有的BB编号，这一步其实可以不需要，主要是为了调试方便
    int k = 1;
    for(auto fun: make_range(this->module->llvmFunBegin(), this->module->llvmFunEnd())){
        auto &s1 = fun->getBasicBlockList();
        for(auto &bb: s1){ // llvm迭代的一般是类本身，bb就是BB类型，不是指针
            this->bb2id.insert({&bb, k++});
        }
    }

    /// 初始化记录分析结果的数据结构
    FlowAnalysisDataTy ret;
    this->initialize(ret);

    /// 每个BB均保留一个答案记录
    for(auto &pp: this->bb2id){
        this->bb2ans.insert({pp.first, ret}); // 注意C++的语义本身就是深拷贝，在Java或者python中要写成ret.copy()
    }    
}

void SignAnalyzer::proc(){
    for(this->iteration_count=1;;++this->iteration_count){
        bool flag = false;
        /// 对每一个BB依次计算
        for(auto &pp: this->bb2id){
            const BB *bb = pp.first;
            flag = this->transfer(bb) || flag;
        }
        if(!flag) break;
    }   
}

/// 根据BB和in计算BB的out
bool SignAnalyzer::transfer(const BB *bb){
    if(this->debug){
        cout<<"In funcion: "<<__func__;
        cout<<"("<<(string)bb->getName()<<")"<<endl;
    }

    /// 首先计算所有前驱的合并
    FlowAnalysisDataTy inout;
    for(auto pred: predecessors(bb)){
        this->merge(this->bb2ans[pred], inout);
    }

    /// 再依次对每条指令计算gen和kill
    for(auto &inst: make_range(bb->begin(), bb->end())){  // 迭代出来的是指令本身，所以要取地址
        this->transfer(&inst, inout);
    }
    
    FlowAnalysisDataTy &ans = this->bb2ans[bb];
    if(this->equal(ans, inout)) return false;

    ans = inout; // 这里应该是深拷贝，C++本身就是如此
    return true;
}

/// 根据指令计算数据流分析的结果，指令的枚举即Instruction:Store等的定义在llvm/IR/Instruction.def
void SignAnalyzer::transfer(const Instruction *inst, FlowAnalysisDataTy &inout){
    switch(inst->getOpcode()){
        case Instruction::Store:{ 
            this->doStore((const StoreInst*)inst, inout);
        }break;
        case Instruction::Load:{  // user = load op0
            this->doLoad((const LoadInst*)inst, inout);
        }break;
        case Instruction::Add:{  // user = add op0, op1
            this->doAdd((const BinaryOperator*)inst, inout);
        }break;
        case Instruction::SDiv:{
            this->doSDiv((const BinaryOperator*)inst, inout);
        }break;
        default:{
            ;
        }
    }
}

/// 处理store指令，store op0, op1， op0有可能是常数，op1必然是地址
void SignAnalyzer::doStore(const StoreInst *inst, FlowAnalysisDataTy &inout){
    const Value *op0 = inst->getOperand(0); // 取出op0和op1
    const Value *op1 = inst->getOperand(1);    
    SignK ret = this->getSign(op0, inout);  // 检查op0的符号
    auto it = inout.find(op1);
    if(inout.end() == it){ // 如果op1本来不存在，直接将op0的符号作为op1的符号记录
        inout.insert(it, {op1, ret});
    }else{ // 如果op1本来就有符号，需要执行一个unite操作
        it->second = this->unite(it->second, ret);
    }
}

/// user = load op0，op0是地址
void SignAnalyzer::doLoad(const LoadInst *inst, FlowAnalysisDataTy &inout){
    const Value *op0 = inst->getOperand(0);
    const User *user = inst->getOperandUse(0).getUser(); 
    auto it = inout.find(op0);    
    SignK ret = it != inout.end() ? it->second : INI;
    it = inout.find(user);
    if(inout.end() == it){
        inout.insert(it, {user, ret});
    }else{
        it->second = this->unite(it->second, ret);
    }    
}

/// user = add op0, op1
void SignAnalyzer::doAdd(const BinaryOperator *inst, FlowAnalysisDataTy &inout){
    const Value *op0 = inst->getOperand(0);
    const Value *op1 = inst->getOperand(1);
    const User *user = inst->getOperandUse(0).getUser(); 
    SignK a = this->getSign(op0, inout);
    SignK b = this->getSign(op1, inout);
    SignK ret = INI;
    switch(a){
        case POS:ret = (POS==b||ZER==b) ? POS : UNK;break;
        case ZER:ret = b;break;
        case NEG:ret = (NEG==b||ZER==b) ? NEG : UNK;break;
        case UNK:ret = UNK;break;
    }

    auto it = inout.find(user);
    if(inout.end() == it){
        inout.insert(it, {user, ret});
    }else{
        it->second = this->unite(it->second, ret);
    } 
}

/// user = sdiv op0, op1
void SignAnalyzer::doSDiv(const BinaryOperator *inst, FlowAnalysisDataTy &inout){
    const Value *op0 = inst->getOperand(0);
    const Value *op1 = inst->getOperand(1);
    const User *user = inst->getOperandUse(0).getUser(); 
    SignK a = this->getSign(op0, inout);
    SignK b = this->getSign(op1, inout);
    SignK ret = INI;
    switch(a){
        case POS:ret = (POS==b||NEG==b) ? b : UNK;break;
        case ZER:ret = ZER;break;
        case NEG:ret = NEG==b ? POS : (POS==b ? NEG : UNK);break;
        case UNK:ret = UNK;break;
    }

    auto it = inout.find(user);
    if(inout.end() == it){
        inout.insert(it, {user, ret});
    }else{
        it->second = this->unite(it->second, ret);
    } 
}

void SignAnalyzer::initialize(FlowAnalysisDataTy &d){
    d.clear();
    /// 先做一个去常转型，否则下一句不能通过const检测
    SymbolTableInfo *tabletmp = const_cast<SymbolTableInfo*>(this->table);
    /// m1是一个<const Value*, ID>的map
    const SymbolTableInfo::ValueToIDMapTy &m1 = tabletmp->valSyms();   
    for(auto &pp: m1){ // 每一个变量的初始状态都是初始化INI
        d.insert({pp.first, INI});
    } 
    return;
}

/// 将src的内容合并到tgt上
void SignAnalyzer::merge(const FlowAnalysisDataTy &src, FlowAnalysisDataTy &tgt){
    for(const auto &pp: src){
        auto it = tgt.find(pp.first);
        if(tgt.end() == it){ // 如果tgt中本来没有，直接插入
            tgt.insert(it, pp);
        }else{ // 否则等于两者之和，这个和由unite函数定义
            it->second = this->unite(it->second, pp.second); 
        }
    }     
}

bool SignAnalyzer::equal(const FlowAnalysisDataTy &src, const FlowAnalysisDataTy &tgt){
    if(src.size() != tgt.size()) return false;

    for(const auto &pp: src){
        auto it = tgt.find(pp.first);
        if(tgt.end() == it || pp.second != it->second) return false;
    }
    return true;
}

void SignAnalyzer::report(ostream &os)const{
    os<<"Total iteration count: "<<this->iteration_count<<endl;
    const vector<string> &vec = SignAnalyzer::getSignKindStrings();

    for(auto pp: this->bb2id){
        const BB *bb = pp.first;
        os<<"===  "<<(string)bb->getName()<<": "<<endl;
        /// 输出符号
        const FlowAnalysisDataTy &ans = this->bb2ans.find(bb)->second;
        for(auto tt: ans){
            const Value *value = tt.first;
            if(value->hasName()){
                os<<(string)value->getName()<<": "<<vec[tt.second]<<endl;
            }
        }
    }
}

const vector<string>&  SignAnalyzer::getSignKindStrings(){
    static const int n = 5;
    static const string a[n] = {
        "INI", "POS", "NEG", "ZER", "UNK"
    };
    static const vector<string> vec(a, a+n);
    return vec;
}

string SignAnalyzer::getSignKindString(SignK k){
    return SignAnalyzer::getSignKindStrings()[k];
}