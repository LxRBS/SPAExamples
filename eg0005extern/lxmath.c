double const EPS = 1E-6;

double integral(double (*f)(double), double s, double e){
    double ans = 0.0;
    for(double x=s;x<=e;x+=EPS){
        ans += f(x);
    }
    return ans *= EPS;
}
