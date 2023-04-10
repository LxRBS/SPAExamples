void f(){
    int n = 100, s = 0;
    while(n > 0){
        s += n--;
    }
    int ret;
    if(s > 10000){
        ret = 1;
        while(s > 0){
            s--;
        }
    }else{
        ret = 0;
    }
    return;
}