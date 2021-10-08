#include <bits/stdc++.h>
#include "SignAnalyzer.h"
#include "lxmsc.h"

using namespace std;
using namespace llvm;
using namespace SVF;

void SignAnalyzer::init(){
    /// 首先从SVFmodule中获取所有的模块，并为其编号
    this->initBBs();
    /// 从符号表中取出所有的变量
    this->initValues();
}

void SignAnalyzer::proc(){
    while(true){
        bool flag = false;
        /// 对每一个BB依次计算
        for(auto pp: this->bb2id){
            const BB* bb = pp.fi;
            /// 首先根据前驱的BB计算IN值，IN值等于所有前驱的并
            for(auto pred: predecessors(bb)){
                flag = this->uniteTo(pred, bb) || flag;
            }
            /// 再根据bb自身的指令计算OUT值
            flag = this->generate(bb) || flag;
        }
        if(this->debug){
            this->report(cout);
            int tmp;
            cin>>tmp;
        }
        if(!flag) break;
    }   
}

void SignAnalyzer::report(ostream &os)const{
    const vector<string> &vec = SignAnalyzer::getSignKindStrings();

    for(auto pp: this->bb2id){
        const BB *bb = pp.fi;
        os<<"===  "<<(string)bb->getName()<<": "<<endl;
        /// 输出符号
        const VariableAnsMapTy &ans = this->bb2ans.find(bb)->se;
        for(auto tt: ans){
            const Value *value = tt.fi;
            if(value->hasName()){
                os<<(string)value->getName()<<": "<<vec[tt.se]<<endl;
            }
        }
    }
}

/// 为所有的BB编号，从1开始
void SignAnalyzer::initBBs(){
    int k = 1;
    for(auto &fun: fanwei(module->llvmFunBegin(), module->llvmFunEnd())){
        auto &s1 = fun->getBasicBlockList();
        for(auto &bb: s1){  // llvm迭代的一般是类本身，bb就是BB类型，不是指针
            this->bb2id.insert(mp(&bb, k++));
        }
    }
}

/// 从符号表中获取所有的变量，并且把每个BB都附着一个记录答案的map
void SignAnalyzer::initValues(){
    /// 先做一个去常转型，否则下一句不能通过const检测
    SymbolTableInfo *tabletmp = const_cast<SymbolTableInfo*>(this->table);
    /// m1是一个<const Value*, ID>的map
    const SymbolTableInfo::ValueToIDMapTy &m1 = tabletmp->valSyms();   
    VariableAnsMapTy ret;
    for(auto pp: m1){
        ret.insert(mp(pp.fi, INI));
    } 
    /// 每个BB均保留一个答案记录
    for(auto pp: this->bb2id){
        this->bb2ans.insert(mp(pp.fi, ret));
    }
}

/// 将src的答案合并到tgt上
bool SignAnalyzer::uniteTo(const BB *src, const BB *tgt){
    if(this->debug){
        cout<<"In funcion: "<<__func__;
        cout<<"("<<(string)src->getName()<<", "<<(string)tgt->getName()<<")"<<endl;   
    }
    bool ret = false;
    /// 取出答案的数据结构
    VariableAnsMapTy &from = this->bb2ans[src];
    VariableAnsMapTy &to = this->bb2ans[tgt];
    /// 对每一个变量的答案
    for(auto &pp: to){  // 使用引用，要修改值
        SignK tmp = SignAnalyzer::unite(from[pp.fi], pp.se);
        if(tmp != pp.se){
            if(this->debug){
                cout<<"value update occured: unite("<<from[pp.fi]<<", "<<pp.se<<"), ";
                cout<<"("<<pp.fi<<", "<<(string)pp.fi->getName()<<")";
                cout<<", oldvalue = "<<pp.se<<", newvalue = "<<tmp<<endl;
                
            }            
            pp.se = tmp;
            ret = true;
            if(this->debug){
                cout<<"after: "<<to[pp.fi]<<endl;
            }
        }
    }
    return ret;
}

/// 根据BB的指令计算答案
bool SignAnalyzer::generate(const BB *bb){
    if(this->debug){
        cout<<"In funcion: "<<__func__;
        cout<<"("<<(string)bb->getName()<<")"<<endl;
    }
    bool flag = false;
    VariableAnsMapTy &ans = this->bb2ans[bb];  
    for(auto &inst: beend(bb)){  // 迭代出来的是指令本身，所以要取地址
        flag = this->generate(&inst, ans) || flag;
    }
    if(this->debug){
        cout<<"return: "<<flag<<endl;
    }
    return flag;
}

/**
 * 根据指令，更新ans中的内容
 * 指令的枚举即Instruction:Store等的定义在
 * llvm/IR/Instruction.def
 * 文件中
*/
bool SignAnalyzer::generate(const Instruction *inst, VariableAnsMapTy &ans){
    bool flag = false;
    switch(inst->getOpcode()){
        case Instruction::Store:{ // store 值, 目标，值有可能是常数，也有可能是变量
            const Value *op0 = inst->getOperand(0);
            const Value *op1 = inst->getOperand(1);
            flag = this->genStore(op0, op1, ans) || flag;
        }break;
        case Instruction::Load:{  // user = load op0
            const Value *op0 = inst->getOperand(0);
            const User *user = inst->getOperandUse(0).getUser();  
            flag = this->genLoad(user, op0, ans) || flag;
        }break;
        case Instruction::Add:{  // user = add op0, op1
            const Value *op0 = inst->getOperand(0);
            const Value *op1 = inst->getOperand(1);
            const User *user = inst->getOperandUse(0).getUser(); 
            flag = this->genAdd(user, op0, op1, ans) || flag;
        }break;
        case Instruction::SDiv:{
            const Value *op0 = inst->getOperand(0);
            const Value *op1 = inst->getOperand(1);
            const User *user = inst->getOperandUse(0).getUser(); 
            flag = this->genSdiv(user, op0, op1, ans) || flag;
        }break;
    }
    return flag;
}

/// 处理store指令，store op0, op1， op0有可能是常数，op1必然是地址
bool SignAnalyzer::genStore(const Value *op0, const Value *op1, VariableAnsMapTy &ans){
    SignK ret = this->getSign(op0, ans);
    SignK &target = ans[op1];
    return ret != target ? (target=ret, true) : false;
}

/// user = load op0，op0是地址
bool SignAnalyzer::genLoad(const User *user, const Value *op0, VariableAnsMapTy &ans){
    SignK ret = ans[op0];
    SignK &target = ans[user];
    return ret != target ? (target=ret, true) : false;
}

/// user = add op0, op1
bool SignAnalyzer::genAdd(const User *user, const Value *op0, const Value *op1, VariableAnsMapTy &ans){
    SignK a = this->getSign(op0, ans);
    SignK b = this->getSign(op1, ans);
    SignK ret = INI;
    switch(a){
        case POS:ret = (POS==b||ZER==b) ? POS : UNK;break;
        case ZER:ret = b;break;
        case NEG:ret = (NEG==b||ZER==b) ? NEG : UNK;break;
        case UNK:ret = UNK;break;
    }
    SignK &target = ans[user];
    return ret != target ? (target=ret, true) : false;
}

/// user = sdiv op0, op1
bool SignAnalyzer::genSdiv(const User *user, const Value *op0, const Value *op1, VariableAnsMapTy &ans){
    SignK a = this->getSign(op0, ans);
    SignK b = this->getSign(op1, ans);
    SignK ret = INI;
    switch(a){
        case POS:ret = (POS==b||NEG==b) ? b : UNK;break;
        case ZER:ret = ZER;break;
        case NEG:ret = NEG==b ? POS : (POS==b ? NEG : UNK);break;
        case UNK:ret = UNK;break;
    }
    SignK &target = ans[user];
    return ret != target ? (target=ret, true) : false;
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