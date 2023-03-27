#include <bits/stdc++.h>
#include <z3++.h>
using namespace z3;


void validate(){
    context c;
    solver s(c);

    expr a = c.int_const("a");
    expr b = c.int_const("b");
    expr a0 = c.int_const("a0");
    expr b0 = c.int_const("b0");
    expr a1 = c.int_const("a1");
    expr b1 = c.int_const("b1");
    expr a2 = c.int_const("a2");
    expr b2 = c.int_const("b2");

    expr conj = implies(a0 == a && b0 == b && a1 == a0 + b0 && b1 == a1 - b0 && a2 == a1 - b1 && b2 == b1,  a2 == b && b2 == a0);
    s.add(!conj);

    std::string out;
    switch(s.check()){
        case unsat: out = "swap success."; break;
        case sat: out = "sth. unsual."; break;
        case unknown: out = "impossible."; break;
    }
    std::cout << out << std::endl;  
}


int main() {
    try {
        validate(); std::cout << "\n";
    }catch (exception & ex) {
        std::cout << "unexpected error: " << ex << "\n";
    }
    Z3_finalize_memory();
    return 0;
}