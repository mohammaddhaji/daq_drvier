
LIBS = -lgpiod -lad7616 -lltc2668 -lmcp23s17 -lrttimer -luart -lrt -lm
FLAGS = -L/usr/lib -I/usr/include
LIB_PATH = /root/src/c

all:
	gcc src/test/main.c -o main.run $(LIBS) $(FLAGS)


libs:
	gcc -fpic -c $(LIB_PATH)/ad7616.c $(FLAGS)
	gcc -shared -o $(LIB_PATH)/libad7616.so $(LIB_PATH)/ad7616.o $(FLAGS)

	gcc -fpic -c $(LIB_PATH)/ltc2668.c $(FLAGS)
	gcc -shared -o $(LIB_PATH)/libltc2668.so $(LIB_PATH)/ltc2668.o $(FLAGS)

	gcc -fpic -c $(LIB_PATH)/mcp23s17.c $(FLAGS)
	gcc -shared -o $(LIB_PATH)/libmcp23s17.so $(LIB_PATH)/mcp23s17.o $(FLAGS)

	gcc -fpic -c $(LIB_PATH)/uart.c $(FLAGS)
	gcc -shared -o $(LIB_PATH)/libuart.so $(LIB_PATH)/uart.o $(FLAGS)

	gcc -fpic -c $(LIB_PATH)/rttimer.c $(FLAGS)
	gcc -shared -o $(LIB_PATH)/librttimer.so $(LIB_PATH)/rttimer.o $(FLAGS)

	cp $(LIB_PATH)/*.so /usr/lib/
	cp $(LIB_PATH)/*.h /usr/include/
	rm $(LIB_PATH)/*.so

	gcc src/test/main.c -o main.run $(LIBS) $(FLAGS)

