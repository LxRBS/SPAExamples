/**
 * 数据流分析中的保值分析，保值分析是可达性分析中的一种
 * 判断哪条赋值语句的结果可以传播到END
 * 输出赋值语句的编号
 * @file: ValueMaintianAnalyzer.h
 * @author: Luo Xun 
 * @version: 0.1 目前只能处理Store指令，目前只能处理整型
*/

#ifndef _VALUE_MAINTAIN_ANALYZER_H_
#define _VALUE_MAINTAIN_ANALYZER_H_

#include <bits/stdc++.h>

#include "MemoryModel/SymbolTableInfo.h"
#include "Util/SVFBasicTypes.h"
#include "Util/SVFModule.h"
#include "Util/WorkList.h"

#include "llvm/IR/Instruction.h"


class ValueMaintainAnalyzer{
public:

public:  // 内部用的一些typedef
    typedef llvm::BasicBlock BB;
    /// BB到ID的映射的type
    typedef std::unordered_map<const BB*, SVF::NodeID> BBIDMapTy;  
    /// 指令到ID的映射的type
    typedef std::unordered_map<const llvm::Instruction*, SVF::NodeID> InstIDMapTy;
    /// ID到指令的映射的type
    typedef std::unordered_map<SVF::NodeID, const llvm::Instruction*> IDInstMapTy;
    /// 答案的数据结构，就是一个稀疏的位向量，每一位对应一条指令
    typedef SVF::NodeBS FlowAnalysisDataTy;
    /// 指令ID定义变量的type
    typedef std::unordered_map<SVF::NodeID, const llvm::Value*> InstIDDefMapTy;
    /// 变量与定义所在指令的ID的type，注意后者是一组语句，因为一个变量可能在多个定义语句中被定义
    typedef std::unordered_map<const llvm::Value*, SVF::NodeBS> DefInstIDsMapTy;
    /// BB到答案的映射
    typedef std::unordered_map<const BB*, FlowAnalysisDataTy> BBAnsMapTy;
    /// 保证元素唯一性的队列的type
    typedef SVF::FIFOWorkList<const BB*> WorkListTy;

public:
    /// 构造函数，给定SVF模块
    ValueMaintainAnalyzer(const SVF::SVFModule *pt, const SVF::SymbolTableInfo *p2):module(pt), table(p2){
        this->init();
        this->proc();
    }

    /// 指定输出流输出分析结果
    void report(std::ostream&os)const;

public:

private:
    void init();  // 初始化
    void proc();  // 分析  

private: // 传递函数
    /// 给定bb，计算out(BB)，即out = gen(BB)并上(in(BB)-Kill(BB))
    /// 其中in等于bb所有前驱的out的合并
    /// 如果计算结果与原来的bb2ans[bb]相比有变化则返回true
    bool transfer(const BB *bb);

    /// 根据每条指令计算数据流分析的结果，inout既是输入参数也是输出结果
    void transfer(const llvm::Instruction *inst, FlowAnalysisDataTy &inout);

private: // 每条指令的具体处理
    /// 处理store指令，格式为 store op0, op1
    void doStore(const llvm::StoreInst *inst, FlowAnalysisDataTy &inout);

    /// 处理load指令, user = load op0
    void doLoad(const llvm::LoadInst *inst, FlowAnalysisDataTy &inout);
    
    /// 处理add指令, user = add op0, op1
    void doAdd(const llvm::BinaryOperator *inst, FlowAnalysisDataTy &inout);

    /// 处理sdiv指令, user = sdiv op0, op1
    void doSDiv(const llvm::BinaryOperator *inst, FlowAnalysisDataTy &inout);    

private: // 与数据结构有关的操作
    /// 将src合并到tgt上
    void merge(const FlowAnalysisDataTy &src, FlowAnalysisDataTy &tgt);

    /// 判断src和tgt是否相等
    bool equal(const FlowAnalysisDataTy &src, const FlowAnalysisDataTy &tgt);

    /// 根据需要初始化流分析的数据结构d
    void initialize(FlowAnalysisDataTy &d);    

private:
    const SVF::SVFModule *module;      // SVF模块，外部给定
    const SVF::SymbolTableInfo *table; // 符号表， 外部给定

    BBIDMapTy bb2id;         // BB到ID的映射
    InstIDMapTy inst2id;     // Inst到ID的映射
    IDInstMapTy id2inst;     // ID到Inst的映射
    BBAnsMapTy bb2ans;       // BB到答案的映射
    InstIDDefMapTy inst2value; // 键值对<ID, value>表示在ID指令中定义了value变量
    DefInstIDsMapTy value2insts; // 键值对<value, NodeBS>表示value的定义位置是在NodeBS所表示的Inst中

    const BB *start_point; // 数据流分析的起点，暂时没用
    WorkListTy worklist;   // 队列，暂时没用

    int iteration_count;
private:
    const bool debug = true;
};

#endif
