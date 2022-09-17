#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include "Util/BasicTypes.h"
#include "Util/SCC.h"
#include "Util/SVFUtil.h"
#include <Graphs/GenericGraph.h>
using namespace SVF;
using namespace llvm;

#define fi first
#define se second
#define pb push_back
#define mp make_pair

/**
 * 将k,v加入到指定的map
 * map维护的(K, V的数组)
 * 如果m中没有k，就新创建一个数组，并把v放进去
 * 如果m中有k，就把v加在已有数组的后面 
*/
template<typename K, typename V>
void insert(const K &k, const V &v, std::map<K, std::vector<V> > &m){
    auto it = m.find(k);
    if(it==m.end()){
        m.insert(it, mp(k, std::vector<V>(1, v)));
    }else{
        it->se.pb(v);
    }
}

/**
 * 简单的输出vector，直接输出成一行
 * typename T: 元素类型，需要重载流输出运算符
*/
template<typename T>
std::ostream& operator<<(std::ostream&os, const std::vector<T>&rhs){
    if(rhs.empty()) return os;
    os<<*rhs.begin();
    for(typename std::vector<T>::const_iterator it=++rhs.begin(),eit=rhs.end();it!=eit;++it){
        os<<" "<<*it;
    }
    return os;
}

namespace SVF{

class LxNode; // 前置声明

/// 定义边的类
class LxEdge: public GenericEdge<LxNode>{
public:    
    LxEdge(LxNode *src, LxNode *dst):GenericEdge(src,dst,0){}
};

/// 定义节点的类
class LxNode:public GenericNode<LxNode, LxEdge>{
public:
    LxNode(NodeID id):GenericNode(id,0){}
};

/// 定义图
class LxGraph:public GenericGraph<LxNode,LxEdge>{
public:
    /// 加边操作
    bool addEdge(LxNode *src, LxNode *dst, LxEdge *edge){
		///不考虑边的类型，直接使用addOutgoingEdge添加即可
        src->addOutgoingEdge(edge);
		dst->addIncomingEdge(edge);
		this->incEdgeNum();
		return true;
	}

    /// 实际上无需提供析构函数，因为父类的析构函数会释放所有的节点和边
    ~LxGraph(){}

    /**
	 * 生成默认的测试用图
	 * 6个点，7条边，2个SCC
	*/
	static LxGraph* getDefaultTestGraph(){
		LxGraph *g = new LxGraph;
		for(int i=1;i<=6;++i){
			g->addGNode(i, new LxNode(i));
		}

		int const an = 7;///7条边
		int const a[][2] = {
			{1,2},{2,3},{3,1},{4,5},{5,6},{6,4},{1,4},{6,3}
		};
		for(int i=0;i<an;++i){
			LxNode *src = g->getGNode(a[i][0]);
			LxNode *dst = g->getGNode(a[i][1]);
			LxEdge *p = new LxEdge(src, dst);
			g->addEdge(src,dst,p);
		}
		return g;
	}


    /**
	 * 将图写成dot文件，可视化
	 * 需要实现DOTGraphTraits，否则就是默认值
	 * string: name  文件本名，最后的文件为"name.dot"
	*/
	void dump(std::string name){
        GraphPrinter::WriteGraphToFile(SVFUtil::outs(), name, this);
    }

    /// 写dot文件所需
	inline std::string getGraphName() const{
        return "LxGraph";
    }

private:
    LxGraph():GenericGraph(){}
};

};



namespace llvm{

/// 以下三个结构体为类型萃取所需
template<> struct GraphTraits<LxNode*>:public GraphTraits<SVF::GenericNode<LxNode,LxEdge>*  >{

};

template<> struct GraphTraits<Inverse<LxNode*> >
    :public GraphTraits<Inverse<SVF::GenericNode<LxNode,LxEdge>* > >{
	
};

template<> struct GraphTraits<LxGraph*>
    : public GraphTraits<SVF::GenericGraph<LxNode,LxEdge>* >{
	typedef LxNode *NodeRef;
};

/// 以下为输出dot文件所需
template<>
struct DOTGraphTraits<LxGraph*> : public DefaultDOTGraphTraits{
    typedef LxNode NodeType;
	typedef NodeType::iterator ChildIteratorTy;
    
	/// 必须的，不可省略
	DOTGraphTraits(bool isSimple = false):DefaultDOTGraphTraits(isSimple)
    {
    }

    /// Return name of the graph
    static std::string getGraphName(LxGraph *graph){
        return graph->getGraphName();
    }

    /**
	 * 指定节点的标签，显示在哪？？？
	 * LxNode* node: 指定节点
	 * LxGraph*:
	 * return: string
	*/
    static std::string getNodeLable(LxNode *node, LxGraph*){
        std::string str;
		raw_string_ostream rawstr(str);
		/// 只写节点编号
		rawstr<<"NodeID: "<<node->getId();
		return rawstr.str();		
	}

    /**
	 * 另一个标签，会显示在竖线之后
	*/
	static std::string getNodeIdentifierLabel(LxNode *node, LxGraph*){
		std::string str;
		raw_string_ostream rawstr(str);
		/// 只写节点编号
		rawstr<<" ID: "<<node->getId();
		return rawstr.str();
	}

    /**
	 * 指定节点的属性字符串,例如"shape=circle"，则节点呈圆形
	*/
	static std::string getNodeAttributes(LxNode *node, LxGraph*){
		return "shape=circle";
	}
};

}//end namespace llvm


/**
 * 测试SCC，给定一个默认的图
*/
void testSCC(){
    LxGraph *g = LxGraph::getDefaultTestGraph();
    g->dump("scc");
    /// 输出搜索顺序，因为节点编号是用哈希保存的，因此不按大小顺序
    cout<<"搜索顺序"<<endl;
    typedef llvm::GraphTraits<LxGraph*> GTraits;
    GTraits::nodes_iterator I = GTraits::nodes_begin(g);
    GTraits::nodes_iterator E = GTraits::nodes_end(g);
    for(;I!=E;++I){
        cout<<GTraits::getNodeID(*I)<<" ";
    }
    cout<<endl;

    SCCDetection<LxGraph*> scc(g);    
	scc.find();
	
    /// SCC具体内容
    cout<<"SCC detail: "<<endl;
	typedef vector<NodeID> vid;
	map<NodeID, vid> sccInfo;
	for(auto it=g->begin();it!=g->end();++it){
        insert(scc.repNode(it->fi), it->fi, sccInfo);
	}
	for(auto p: sccInfo){
		cout<<p.fi<<": "<<p.se<<endl;
    }

    /// 输出拓扑排序
    auto stack = scc.topoNodeStack();
    vid vec;
    while(!stack.empty()){
        vec.pb(stack.top());
        stack.pop();
    }
    cout<<"拓扑排序"<<endl;
    cout<<vec<<endl;
    delete g;
}
int main(int argc, char **argv){
    testSCC();
    return 0;
}