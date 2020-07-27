// assert prints out reason and exits with code 1 if the given assertion is false (= 0).
int assert(int assertion, char *reason) {
    if (assertion != 0) {
        return 0;
    }
    printf(reason);
    printf("\n");
    exit(1);
}

int assertEquals(int got, int want, char *reason) {
    if (got == want) {
        return 0;
    }
    printf(reason);
    printf("\n");
    printf("want: %d, but got: %d\n", want, got);
    exit(1);
}

// assert test_1 returns 5
int test_1() {
    return 5;
    return 8;
}

// assert test_2 returns 1
int test_2() {
    if (1 == 1)
        return 1;
    else
        return 2;
}

// assert test_3 returns 5
int test_3() {
    int i;
    i = 5;
    if (i == 4)
        return 3;
    return 5;
}

// assert test_4 returns 5
int test_4() {
    int i;
    i = 0;
    for (; i < 5; )
        i = i + 1;
    return i;
}

// assert test_5 returns 16
int test_5() {
    int ans;
    ans = 1;
    int i;
    for (i = 0; i < 4; i = i + 1)
        ans = ans * 2;
    return ans;
}

// assert test_6 returns 17
int test_6() {
    int test;
    test = 20;
    while (test / 3 != 5)
        test = test - 1;
    return test;
}

// assert test_7 returns 7
int test_7() {
    int i;
    i = 2;
    if (i == 2) {
        i = i + 1;
        i = i * 2 + 1;
    }
    return i;
}

// for test: genuine fibonacci recursive function
int fib(int x) {
    if (x <= 1) return x;
    return fib(x - 1) + fib(x - 2);
}

// Takeuchi function
int tarai(int x, int y, int z) {
    if (x <= y) return y;
    return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y));
}

// assert test_8 returns 3
int test_8() {
    int x;
    x = 3;
    int *y;
    y = &x;
    return *y;
}

// assert test_9 returns 3
int test_9() {
    int x;
    int *y;
    y = &x;
    *y = 3;
    return x;
}

// assert test_10 returns 3
int test_10() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    return *p + *(p + 1);
}

// assert test_11 returns 8
int test_11() {
    int a[2];
    a[0] = 5;
    a[1] = 3;
    return a[0] + a[1];
}

// global variables test
int a;

// assert test_12 returns 110
int test_12() {
    int *p;
    p = &a;
    *p = 110;
    return *p;
}

int arr[2];

// assert test_13 returns 8
int test_13() {
    int x;
    x = 5;
    arr[0] = 1;
    arr[1] = 2;
    int *p;
    p = arr;
    return p[0] + arr[1] + x;
}

int y[3];

// assert test_14 returns 18
int test_14() {
    int x;
    x = 5;
    int i;
    for (i = 0; i < 3; i = i + 1) {
        y[i] = i * 3 + 3;
    }
    if (x == 5) {
        return y[0] + y[1] + y[2];
    } else {
        return y[0] + y[1];
    }
}

// assert test_15 returns 3
int test_15() {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[0] + y;
}

// assert test_16 returns 97
int test_16() {
    char *x;
    x = "abc";
    return x[0];
}

// global variable initializers test

int gv_17 = 5;
char gv_18[] = "foobar";
int *gv_19 = &gv_17;
char *gv_20 = gv_18 + 3;
int gv_21[] = {1, 2, 3};
int gv_22[5] = {1, 2, 3};
char gv_23[10] = "test";

// assert test_17 returns 5
int test_17() {
    return gv_17;
}

// assert test_18 returns 102
int test_18() {
    return gv_18[0];
}

// assert test_19 returns 5
int test_19() {
    return *gv_19;
}

// assert test_20 returns 97
int test_20() {
    return gv_20[1];
}

// assert test_21 returns 2
int test_21() {
    return gv_21[1];
}

// assert test_22 returns 3
int test_22() {
    return gv_22[2] + gv_22[3] + gv_22[4];
}

// assert test_23 returns 116
int test_23() {
    // t (116) + zero fill
    return gv_23[3] + gv_23[9];
}

// local variable initializer test

// assert test_24 returns 8
int test_24() {
    int x = 8;
    return x;
}

// assert test_25 returns 10
int test_25() {
    int x[] = {1, 2, test_24()};
    return x[1] + x[2];
}

// assert test_26 returns 234
int test_26() {
    char s1[] = "oisu~";
    char *s2 = "xxpoxx";
    int x[3] = {1, 2, 3};
    // 111 + 120 + 3 = 234
    return s1[0] + s2[0] + x[2];
}

// assert test_27 returns 98
int test_27() {
    char s[][4] = {"ok", "bar"};
    // 'b' = 98
    return s[1][0];
}

// helper function for test_28 (nested type test)
// type: func() * [5] int
int (*test_28_helper())[5] {
    int a[5];
    a = calloc(1, sizeof(a));
    a[1] = 10;
    return &a;
}

// assert test_28 returns 10
int test_28() {
    return (*test_28_helper())[1];
}

// assert test_29 returns 1
int test_29() {
    int i = 5;
    if (1 && i > 4) {
        return 1;
    } else {
        return 0;
    }
}

// assert test_30 returns 0
int test_30() {
    // purposefully not initialized
    int *p;
    // test minimal evaluation
    if (0 && *p) {
        return 1;
    } else {
        return 0;
    }
}

// assert test_31 returns 1
int test_31() {
    // purposefully not initialized
    int *p;
    // test minimal evaluation
    if (1 || *p) {
        return 1;
    } else {
        return 0;
    }
}

// assert test_32 returns 1
int test_32() {
    int a = 6;
    int b = 10;
    return 0 || 1 && a > 5 || b < 10;
}

int gv_33 = 1 && 1;

// assert test_33 returns 1
int test_33() {
    return gv_33;
}

typedef int my_int;

// assert test_34 returns 57
int test_34() {
    my_int i = 56;
    my_int (f) = 1;
    return i + f;
}

struct MyStruct {
    int a;
    int b;
};
typedef struct MyStruct MyStruct;

// assert test_35 returns 18
int test_35() {
    struct MyStruct s;
    s.a = 3;
    s.b = 5;
    MyStruct b;
    b.a = 5;
    b.b = 5;
    return s.a + s.b + b.a + b.b;
}

typedef struct MyStruct2 {
    char a;
    char b;
} MyStruct2;

// assert test_36 returns 10
int test_36() {
    MyStruct2 s;
    s.b = 10;
    return s.b;
}

typedef struct {
    int a;
    int b;
    char c;
} MyStruct3;

// assert test_37 returns 15
int test_37() {
    MyStruct3 s;
    s.a = 4;
    s.b = 5;
    s.c = 6;
    return s.a + s.b + s.c;
}

// assert test_38 returns 0
int test_38() {
    int i = 5;
    return !i;
}

// assert test_39 returns 1
int test_39() {
    int i = 0;
    return !i;
}

// assert test_40 returns 0
int test_40() {
    int i = 0;
    return !!i;
}

// assert test_41 returns 6
int test_41() {
    int i = 5;
    ++i;
    ++i;
    --i;
    return i;
}

// assert test_42 returns 97
int test_42() {
    return 'a';
}

int main() {
    assert(1 == 1, "1 == 1 assertion failure");
    assert(0 != 1, "0 != 0 assertion failure");

    assertEquals(55 + 5 - 3, 57, "55 + 5 - 3 does not equal to 57");
    assertEquals(5 + 6 * 7, 47, "5 + 6 * 7 does not equal to 47");
    assertEquals(8 - 3 - 2, 3, "8 - 3 - 2 does not equal to 3");
    assertEquals(5 * (9 - 6), 15, "5 * (9 - 6) does not equal to 15");
    assertEquals((5 + 4) / 2, 4, "(5 + 4) / 2 does not equal to 4");

    assertEquals(4 - 5, -1, "4 - 5 does not equal to -1");
    assertEquals(-10 + 20, 10, "-10 + 20 does not equal to 10");
    assertEquals(5 + -10, -5, "5 + -10 does not equal to -5");

    assertEquals(10 == 2 + 2 * 4, 1, "10 == 2 + 2 * 4 does not equal to 1");
    assertEquals(10 != 10, 0, "10 != 10 does not equal to 0");
    assertEquals(10 > 5, 1, "10 > 5 does not equal to 1");
    assertEquals(2 * 5 >= 11, 0, "2 * 5 >= 11 does not equal to 0");
    assertEquals(10 <= 5, 0, "10 <= 5 does not equal to 0");
    assertEquals(5 < 2 * 3, 1, "5 < 2 * 3 does not equal to 1");

    int a;
    a = 10;
    assertEquals(a, 10, "variable a does not equal to 10 after substitution");
    assertEquals(a + 5, 15, "a + 5 does not equal to 15");
    int b;
    b = 4 + a;
    assertEquals(a * b, 140, "a * b does not equal to 140");

    assertEquals(test_1(), 5, "return value of test_1 does not equal to 5");
    assertEquals(test_2(), 1, "return value of test_2 does not equal to 1");
    assertEquals(test_3(), 5, "return value of test_3 does not equal to 5");
    assertEquals(test_4(), 5, "return value of test_4 does not equal to 5");
    assertEquals(test_5(), 16, "return value of test_5 does not equal to 16");
    assertEquals(test_6(), 17, "return value of test_6 does not equal to 17");
    assertEquals(test_7(), 7, "return value of test_7 does not equal to 7");

    assertEquals(fib(7), 13, "fib(7) does not equal to 13");
    assertEquals(tarai(10, 6, 0), 10, "tarai(10, 6, 0) does not equal to 10");

    assertEquals(test_8(), 3, "return value of test_8 does not equal to 3");
    assertEquals(test_9(), 3, "return value of test_9 does not equal to 3");

    int x;
    int *y;
    char ch;
    assertEquals(sizeof(x), 4, "sizeof(x) does not equal to 4");
    assertEquals(sizeof(y), 8, "sizeof(y) does not equal to 8");
    assertEquals(sizeof(ch), 1, "sizeof(ch) does not equal to 1");
    assertEquals(sizeof(x + 3), 4, "sizeof(x + 3) does not equal to 4");
    assertEquals(sizeof(y + 3), 8, "sizeof(y + 3) does not equal to 8");
    assertEquals(sizeof(*y), 4, "sizeof(*y) does not equal to 4");
    assertEquals(sizeof(1), 4, "sizeof(1) does not equal to 4");
    assertEquals(sizeof(sizeof(1)), 4, "sizeof(sizeof(1)) does not equal to 4");

    assertEquals(test_10(), 3, "return value of test_10 does not equal to 3");
    assertEquals(test_11(), 8, "return value of test_11 does not equal to 8");

    assertEquals(test_12(), 110, "return value of test_12 does not equal to 110");
    assertEquals(test_13(), 8, "return value of test_13 does not equal to 8");
    assertEquals(test_14(), 18, "return value of test_14 does not equal to 18");

    assertEquals(test_15(), 3, "return value of test_15 does not equal to 3");
    assertEquals(test_16(), 97, "return value of test_16 does not equal to 97");

    assertEquals(test_17(), 5, "return value of test_17 does not equal to 5");
    assertEquals(test_18(), 102, "return value of test_18 does not equal to 102");
    assertEquals(test_19(), 5, "return value of test_19 does not equal to 5");
    assertEquals(test_20(), 97, "return value of test_20 does not equal to 97");

    assertEquals(test_21(), 2, "return value of test_21 does not equal to 2");
    assertEquals(test_22(), 3, "return value of test_22 does not equal to 3");
    assertEquals(test_23(), 116, "return value of test_23 does not equal to 116");

    assertEquals(test_24(), 8, "return value of test_24 does not equal to 8");
    assertEquals(test_25(), 10, "return value of test_25 does not equal to 10");
    assertEquals(test_26(), 234, "return value of test_26 does not equal to 234");
    assertEquals(test_27(), 98, "return value of test_27 does not equal to 98");

    assertEquals(test_28(), 10, "return value of test_28 does not equal to 10");

    assertEquals(test_29(), 1, "return value of test_29 does not equal 1");
    assertEquals(test_30(), 0, "return value of test_30 does not equal 0");
    assertEquals(test_31(), 1, "return value of test_31 does not equal 1");
    assertEquals(test_32(), 1, "return value of test_32 does not equal 1");
    assertEquals(test_33(), 1, "return value of test_33 does not equal 1");

    assertEquals(test_34(), 57, "return value of test_34 does not equal 57");

    assertEquals(test_35(), 18, "return value of test_35 does not equal 18");
    assertEquals(test_36(), 10, "return value of test_36 does not equal 10");
    assertEquals(test_37(), 15, "return value of test_37 does not equal 15");

    assertEquals(test_38(), 0, "return value of test_38 does not equal 0");
    assertEquals(test_39(), 1, "return value of test_39 does not equal 1");
    assertEquals(test_40(), 0, "return value of test_40 does not equal 0");

    assertEquals(test_41(), 6, "return value of test_41 does not equal 6");

    assertEquals(test_42(), 97, "return value of test_42 does not equal 97");

    /*
    This is a block comment
    */

    return 0;
}
