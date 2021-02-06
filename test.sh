#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./frcc "$input" > tmp.s
    cc -o tmp tmp.s -no-pie
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

# assert 0 0
# assert 43 43
# assert 21 "5+20-4"
# assert 41 " 12 + 34 - 5 "
# assert 47 '5+6*7'
# assert 15 '5*(9-6)'
# assert 4 '(3+5)/2'
# assert 10 "-10+20"
# assert 4 "(3+5)/+2"

# assert 0 '0==1'
# assert 1 '42==42'
# assert 1 '0!=1'
# assert 0 '42!=42'

# assert 1 '0<1'
# assert 0 '1<1'
# assert 0 '2<1'
# assert 1 '0<=1'
# assert 1 '1<=1'
# assert 0 '2<=1'

# assert 1 '1>0'
# assert 0 '1>1'
# assert 0 '1>2'
# assert 1 '1>=0'
# assert 1 '1>=1'
# assert 0 '1>=2'

# assert 0 "0;"
# assert 4 "(3+5)/+2;"
# assert 4 "a=3;(a+5)/+2;"
# assert 4 "hoge=3;fuga=2+hoge;(hoge+fuga)/+2;"
# assert 4 "hoge=3;fuga=2+hoge;return (hoge+fuga)/+2; 2;"
# assert 2 "hoge=2;return hoge;"
# assert 5 "hoge=2;while(hoge < 5) hoge = hoge+1; return hoge;"
# assert 7 "hoge=2;for(i=0;i<5;i=i+1) hoge=hoge+1; return hoge;"
# assert 1 "hoge=2;if(hoge==2) return 1;return 2;"
# assert 3 "{ 1; 2; 3; }"
# assert 3 "hoge=2;foo();return hoge+1;"
# assert 3 "hoge=2;return hoge+1;"

# assert 1 "main() { return 1; }"
# assert 2 "foo() { return 1; } main() { return foo() + 1; }"
# assert 3 "main() { x = 3; y = x; return y; }"
# assert 3 "main() { x = 3; y = &x; return *y; }"
# assert 5 "main() { x = 3; y = 5; z = &y; return *z; }"
# assert 3 "main() { x = 3; y = 5; z = &x; return *z; }"
# assert 3 "main() { x = 3; y = 5; z = &y + 8; return *z; }"

#assert 2 "int foo() { return 1; } int main() { return foo() + 1; }"
# assert 3 "int main() { int x; int y; x = 3; y = x; return y; }"
# assert 3 "int main() { int x; int* y; x = 3; y = &x; return *y; }"
# assert 5 "int main() { int x; int yoo; x = 3; yoo = 5; return yoo; }"
# assert 5 "int main() { int x; int yoo; x = 3; yoo = 5; int* z; z = &yoo; return *z; }"
# assert 5 "int main() { int x; int y; x = 3; y = 5; int* z; z = &y; return *z; }"
# assert 3 "int main() { int x; int y; int* z; x = 3; y = 5; z = &x; return *z; }"
# assert 3 "int main() { int x; int y; int* z; x = 3; y = 5; z = &y + 2; return *z; }"
# assert 3 "int foo(int x, int y) { return x+y; } int main() { return foo(1,2); }"
# assert 3 "int main() {int x; int *y; y=&x; *y=3; return x;}"
# assert 4 "int main() {int x; return sizeof(x);}"
# assert 8 "int main() {int *x; return sizeof(x);}"
# assert 4 "int main() {return sizeof(1);}"
# assert 3 "int main() {int a[2]; *a=1; *(a+1)=2; int *p; p=a; return *p+*(p+1);}"
# assert 3 "int main() {int a[2]; a[0]=1; a[1]=2; return a[1]+1;}"
# assert 3 "int main() {int *x; int a[2]; a[0]=1; a[1]=2; return a[1]+1;}"
# assert 3 "int x; int y; int main() { x = 3; y = x; return y; }"
# assert 3 "int x; int* y; int main() { x = 3; y = &x; return *y; }"
# assert 1 "int a[2]; int main() {*a=1; return 1; }"
# assert 1 "int a[2]; int main() {*a=1; return *a; }"
# assert 1 "int a[2]; int main() {*a=1; *(a+1)=2; return *a; }"
# assert 3 "int a[2]; int main() {*a=1; *(a+1)=2; int *p; p=a; return *p+*(p+1);}"
# assert 5 "int main() { char x[3]; x[0]=1; x[1]=2; int y; y=4; return x[0]+y; }"
# assert 97 "int main() { char *x; x = \"abc\"; return x[0]; }"

assert 1 "./tests/a.c"
assert 5 "./tests/b.c"
assert 5 "./tests/c.c"

echo OK
