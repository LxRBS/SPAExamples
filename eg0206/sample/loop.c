void f(){
    int i, j, n = 998232453;
    for(i=0;i<n;i+=23){
        if(i % 17 == 0) continue;
        i += 5;
        if(i % 19 == 2) break;
        for(j=0;j<i;++j){
            if(j % 29 == 8) break;
            j -= 1;
            if(j % 37 == 1) continue;
            --n;
        }
    }
    n = j = i;
    while(n--){
        i -= 2;
        j -= 3;
        if(i % 13 == 0) break;
        --j;
        if(j % 37 == 0) continue;
        ++i;
    }
    n = j = i;
}