#include <bits/stdc++.h>
#include <z3++.h>
using namespace z3;


void validate_invariant() {
    std::cout << "validate invariant\n";
    context c;

    expr i = c.int_const("i");
    expr ip = c.int_const("ip");

    expr Prex = i == 0;
    expr Invx = 0 <= i && i <= 10;
    expr Gx = i < 10;
    expr T = ip == i + 1;
    expr Invxp = 0 <= ip && ip <= 10;
    expr Postx = i == 10;

    expr c1 = implies(Prex, Invx);
    expr c2 = implies(Invx && Gx && T, Invxp); 
    expr c3 = implies(Invx && !Gx, Postx);
    expr conjecture = (c1 && c2 && c3);

    solver s(c);
    s.add(conjecture);
    switch(s.check()){
    case unsat:   std::cout << "loop invariant is not valid\n"; break;
    case sat:     std::cout << "loop invariant is valid\n"; break;
    case unknown: std::cout << "unknown\n"; break;        
    }
    return;
}


int main() {
    try {
        validate_invariant(); std::cout << "\n";
    }catch (exception & ex) {
        std::cout << "unexpected error: " << ex << "\n";
    }
    Z3_finalize_memory();
    return 0;
}