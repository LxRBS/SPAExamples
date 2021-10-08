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

#include "SVF-FE/SymbolTableInfo.h"
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
    /// 答案的数据结构，就是一个稀疏的位向量，每一位对应一条指令
    typedef SVF::NodeBS AnsTy;
    /// 指令定义变量的type
    typedef std::unordered_map<const llvm::Instruction*, const llvm::Value*> InstDefMapTy;
    /// 变量与定义所在指令的type
    typedef std::unordered_map<const llvm::Value*, const llvm::Instruction*> DefInstMapTy;
    /// 每个BB的答案
    typedef std::unordered_map<const BB*, AnsTy> BBAnsMapTy;
    /// 保证元素唯一性的队列的type
    typedef SVF::FIFOWorkList<const BB*> WorkListTy;

public:
    /// 构造函数，给定SVF模块
    ValueMaintainAnalyzer(const SVF::SVFModule *pt, const SVF::SymbolTableInfo *p2):module(pt), table(p2), entry(nullptr){
        this->init();
        this->proc();
    }

    /// 指定输出流输出分析结果
    void report(std::ostream&os)const;

public:

private:
    void init();  // 初始化
    void proc();  // 分析  

private:
    /// 处理store指令 store op0, op1， op0是值，op1是指针
    void procStore(const llvm::Instruction *inst, AnsTy &ans);

private:
    const SVF::SVFModule *module;  // SVF模块，外部给定
    const SVF::SymbolTableInfo *table; // 符号表， 外部给定
    WorkListTy worklist; // 队列

    BBIDMapTy bb2id;         //BB到ID的映射
    InstIDMapTy inst2id;     //Inst到ID的映射
    BBAnsMapTy bb2ans;       //BB到答案的映射
    InstDefMapTy inst2value; // 键值对<inst, value>表示在inst指令中定义了value变量
    DefInstMapTy value2inst; // 键值对<value,inst>表示value的定义位置是在inst

    const BB *entry;

private:
    const bool debug = true;
};

#endif
