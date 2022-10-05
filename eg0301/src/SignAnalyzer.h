/**
 * 数据流分析中的符号分析
 * 判断每个变量的正负符号
 * @file: SignAnalyzer.h
 * @author: Luo Xun 
 * @version: 0.1 目前只能处理Load、Store、加法和除法指令，目前只能处理整型，不能处理循环
*/

#ifndef _SIGN_ANALYZER_H_
#define _SIGN_ANALYZER_H_

#include <bits/stdc++.h>
#include "MemoryModel/SymbolTableInfo.h"
#include "Util/SVFModule.h"
#include "Util/WorkList.h"


class SignAnalyzer{
public:
    enum SignK{  // 符号种类的枚举
        INI,  // 初始化，未计算
        POS,  // 正
        NEG,  // 负
        ZER,  // 零
        UNK   // unkown，可正可负
    };

public:  // 内部用的一些typedef
    typedef llvm::BasicBlock BB;
    /// BB到ID的映射的type
    typedef std::unordered_map<const BB*, SVF::NodeID> BBIDMapTy;  
    /// 变量到答案的映射的type
    typedef std::unordered_map<const llvm::Value*, SignK> VariableAnsMapTy;
    /// 保存数据流分析结果的数据结构 
    typedef VariableAnsMapTy FlowAnalysisDataTy;
    /// BB到答案的映射
    typedef std::unordered_map<const BB*, FlowAnalysisDataTy> BBAnsMapTy;
    /// 保证元素唯一性的队列的type
    typedef SVF::FIFOWorkList<const BB*> WorkListTy;

public:
    /// 构造函数，给定SVF模块
    SignAnalyzer(const SVF::SVFModule *pt, const SVF::SymbolTableInfo *p2):module(pt), table(p2){
        this->init();
        this->proc();
    }

    /// 指定输出流输出分析结果
    void report(std::ostream&os)const;

public:
    /// 获取SignK的对应的字符串数组
    static const std::vector<std::string>& getSignKindStrings();
    /// 给定枚举符，获取对应的字符串
    static std::string getSignKindString(SignK k);

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
    /// 获取指定变量的符号
    SignK getSign(const llvm::Value *value, const FlowAnalysisDataTy &ans)const{
        /// 暂时只处理整型数值
        if(auto c = llvm::dyn_cast<llvm::ConstantInt>(value)){  // 如果是常数
            int64_t cc = c->getSExtValue();  // 取出具体的数值
            return cc > 0 ? POS : (cc < 0 ? NEG : ZER);
        }
        auto it = ans.find(value);
        return it != ans.end() ? it->second : INI;
    }

    /// 合并答案
    static SignK unite(SignK a, SignK b){
        switch(a){
            case INI:return b;  // a是未初始化的，直接返回b即可
            case POS:return POS==b||INI==b?POS:UNK;  // a是正的，b也是正的或者未初始化则返回正，否则unkown
            case ZER:return ZER==b||INI==b?ZER:UNK;
            case NEG:return NEG==b||INI==b?NEG:UNK;
            case UNK:return UNK;
            default:throw std::runtime_error("Unkown Sign Analysis kind.");
        }
    }

private:
    const SVF::SVFModule *module;      // SVF模块，外部给定
    const SVF::SymbolTableInfo *table; // 符号表， 外部给定 
    
    BBIDMapTy bb2id;   // BB的编号，从1开始
    BBAnsMapTy bb2ans; // 每个BB的结果

    const BB *start_point; // 数据流分析的起点，暂时没用
    WorkListTy worklist;   // 队列，暂时没用

    int iteration_count;
private:
    const bool debug = false;
};

#endif
