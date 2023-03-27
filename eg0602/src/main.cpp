#include <bits/stdc++.h>
#include <z3++.h>
using namespace z3;


void find_model_example() {
    std::cout << "find_model_example1\n";
    context c;
    expr x = c.int_const("x");
    expr y = c.int_const("y");
    solver s(c);

    s.add(x < 0);
    s.add(y > 0);
    s.add(x + y < 3);
    // check判断是否有解
    std::cout << s.check() << "\n";
    // get_model返回其中一个解
    model m = s.get_model();
    // std::cout << m << "\n";
    // traversing the model
    for (unsigned i = 0; i < m.size(); i++) {
        func_decl v = m[i];
        assert(v.arity() == 0); 
        std::cout << v.name() << " = " << m.get_const_interp(v) << "\n";
    }
    return;
}


int main() {
    try {
        find_model_example(); std::cout << "\n";
    }catch (exception & ex) {
        std::cout << "unexpected error: " << ex << "\n";
    }
    Z3_finalize_memory();
    return 0;
}