#CPPFLAG=-I/opt/homebrew/include

all: czmq

czmq : c-zmq.c
# For a Mac with a localized install via homebrew
#	gcc -I/opt/homebrew/include -L/opt/homebrew/lib c-zmq.c -l zmq -o czmq
# Otherwise use
	gcc c-zmq.c -l zmq -o czmq