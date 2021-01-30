#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./frcc "$input" > tmp.s
    cc -o tmp tmp.s test1.o
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
assert 2 "foo() { return 1; } main() { return foo() + 1; }"
assert 3 "main() { x = 3; y = x; return y; }"
assert 3 "main() { x = 3; y = &x; return *y; }"
assert 5 "main() { x = 3; y = 5; z = &y; return *z; }"
assert 3 "main() { x = 3; y = 5; z = &x; return *z; }"
assert 3 "main() { x = 3; y = 5; z = &y + 8; return *z; }"

echo OK
