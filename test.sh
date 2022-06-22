echo "COMPILE"
time ./mayflyc testcode/fib.mf
echo "CFIB -O0"
time ./cfib
echo "JAVA"
time java Main.java
echo "SWITCH THREADED:"
time ./mayfly_switch binary.mayfly
echo "Call/Goto THREADED"
time ./mayfly binary.mayfly

