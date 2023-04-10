void f(){
    int i = 0, a = 1;
    if(a > 3){
        if(i < 0) ++i;
    }else{
        --a;
    }
    i = a + i;
}