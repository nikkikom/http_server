#define A(...)     func(C,##__VA_ARGS__)
#define B(args...) func(C,args)

A(1,2,3);
A(1,2);
A(1);
A();

B(1,2,3);
B(1,2);
B(1);
B();

