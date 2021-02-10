rm ./test
clear
g++ -o test button.cpp test.cpp encoder.cpp dispdrv.cpp led.cpp -lpigpio -lrt
sudo ./test
