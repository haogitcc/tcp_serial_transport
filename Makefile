BOARD=MYD
ifeq ($(BOARD), MYD)
# MYD toolchains
$(warning "Myd BOARD=$(BOARD)")
CC=/opt/myd_toolchains/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
LIBS = lib_myd/lib_static
else
# 28x toolchains
$(warning "M28x BOARD=$(BOARD)")
CC=/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc
LIBS = lib_m28x/lib_static
endif

CLFAGS= -Wall

tcp_serial_transport:lan2serial.o \
	lan2serial.o \
	serial.o \
	tcpserver.o \
	utils.o
	$(CC) $(CLFAGS) -o tcp_serial_transport	lan2serial.o \
	serial.o \
	tcpserver.o \
	utils.o \
	 -lpthread  
#-I/opt/include -L/opt/lib/   -L./lib/libdhcpcd -lrt ./$(LIBS)/libmercuryapi.a ./$(LIBS)/libltkc.a ./$(LIBS)/libltkctm.a


lan2serial.o:lan2serial.c lan2serial.h
	$(CC) $(CLFAGS) -c  lan2serial.c
tcpserver.o:tcpserver.c tcpserver.h
	$(CC) $(CLFAGS) -c  tcpserver.c
serial.o:serial.c serial.h
	$(CC) $(CLFAGS) -c serial.c
utils.o:utils.c utils.h
	$(CC) $(CLFAGS) -c utils.c

clean:
	rm tcp_serial_transport  *.o

