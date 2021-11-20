/**
 * 数据流分析的框架
*/
#ifndef _DATA_FLOW_ANALYSIS_H_
#define _DATA_FLOW_ANALYSIS_H_

#include "SVF-FE/SymbolTableInfo.h"
#include "Util/SVFBasicTypes.h"
#include "Util/SVFModule.h"
#include "Util/WorkList.h"

#include "llvm/IR/Instruction.h"

template<typename FlowAnalysisDataTy>
class DataFlowAnalysis{

public: // 内部用的一些typedef
    typedef llvm::BasicBlock BB;
    /// BB到ID的映射的type
    typedef std::unordered_map<const BB*, SVF::NodeID> BBIDMapTy;  
    /// BB到答案的映射
    typedef std::unordered_map<const BB*, FlowAnalysisDataTy> BBAnsMapTy;
    /// 保证元素唯一性的队列的type
    typedef SVF::FIFOWorkList<const BB*> WorkListTy;    

public:
    /// 构造函数，给定SVF模块
    DataFlowAnalysis(const SVF::SVFModule *pt, const SVF::SymbolTableInfo *p2):module(pt), table(p2){
        this->init();
        this->proc();
    }    

protected: // 需要子类继承的纯虚函数
    /// 指定输出流输出分析结果
    virtual void report(std::ostream&os)const = 0;

protected: // 子类可以实现也可以直接继承的
    virtual void init(){
        /// 首先为所有的BB编号，这一步其实可以不需要，主要是为了调试方便
        int k = 1;
        for(auto fun: llvm::make_range(this->module->llvmFunBegin(), this->module->llvmFunEnd())){
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

    virtual void merge(const FlowAnalysisDataTy &src, FlowAnalysisDataTy &tgt){
        tgt.merge(src);
    }

    /// 判断src和tgt是否相等
    virtual bool equal(const FlowAnalysisDataTy &src, const FlowAnalysisDataTy &tgt)const{
        return src.equal(tgt);
    }

    /// 根据需要初始化流分析的数据结构d
    virtual void initialize(FlowAnalysisDataTy &d){
        d.init();
    }

protected: // 具体的指令处理函数，子类如果不实现则直接继承，效果相当于忽略该条指令
    /// 处理store指令，格式为 store op0, op1
    virtual void doStore(const llvm::StoreInst *inst, FlowAnalysisDataTy &inout){}

    /// 处理load指令, user = load op0
    virtual  void doLoad(const llvm::LoadInst *inst, FlowAnalysisDataTy &inout){}
    
    /// 处理add指令, user = add op0, op1
    virtual void doAdd(const llvm::BinaryOperator *inst, FlowAnalysisDataTy &inout){}

    /// 处理sdiv指令, user = sdiv op0, op1
    virtual void doSDiv(const llvm::BinaryOperator *inst, FlowAnalysisDataTy &inout){}     

private: // 不让子类继承的函数，即框架的实现
    /// 框架执行
    void proc(){
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

    /// 给定bb，计算out(BB)，即out = gen(BB)并上(in(BB)-Kill(BB))
    /// 其中in等于bb所有前驱的out的合并
    /// 如果计算结果与原来的bb2ans[bb]相比有变化则返回true
    bool transfer(const BB *bb){
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

    /// 根据每条指令计算数据流分析的结果，inout既是输入参数也是输出结果
    void transfer(const llvm::Instruction *inst, FlowAnalysisDataTy &inout){
        switch(inst->getOpcode()){
            case llvm::Instruction::Store:{ 
                this->doStore((const llvm::StoreInst*)inst, inout);
            }break;
            case llvm::Instruction::Load:{  // user = load op0
                this->doLoad((const llvm::LoadInst*)inst, inout);
            }break;
            case llvm::Instruction::Add:{  // user = add op0, op1
                this->doAdd((const llvm::BinaryOperator*)inst, inout);
            }break;
            case llvm::Instruction::SDiv:{
                this->doSDiv((const llvm::BinaryOperator*)inst, inout);
            }break;
            default:{
                ;
            }
        }        
    }

protected: // 数据成员，为了不提供函数，直接写成prtected
    const SVF::SVFModule *module;      // SVF模块，外部给定
    const SVF::SymbolTableInfo *table; // 符号表， 外部给定

    BBIDMapTy bb2id;         // BB到ID的映射
    BBAnsMapTy bb2ans;       // BB到答案的映射

    const BB *start_point; // 数据流分析的起点，暂时没用
    WorkListTy worklist;   // 队列，暂时没用

    int iteration_count;      
};

#endif