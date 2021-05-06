#	Embedded Systems Lab1 makegcc Makefile

# Source file: helloes.c

# Do everything 
all: nc cc 

# Native complie 
nc: 
	gcc -o helloes helloes.c

# Cross complie for Bone ubuntu 

cc: 
	arm-linux-gnueabihf-gcc -o helloes.x helloes.c 

# Clean up 
clean:
	rm 0f helloes helloes.x
