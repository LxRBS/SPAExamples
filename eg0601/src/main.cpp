#include <bits/stdc++.h>
#include <z3++.h>
using namespace z3;

void demorgan() {
    std::cout << "de-Morgan example\n";
    
    context c;

    expr x = c.bool_const("x");
    expr y = c.bool_const("y");
    expr conjecture = (!(x && y)) == (!x || !y);
    
    solver s(c);       // 定义求解器
    s.add(conjecture); // 将约束加入求解器
 
    // check检查是否有解，结果是sat，但这个结果实际上并不能证明Demorgan定理
    // 应该是check(非Demorgan)结果是sat，才能证明Demorgan定理
    // 这个与全称量词有关
    switch (s.check()) {
    case unsat:   std::cout << "de-Morgan is not valid\n"; break;
    case sat:     std::cout << "de-Morgan is valid\n"; break;
    case unknown: std::cout << "unknown\n"; break;
    }
}


int main() {
    try {
        demorgan(); std::cout << "\n";
    }catch (exception & ex) {
        std::cout << "unexpected error: " << ex << "\n";
    }
    Z3_finalize_memory();
    return 0;
}