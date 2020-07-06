// https://www.sigbus.info/compilerbook#c%E3%81%AE%E5%9E%8B%E3%81%AE%E8%AA%AD%E3%81%BF%E6%96%B9
// Cの型の表記	意味
int x1; //	int
int *x2; //	* int
int x3[1]; //	[] int
int x4() {} //	func() int
int **x5; //	* * int
int (*x6)[1]; //	* [] int
int (*x7)(); //	* func() int
int *x8[1]; //	[] * int
int x9[1][1]; //	[] [] int
int *x10() {} //	func() * int
int ***x11; //	* * * int
int (**x12)[1]; //	* * [] int
int (**x13)(); //	* * func() int
int *(*x14)[1]; //	* [] * int
int (*x15)[1][1]; //	* [] [] int
int *(*x16)(); //	* func() * int
int **x17[1]; //	[] * * int
int (*x18[1])[1]; //	[] * [] int
int (*x19[1])(); //	[] * func() int
int *x20[1][1]; //	[] [] * int
int x21[1][1][1]; //	[] [] [] int
int **x22() {} //	func() * * int
int (*x23())[1] {} //	func() * [] int
int (*x24())() {} //	func() * func() int

// func signal(int i, * func(int) void f) * func(int) void
void (*signal(int i, void (*f)(int)))(int) {}

int main() {
    return 0;
}