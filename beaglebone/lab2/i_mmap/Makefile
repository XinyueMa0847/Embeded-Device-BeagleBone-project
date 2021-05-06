#	Embedded Systems Lab2 makegcc Makefile

# Source file: pushLEDmmap.c, pushLEDmmap.h userLEDmmap.c userLEDmmap.h

# Do everything 
all: m1 m2 


# Cross complie for Bone ubuntu 
debug:
	gcc -g -o userLEDmmap userLEDmmap.c
m1: 
	arm-linux-gnueabihf-gcc -o userLEDmmap userLEDmmap.c

m2: 
	arm-linux-gnueabihf-gcc -o Metronome_led Metronome_led.c

# Clean up 
clean:
	rm 0f pushLEDmmap userLEDmmap 