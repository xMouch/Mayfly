echo "COMPILE"
time ./mayflyc testcode/fib.mf
echo "SWITCH THREADED:"
time ./mayfly_switch binary.mayfly
echo "Call/Goto THREADED"
time ./mayfly binary.mayfly
