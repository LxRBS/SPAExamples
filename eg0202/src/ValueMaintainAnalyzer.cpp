#include <bits/stdc++.h>
#include "ValueMaintainAnalyzer.h"
#include "lxmsc.h"

using namespace std;
using namespace llvm;
using namespace SVF;

void ValueMaintainAnalyzer::init(){
    /// 为所有的BB编号，并且为所有的指令编号
    int kbb = 1, kinst = 1;
    for(auto &fun: fanwei(module->llvmFunBegin(), module->llvmFunEnd())){
        auto &s1 = fun->getBasicBlockList();
        for(auto &bb: s1){  // llvm迭代的一般是类本身，bb就是BB类型，不是指针
            this->bb2id.insert(mp(&bb, kbb++));
            /// 对bb中的每一条指令
            for(auto &inst: beend(&bb)){  // 迭代出来的是指令本身，所以要取地址
                this->inst2id.insert(mp(&inst, kinst++));
            }        
            /// 找entry节点, 如果没有前驱就是entry
            if(!bb.hasNPredecessorsOrMore(1)){
                if(entry != nullptr) throw runtime_error("More than one entry, WRONG!!!");
                entry = &bb;
            }    
        }
    }
    /// 将所有答案初始化为空
    for(auto &pp: this->bb2id){
        const BB *bb = pp.fi;
        this->bb2ans.insert(mp(bb, NodeBS()));
    }
}

void ValueMaintainAnalyzer::proc(){
    /// 计算entry节点的答案
    for(auto &inst: beend(this->entry)){
        /// 输出所有的inst
        if(this->debug){
            cout<<"("<<inst.getOpcode()<<", "<<(string)inst.getOpcodeName()<<endl;
        }
        switch(inst.getOpcode()){
            case Instruction::Store:{
                this->procStore(&inst, this->bb2ans[this->entry]);
            }break;
        }
    }
}

void ValueMaintainAnalyzer::report(ostream &os)const{
    for(auto &pp: this->bb2ans){
        const BB* bb = pp.fi;
        const AnsTy &ans = pp.se;
        os<<(string)bb->getName()<<": "<<endl;
        for(unsigned i: beend(&ans)){
            os<<" "<<i;
        }
        os<<endl;
    }    
}

void ValueMaintainAnalyzer::procStore(const Instruction *inst, AnsTy &ans){
    /// store op0, op1  op0是值，op1是地址
    const Value *op0 = inst->getOperand(0);
    const Value *op1 = inst->getOperand(1);
    /// op1被重新定义，原有的定义要被kill
    auto def2instIt = this->value2inst.find(op1);
    if(def2instIt != this->value2inst.end()){        
        this->inst2value.erase(def2instIt->se);
        ans.reset(this->inst2id[def2instIt->se]);
        this->value2inst.erase(def2instIt);
    }
    /// 重新定义
    this->value2inst.insert(mp(op1, inst));
    this->inst2value.insert(mp(inst, op1));
    /// 记录答案
    ans.set(this->inst2id[inst]);
}


