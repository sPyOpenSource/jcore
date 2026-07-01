cd sample/17-fractal && make
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio -semihosting 2>&1 &
QPID=$!
sleep 5
kill $QPID 2>/dev/null
wait $QPID 2>/dev/null
