void f(int * x){++*x;}
void mn(){
    int a;
    f(&a);
    a = a + 1;
}
