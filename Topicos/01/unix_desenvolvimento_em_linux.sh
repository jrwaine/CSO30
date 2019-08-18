echo "1)"
cc -static -o piStc.out pi.c -lm
cc -o piDyn.out pi.c -lm
ls -l piDyn.out piStc.out
nm piDyn.out piStc.out
echo ""

echo "2)"
# estática
gcc -c aritm.c
ar rvs libtest.a aritm.o
# dinâmica
gcc -fPIC -c aritm.c
gcc -g -shared -Wl,-soname=libtest.so.0 -o libtest.so.0.0 aritm.o
export LD_LIBRARY_PATH=./
#mv libtest.so.0.0 /usr/local/lib
ln -s libtest.so.0.0 libtest.so.0
ln -s libtest.so.0 libtest.so
echo ""

echo "3)"
gcc -static -o aritmStc.out testeAritm.c libtest.a
gcc -o aritmDyn.out -L./ ./libtest.so testeAritm.c
echo "Estática:"
./aritmStc.out
echo "Dinâmica:"
./aritmDyn.out
ls -l aritmStc.out aritmDyn.out
nm aritmStc.out aritmDyn.out
echo ""

echo "4)"
cc -g -o aritmDbg.out testeAritm.c -ltest
ddd ./aritmDbg.out
echo ""

echo "5)"
gcc -static -pg -g -o ./aritmGprof.out testeAritm.c libtest.a
./aritmGprof.out
gprof ./aritmGprof.out gmon.out
echo ""

echo "6)"
strace uptime
echo ""

rm *.o
