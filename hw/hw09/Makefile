all: redextract

clean: 
	rm redextract
	rm *.o

redextract: packet.c packet.h pcap-read.c pcap-read.h pcap-process.c pcap-process.h main.c
	gcc packet.c pcap-process.c pcap-read.c main.c -Wall --std=c99 -lpthread -lm -o redextract