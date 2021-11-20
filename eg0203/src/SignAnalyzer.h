/**
 * 继承数据流框架，来实现符号分析
 * @file: SignAnalyzer.h
 * @author: Luo Xun 
 * @version: 0.1 目前只能处理Load、Store、加法指令，目前只能处理整型，不能处理循环
*/
#ifndef _SIGN_ANALYZER_H_
#define _SIGN_ANALYZER_H_

#include "DataFlowAnalysis.h"

enum SignK{
    INI, POS, NEG, ZER, UNK
};

class SignAnalyzer 
    : public DataFlowAnalysis<std::unordered_map<const llvm::Value*, SignK>>{

public:
    typedef std::unordered_map<const llvm::Value*, SignK> FlowAnalysisDataTy; 
    typedef DataFlowAnalysis<FlowAnalysisDataTy> BaseTy;

public: // constructor
    SignAnalyzer(const SVF::SVFModule *pt, const SVF::SymbolTableInfo *p2):BaseTy(pt, p2){}

public:
    void report(std::ostream &)const override;

private:
    void merge(const FlowAnalysisDataTy &src, FlowAnalysisDataTy &tgt)override;

    /// 判断src和tgt是否相等
    bool equal(const FlowAnalysisDataTy &src, const FlowAnalysisDataTy &tgt)const override;

    /// 根据需要初始化流分析的数据结构d
    void initialize(FlowAnalysisDataTy &d)override;  

private: // 每条指令的具体处理
    /// 处理store指令，格式为 store op0, op1
    void doStore(const llvm::StoreInst *inst, FlowAnalysisDataTy &inout)override;

    /// 处理load指令, user = load op0
    void doLoad(const llvm::LoadInst *inst, FlowAnalysisDataTy &inout)override;
    
    /// 处理add指令, user = add op0, op1
    void doAdd(const llvm::BinaryOperator *inst, FlowAnalysisDataTy &inout)override;

public:
    /// 获取SignK的对应的字符串数组
    static const std::vector<std::string>& getSignKindStrings();
    /// 给定枚举符，获取对应的字符串
    static std::string getSignKindString(SignK k);

private:
    /// 合并
    static SignK unite(SignK, SignK);  

    /// 获取指定变量的符号  
    SignK getSign(const llvm::Value *value, const FlowAnalysisDataTy &ans)const;
};

#endif