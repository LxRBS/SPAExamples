void f(int * p){++*p;}

void mn(){
    int a = 3;
    f(&a);
    a = a + a;
}
