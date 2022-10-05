/**
 * 用于表示BB的流程图，也包含了节点类和边类。三个类写在同一个文件中
 * @file LxBBGraph.h
 * @author Luoxun
*/

#ifndef _LX_BB_GRAPH_H_
#define _LX_BB_GRAPH_H_

#include <map>

#include <Util/SVFModule.h>
#include <Graphs/GenericGraph.h>

#include "lxmsc.h"

/// 凡是跟SVF图有关的类均写在SVF的命名空间中
namespace SVF{

/// 前置声明
class LxBBNode;

/// 边类
class LxBBEdge: public GenericEdge<LxBBNode>{
public:    
    LxBBEdge(LxBBNode *src, LxBBNode *dst):GenericEdge(src,dst,0){}
};

/// 点类
class LxBBNode:public GenericNode<LxBBNode, LxBBEdge>{
public:
    /**
     * 构造函数
     * @param id: 图节点的编号
     * @param bb: 跟该节点相关联的BB
    */
    LxBBNode(NodeID id, llvm::BasicBlock *b):GenericNode(id,0), bb(b){}
	const llvm::BasicBlock* getBB()const{return bb;}
private:
    llvm::BasicBlock *bb; 
};

class LxBBGraph: public GenericGraph<LxBBNode, LxBBEdge>{
public:
    
    bool addEdge(LxBBNode *src, LxBBNode *dst, LxBBEdge *edge){
		///不考虑边的类型，直接使用addOutgoingEdge添加即可
        src->addOutgoingEdge(edge);
		dst->addIncomingEdge(edge);
		this->incEdgeNum();
		return true;
	}

    /**
     * 将图的内容写到指定文件
     * @param name: 文件名
    */
    void dump(std::string name){
        GraphPrinter::WriteGraphToFile(SVFUtil::outs(), name, this);
    }

    /// 写dot文件所需
	inline std::string getGraphName() const{
        return "LxBBGraph";
    }

    /**
	 * 根据SVFModule生成BB控制流图
	 * 目前只能生成单函数
	*/
	static LxBBGraph* newInstance(const SVFModule *module){
        LxBBGraph *g = new LxBBGraph;
		/// 找出所有的BB，并按照名字编号，BB的名字应该是唯一的？
		std::map<string, int> name2Idx;
		int k = 1;
		for(auto &fun: llvm::make_range(module->llvmFunBegin(), module->llvmFunEnd())){
			auto &s1 = fun->getBasicBlockList();			
			for(auto &bb: s1){				
				name2Idx.insert(mp((string)bb.getName(), k));
				/// 每一个BB添加一个节点
				g->addGNode(k, new LxBBNode(k, &bb));
				++k;
			}
		}
        /// 找出所有的边
		for(auto &fun: llvm::make_range(module->llvmFunBegin(), module->llvmFunEnd())){
			auto &s1 = fun->getBasicBlockList();
			for(auto &bb: s1){
				llvm::BasicBlock *pBB = &bb;
				int from = name2Idx[(string)bb.getName()];
				for(auto suc: successors(pBB)){
					int to = name2Idx[(string)suc->getName()];
					LxBBNode *src = g->getGNode(from);
					LxBBNode *dst = g->getGNode(to);
					LxBBEdge *edge = new LxBBEdge(src, dst);
					g->addEdge(src, dst, edge);
				}
			}
		}
		return g;
	}

private:
    LxBBGraph():GenericGraph(){}
};

};  // end namespace SVF


namespace llvm{

/// 以下三个结构体为类型萃取所需
template<> struct GraphTraits<SVF::LxBBNode*>:public GraphTraits<SVF::GenericNode<SVF::LxBBNode,SVF::LxBBEdge>*  >{

};

template<> struct GraphTraits<Inverse<SVF::LxBBNode*> >
    :public GraphTraits<Inverse<SVF::GenericNode<SVF::LxBBNode,SVF::LxBBEdge>* > >{
	
};

template<> struct GraphTraits<SVF::LxBBGraph*>
    : public GraphTraits<SVF::GenericGraph<SVF::LxBBNode,SVF::LxBBEdge>* >{
	typedef LxBBNode *NodeRef;
};

/// 以下为输出dot文件所需
template<>
struct DOTGraphTraits<SVF::LxBBGraph*> : public DefaultDOTGraphTraits{
    typedef LxBBNode NodeType;
	typedef NodeType::iterator ChildIteratorTy;
    
	/// 必须的，不可省略
	DOTGraphTraits(bool isSimple = false) :
        DefaultDOTGraphTraits(isSimple)
    {
    }

    /// Return name of the graph
    static std::string getGraphName(LxBBGraph *graph){
        return graph->getGraphName();
    }

    /**
	 * 指定节点的标签
	 * LxBBNode* node: 指定节点
	 * LxBBGraph*:
	 * return: string, 该字符串的内容不知道会显示在哪
	*/
    static std::string getNodeLable(LxBBNode *node, LxBBGraph*){
        std::string str;
		llvm::raw_string_ostream rawstr(str);
		/// 只写节点编号
		rawstr<<"NodeID: "<<node->getId();
		return rawstr.str();		
	}

    /**
     * 指定节点的标签
     * LxBBNode* node: 指定节点
	 * LxBBGraph*:
	 * return: string, 该字符串的内容会显示在dot上
    */
	static std::string getNodeIdentifierLabel(LxBBNode *node, LxBBGraph*){
		std::string str;
		llvm::raw_string_ostream rawstr(str);
		/// 写节点编号，以及该BB的名字
		rawstr<<"ID: "<<node->getId();
        rawstr<<", BBName: "<<(string)node->getBB()->getName();
		return rawstr.str();
	}

    /**
	 * 指定节点的属性字符串,例如"shape=circle"，则节点呈圆形
	*/
	static std::string getNodeAttributes(LxBBNode *node, LxBBGraph*){
		return "shape=rectangle";
	}
};

}//end namespace llvm

#endif