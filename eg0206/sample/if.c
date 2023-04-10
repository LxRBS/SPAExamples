void f(){
    int a, b, c, d, r;
    r = a;
    if(r < a) r = a;
    if(r < b) r = b;
    if(r < c) r = c;
    a = b = c = d = r;
}