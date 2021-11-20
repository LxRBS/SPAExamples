#include "SignAnalyzer.h"

using namespace llvm;
using namespace SVF;
using namespace std;

void SignAnalyzer::initialize(FlowAnalysisDataTy &d){
    d.clear();
    /// 先做一个去常转型，否则下一句不能通过const检测
    SymbolTableInfo *tabletmp = const_cast<SymbolTableInfo*>(this->table);
    /// m1是一个<const Value*, ID>的map
    const SymbolTableInfo::ValueToIDMapTy &m1 = tabletmp->valSyms();   
    for(auto &pp: m1){ // 每一个变量的初始状态都是初始化INI
        d.insert({pp.first, INI});
    } 
    return;
}

/// 将src的内容合并到tgt上
void SignAnalyzer::merge(const FlowAnalysisDataTy &src, FlowAnalysisDataTy &tgt){
    for(const auto &pp: src){
        auto it = tgt.find(pp.first);
        if(tgt.end() == it){ // 如果tgt中本来没有，直接插入
            tgt.insert(it, pp);
        }else{ // 否则等于两者之和，这个和由unite函数定义
            it->second = this->unite(it->second, pp.second); 
        }
    }     
}

bool SignAnalyzer::equal(const FlowAnalysisDataTy &src, const FlowAnalysisDataTy &tgt)const{
    if(src.size() != tgt.size()) return false;

    for(const auto &pp: src){
        auto it = tgt.find(pp.first);
        if(tgt.end() == it || pp.second != it->second) return false;
    }
    return true;
}

/// 处理store指令，store op0, op1， op0有可能是常数，op1必然是地址
void SignAnalyzer::doStore(const StoreInst *inst, FlowAnalysisDataTy &inout){
    const Value *op0 = inst->getOperand(0); // 取出op0和op1
    const Value *op1 = inst->getOperand(1);    
    SignK ret = this->getSign(op0, inout);  // 检查op0的符号
    auto it = inout.find(op1);
    if(inout.end() == it){ // 如果op1本来不存在，直接将op0的符号作为op1的符号记录
        inout.insert(it, {op1, ret});
    }else{ // 如果op1本来就有符号，需要执行一个unite操作
        it->second = this->unite(it->second, ret);
    }
}

/// user = load op0，op0是地址
void SignAnalyzer::doLoad(const LoadInst *inst, FlowAnalysisDataTy &inout){
    const Value *op0 = inst->getOperand(0);
    const User *user = inst->getOperandUse(0).getUser(); 
    auto it = inout.find(op0);    
    SignK ret = it != inout.end() ? it->second : INI;
    it = inout.find(user);
    if(inout.end() == it){
        inout.insert(it, {user, ret});
    }else{
        it->second = this->unite(it->second, ret);
    }    
}

/// user = add op0, op1
void SignAnalyzer::doAdd(const BinaryOperator *inst, FlowAnalysisDataTy &inout){
    const Value *op0 = inst->getOperand(0);
    const Value *op1 = inst->getOperand(1);
    const User *user = inst->getOperandUse(0).getUser(); 
    SignK a = this->getSign(op0, inout);
    SignK b = this->getSign(op1, inout);
    SignK ret = INI;
    switch(a){
        case POS:ret = (POS==b||ZER==b) ? POS : UNK;break;
        case ZER:ret = b;break;
        case NEG:ret = (NEG==b||ZER==b) ? NEG : UNK;break;
        case UNK:ret = UNK;break;
    }

    auto it = inout.find(user);
    if(inout.end() == it){
        inout.insert(it, {user, ret});
    }else{
        it->second = this->unite(it->second, ret);
    } 
}

void SignAnalyzer::report(ostream &os)const{
    os<<"Total iteration count: "<<this->iteration_count<<endl;
    const vector<string> &vec = SignAnalyzer::getSignKindStrings();

    for(auto pp: this->bb2id){
        const BB *bb = pp.first;
        os<<"===  "<<(string)bb->getName()<<": "<<endl;
        /// 输出符号
        const FlowAnalysisDataTy &ans = this->bb2ans.find(bb)->second;
        for(auto tt: ans){
            const Value *value = tt.first;
            if(value->hasName()){
                os<<(string)value->getName()<<": "<<vec[tt.second]<<endl;
            }
        }
    }
}

/// 获取指定变量的符号
SignK SignAnalyzer::getSign(const llvm::Value *value, const FlowAnalysisDataTy &ans)const{
    /// 暂时只处理整型数值
    if(auto c = llvm::dyn_cast<llvm::ConstantInt>(value)){  // 如果是常数
        int64_t cc = c->getSExtValue();  // 取出具体的数值
        return cc > 0 ? POS : (cc < 0 ? NEG : ZER);
    }
    auto it = ans.find(value);
    return it != ans.end() ? it->second : INI;
}

/// 合并答案
SignK SignAnalyzer::unite(SignK a, SignK b){
    switch(a){
        case INI:return b;  // a是未初始化的，直接返回b即可
        case POS:return POS==b||INI==b?POS:UNK;  // a是正的，b也是正的或者未初始化则返回正，否则unkown
        case ZER:return ZER==b||INI==b?ZER:UNK;
        case NEG:return NEG==b||INI==b?NEG:UNK;
        case UNK:return UNK;
        default:throw std::runtime_error("Unkown Sign Analysis kind.");
    }
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