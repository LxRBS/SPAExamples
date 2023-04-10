/**
 * 用于表示一个理论上的图
 * @file LxGraph.h
 * @author Luoxun
*/

#ifndef _LX_GRAPH_H_
#define _LX_GRAPH_H_

#include <bits/stdc++.h>
#include <Util/SVFModule.h>
#include <Graphs/GenericGraph.h>

/// 凡是跟SVF图有关的类均写在SVF的命名空间中
namespace SVF{

/// 前置声明
class LxNode;
class LxGraph;

/// 边类
class LxEdge: public GenericEdge<LxNode>{
public:    
    LxEdge(LxNode *src, LxNode *dst):GenericEdge(src,dst,0){}
};

/// 点类
class LxNode:public GenericNode<LxNode, LxEdge>{
public:
    /**
     * 构造函数
     * @param id: 图节点的编号
	 * @param p: 该节点所在图的指针
    */
    LxNode(NodeID id, LxGraph * p):GenericNode(id, 0), parent(p){}

	/**
	 * @brief 为了使用llvm支配树，必须提供这个接口
	 */
	LxGraph * getParent() const {return this->parent;}

    /**
     * @brief 为了使用llvm支配树必须提供的接口
     */
    void printAsOperand(llvm::raw_ostream & os, bool){}
private:
    LxGraph * parent;
};

class LxGraph: public GenericGraph<LxNode, LxEdge>{
public:
    
    bool addEdge(LxNode *src, LxNode *dst, LxEdge *edge){
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
        // GraphPrinter::WriteGraphToFile(SVFUtil::outs(), name, this);
    }

    /// 写dot文件所需
	inline std::string getGraphName() const{
        return "LxGraph";
    }

    /**
     * @brief 生成如下的有向图
     * digraph demo{
	    0 -> {1} // 0作为entry
		1 -> {2}
		2 -> {3}
		3 -> {4, 5, 6}
		4 -> {1, 7}
		5 -> {3, 6}
		6 -> {8}
		7 -> {8, 9, 10}
	   }
     * @return LxGraph* 
     */
    static LxGraph * getInstanceOne(){
		using vi = std::vector<int>;
		static const std::vector<vi> vec{
			vi{1},
			vi{2},
			vi{3},
			vi{4, 5, 6},
			vi{1, 7},
			vi{3, 6},
			vi{8},
			vi{8, 9, 10},
			vi{},
			vi{},
			vi{}
		};
        auto * g = new LxGraph;
		/// 生成所有节点
		int const n = 10;
		for(int i=0;i<=10;++i){
			g->addGNode(i, new LxNode(i, g));
		}
		/// 加边
		for(int i=0;i<=10;++i){
			auto * from = g->getGNode(i);
			for(int j : vec[i]){
				auto * to = g->getGNode(j);
				g->addEdge(from, to, new LxEdge(from, to));
			}
		}
        return g;
	}

private:
    LxGraph():GenericGraph(){}
};

};  // end namespace SVF


namespace llvm{

/// 以下三个结构体为类型萃取所需
template<> struct GraphTraits<SVF::LxNode*>:public GraphTraits<SVF::GenericNode<SVF::LxNode,SVF::LxEdge>*  >{
    typedef SVF::LxNode * NodeRef;
};

template<> struct GraphTraits<Inverse<SVF::LxNode*> >
    :public GraphTraits<Inverse<SVF::GenericNode<SVF::LxNode,SVF::LxEdge>* > >{
	typedef SVF::LxNode * NodeRef; 
};

template<> struct GraphTraits<SVF::LxGraph*>
    : public GraphTraits<SVF::GenericGraph<SVF::LxNode,SVF::LxEdge>* >{
	typedef SVF::LxNode * NodeRef;
};

// 以下为输出dot文件所需
template<>
struct DOTGraphTraits<SVF::LxGraph*> : public DefaultDOTGraphTraits{
    typedef SVF::LxNode NodeType;
	typedef NodeType::iterator ChildIteratorTy;
    
	/// 必须的，不可省略
	DOTGraphTraits(bool isSimple = false) :
        DefaultDOTGraphTraits(isSimple)
    {
    }

    /// Return name of the graph
    static std::string getGraphName(SVF::LxGraph *graph){
        return graph->getGraphName();
    }

    /**
	 * 指定节点的标签
	 * LxNode* node: 指定节点
	 * LxGraph*:
	 * return: string, 该字符串的内容不知道会显示在哪
	*/
    static std::string getNodeLable(SVF::LxNode *node, SVF::LxGraph*){
        std::string str;
		llvm::raw_string_ostream rawstr(str);
		/// 只写节点编号
		rawstr<<"NodeID: "<<node->getId();
		return rawstr.str();		
	}

    /**
     * 指定节点的标签
     * LxNode* node: 指定节点
	 * LxGraph*:
	 * return: string, 该字符串的内容会显示在dot上
    */
	static std::string getNodeIdentifierLabel(SVF::LxNode *node, SVF::LxGraph*){
		std::string str;
		llvm::raw_string_ostream rawstr(str);
		/// 写节点编号，以及该BB的名字
		rawstr<<"ID: "<<node->getId();
		return rawstr.str();
	}

    /**
	 * 指定节点的属性字符串,例如"shape=circle"，则节点呈圆形
	*/
	static std::string getNodeAttributes(SVF::LxNode *node, SVF::LxGraph*){
		return "shape=rectangle";
	}
};

}//end namespace llvm

#endif