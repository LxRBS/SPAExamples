/**
 * 本文件用于存放一些杂项的宏定义以及一些模板
 * @file lxmsc.h
 * @author Luoxun
*/

#ifndef _LX_MSC_H_
#define _LX_MSC_H_

#include <utility>
#include <vector>
#include <map>
#include <iostream>

/// 引用pair时保持对齐,生成pair时保持简化
#define fi first
#define se second
#define mp make_pair

/// 使用拼音防止命名冲突，该宏用于迭代，p必须是指针且有begin()和end()函数
// #define fanwei(p) (llvm::make_range((p)->begin(), (p)->end()))


/**
 * 将pair<a, b>加入到v中
 * v必须是vector<pair<A,B>>类型
*/
template<typename A,typename B>
void insert(std::vector<std::pair<A,B> >&v, const A&a, const B&b){
    v.push_back(std::mp(a, b));
}


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
        it->se.push_back(v);
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

#endif