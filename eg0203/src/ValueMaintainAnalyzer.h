/**
 * 继承数据流框架来实现保值分析
 * @file: ValueMaintianAnalyzer.h
 * @author: Luo Xun 
 * @version: 0.1 目前只能处理Store指令，目前只能处理整型
*/

#ifndef _VALUE_MAINTAIN_ANALYZER_H_
#define _VALUE_MAINTAIN_ANALYZER_H_

#include <bits/stdc++.h>

#include "DataFlowAnalysis.h"


class ValueMaintainAnalyzer 
 : public DataFlowAnalysis<SVF::NodeBS>{

public:
    typedef SVF::NodeBS FlowAnalysisDataTy; 
    typedef DataFlowAnalysis<FlowAnalysisDataTy> BaseTy;

    /// 指令到ID的映射的type
    typedef std::unordered_map<const llvm::Instruction*, SVF::NodeID> InstIDMapTy;
    /// ID到指令的映射的type
    typedef std::unordered_map<SVF::NodeID, const llvm::Instruction*> IDInstMapTy;
    /// 指令ID定义变量的type
    typedef std::unordered_map<SVF::NodeID, const llvm::Value*> InstIDDefMapTy;
    /// 变量与定义所在指令的ID的type，注意后者是一组语句，因为一个变量可能在多个定义语句中被定义
    typedef std::unordered_map<const llvm::Value*, SVF::NodeBS> DefInstIDsMapTy;

public: // constructor
    ValueMaintainAnalyzer(const SVF::SVFModule *pt, const SVF::SymbolTableInfo *p2):BaseTy(pt, p2){}

public:
    void report(std::ostream &)const override;

private:
    void init()override;
    
    void merge(const FlowAnalysisDataTy &src, FlowAnalysisDataTy &tgt)override;

    /// 判断src和tgt是否相等
    bool equal(const FlowAnalysisDataTy &src, const FlowAnalysisDataTy &tgt)const override;

    /// 根据需要初始化流分析的数据结构d
    void initialize(FlowAnalysisDataTy &d)override;  

private: // 每条指令的具体处理
    /// 处理store指令，格式为 store op0, op1
    void doStore(const llvm::StoreInst *inst, FlowAnalysisDataTy &inout)override;
   
    /// 处理add指令, user = add op0, op1
    void doAdd(const llvm::BinaryOperator *inst, FlowAnalysisDataTy &inout)override;

private:
    InstIDMapTy inst2id;     // Inst到ID的映射
    IDInstMapTy id2inst;     // ID到Inst的映射
    InstIDDefMapTy inst2value; // 键值对<ID, value>表示在ID指令中定义了value变量
    DefInstIDsMapTy value2insts; // 键值对<value, NodeBS>表示value的定义位置是在NodeBS所表示的Inst中

};


#endif