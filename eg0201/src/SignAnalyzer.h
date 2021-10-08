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
#include "SVF-FE/SymbolTableInfo.h"
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
    /// 每个BB的答案的type
    typedef std::unordered_map<const BB*, VariableAnsMapTy> BBAnsMapTy;
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

    void initBBs();    // 为所有的BB编号，初始化成员变量bb2id
    void initValues(); // 获取所有的变量，初始化ans

private:
    /// 把src的答案合并到tgt上，即 tgt |= src
    /// 返回true表示tgt的答案有所变化，否则返回false
    bool uniteTo(const BB *src, const BB *tgt);

    /// 根据BB自身的指令计算答案，如果有所改变返回true
    bool generate(const BB *bb);

    /// 根据给定指令，更新相应的结果
    bool generate(const llvm::Instruction *inst, VariableAnsMapTy &ans);

    /// 处理store指令，格式为 store op0, op1
    bool genStore(const llvm::Value *op0, const llvm::Value *op1, VariableAnsMapTy &ans);

    /// 处理load指令, user = load op0
    bool genLoad(const llvm::User *user, const llvm::Value *op0, VariableAnsMapTy &ans);
    
    /// 处理add指令, user = add op0, op1
    bool genAdd(const llvm::User *user, const llvm::Value *op0, const llvm::Value *op1, VariableAnsMapTy &ans);

    /// 处理sdiv指令, user = sdiv op0, op1
    bool genSdiv(const llvm::User *user, const llvm::Value *op0, const llvm::Value *op1, VariableAnsMapTy &ans);

    
    /// 获取指定变量的符号
    SignK getSign(const llvm::Value *value, const VariableAnsMapTy &ans)const{
        /// 暂时只处理整型数值
        if(auto c = llvm::dyn_cast<llvm::ConstantInt>(value)){  // 如果是常数
            int64_t cc = c->getSExtValue();  // 取出具体的数值
            return cc > 0 ? POS : (cc < 0 ? NEG : ZER);
        }
        return ans.find(value)->second;
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
    const SVF::SVFModule *module;  // SVF模块，外部给定
    const SVF::SymbolTableInfo *table; // 符号表， 外部给定 
    
    BBIDMapTy bb2id;  // BB的编号，从1开始
    BBAnsMapTy bb2ans;   // 每个BB的结果

    WorkListTy worklist; // 队列，暂时没用

private:
    const bool debug = false;
};

#endif