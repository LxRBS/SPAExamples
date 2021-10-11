#include <bits/stdc++.h>
#include "ValueMaintainAnalyzer.h"

using namespace std;
using namespace llvm;
using namespace SVF;

void ValueMaintainAnalyzer::init(){
    /// 首先为所有的BB编号，这一步其实可以不需要，主要是为了调试方便
    int k = 1, ink = 1;
    for(auto &fun: make_range(this->module->llvmFunBegin(), this->module->llvmFunEnd())){
        auto &s1 = fun->getBasicBlockList();
        for(auto &bb: s1){ // llvm迭代的一般是类本身，bb就是BB类型，不是指针
            this->bb2id.insert({&bb, k++});

            /// 还要为bb中的每一条指令进行编号
            for(auto &inst: make_range(bb.begin(), bb.end())){ // 同理，这里迭代出来的是指令本身，使用的时候要取地址
                this->inst2id.insert({&inst, ink});
                this->id2inst.insert({ink, &inst});
                ++ink;
            }
        }
    }

    /// 初始化记录分析结果的数据结构
    FlowAnalysisDataTy ret;
    this->initialize(ret);

    /// 每个BB均保留一个答案记录
    for(auto &pp: this->bb2id){
        this->bb2ans.insert({pp.first, ret}); 
    } 

    /// 初始化inst2value和value2insts
    for(const auto &pp: this->id2inst){
        NodeID id = pp.first;
        const llvm::Instruction *inst = pp.second;

        switch(inst->getOpcode()){
            case Instruction::Store:{ // store op0, op1， 该语句定义了op1                
                const Value *op1 = ((const llvm::StoreInst*)inst)->getOperand(1);
                this->value2insts[op1].set(id);
                this->inst2value.insert({id, op1});                  
            }break;
            case Instruction::Load:{ // user = load op0, 该语句定义了user
                const User *user = ((const LoadInst*)inst)->getOperandUse(0).getUser();
                this->value2insts[user].set(id);
                this->inst2value.insert({id, user});
            }break;
            case Instruction::Add:{ // user = add op0, op1, 该语句定义了user
                const User *user = ((const BinaryOperator*)inst)->getOperandUse(0).getUser(); 
                this->value2insts[user].set(id);
                this->inst2value.insert({id, user});
            }break;
        }
    }
}

void ValueMaintainAnalyzer::proc(){
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
bool ValueMaintainAnalyzer::transfer(const BB *bb){
    if(this->debug){
        cout<<"In funcion: "<<__func__;
        cout<<"("<<(string)bb->getName()<<")"<<endl;
    }

    /// 首先计算所有前驱的合并
    FlowAnalysisDataTy inout;
    for(auto pred: predecessors(bb)){
        this->merge(this->bb2ans[pred], inout);
    }

    if(this->debug){
        cout<<"输入:";
        for(auto i: inout){
            cout<<" "<<i;
        }
        cout<<endl;
    }

    /// 再依次对每条指令计算gen和kill
    for(auto &inst: make_range(bb->begin(), bb->end())){  // 迭代出来的是指令本身，所以要取地址
        this->transfer(&inst, inout);
    }
    
    if(this->debug){
        cout<<"输出:";
        for(auto i: inout){
            cout<<" "<<i;
        }
        cout<<endl;
    }

    FlowAnalysisDataTy &ans = this->bb2ans[bb];
    if(this->equal(ans, inout)) return false;

    ans = inout; // 这里应该是深拷贝，C++本身就是如此
    return true;
}

/// 根据指令计算数据流分析的结果，指令的枚举即Instruction:Store等的定义在llvm/IR/Instruction.def
void ValueMaintainAnalyzer::transfer(const Instruction *inst, FlowAnalysisDataTy &inout){
    switch(inst->getOpcode()){
        case Instruction::Store:{ 
            this->doStore((const StoreInst*)inst, inout);
        }break;
        // case Instruction::Load:{  // user = load op0
        //     this->doLoad((const LoadInst*)inst, inout);
        // }break;
        case Instruction::Add:{  // user = add op0, op1
            this->doAdd((const BinaryOperator*)inst, inout);
        }break;
        // case Instruction::SDiv:{
        //     this->doSDiv((const BinaryOperator*)inst, inout);
        // }break;
        default:{
            ;
        }
    }
}

/// 处理store指令，store op0, op1， op0有可能是常数，op1必然是地址，主要是一个gen和可能的kill
void ValueMaintainAnalyzer::doStore(const StoreInst *inst, FlowAnalysisDataTy &inout){
    const Value *op0 = inst->getOperand(0); // 取出op0和op1
    const Value *op1 = inst->getOperand(1);  
    /// 该语句定义了op1，inout中其他的op1的定义全部都要kill
    auto allIt = this->value2insts.find(op1);
    assert(allIt != this->value2insts.end());
    for(auto id: allIt->second){
        inout.reset(id);
    }
    /// 这一步相当于gen
    inout.set(this->inst2id.find(inst)->second);
    return;
}

/// user = add op0, op1
void ValueMaintainAnalyzer::doAdd(const BinaryOperator *inst, FlowAnalysisDataTy &inout){
    const Value *op0 = inst->getOperand(0);
    const Value *op1 = inst->getOperand(1);
    const User *user = inst->getOperandUse(0).getUser(); 
    /// 该语句定义了user，inout中其他的user定义全部都要kill
    auto allIt = this->value2insts.find(user);
    assert(allIt != this->value2insts.end());
    for(auto id: allIt->second){
        inout.reset(id);
    }
    /// 这一步相当于gen
    inout.set(this->inst2id.find(inst)->second);
    return;    
}

void ValueMaintainAnalyzer::initialize(FlowAnalysisDataTy &d){
    d.clear();
}

/// 将src的内容合并到tgt上
void ValueMaintainAnalyzer::merge(const FlowAnalysisDataTy &src, FlowAnalysisDataTy &tgt){
    tgt |= src;
}

bool ValueMaintainAnalyzer::equal(const FlowAnalysisDataTy &src, const FlowAnalysisDataTy &tgt){
    return src == tgt;
}

void ValueMaintainAnalyzer::report(ostream &os)const{
    os<<"Total iteration count: "<<this->iteration_count<<endl;

    for(auto &pp: this->bb2ans){
        const BB *bb = pp.first;
        os<<"===  "<<(string)bb->getName()<<": "<<endl;

        const FlowAnalysisDataTy &ans = this->bb2ans.find(bb)->second;
        for(unsigned i: ans){
            os<<i<<": ";
            /// 打印出第i条指令
            const Instruction *p = this->id2inst.find(i)->second;
            os<<"("<<p->getOpcodeName()<<":";
            switch(p->getOpcode()){
                case Instruction::Store:{
					const StoreInst *storeInst = (const StoreInst*)p;
					const Value *value = storeInst->getValueOperand();
					const Value *pointer = storeInst->getPointerOperand();
					os<<" "<<(string)value->getName()<<", "<<(string)pointer->getName()<<")"<<endl;                    
                }break;
            }
        }
        os<<endl;
    }    
}



