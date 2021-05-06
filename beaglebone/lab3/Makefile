all: pc cc

pc: 
	gcc -o metronome_tui_thread metronome_tui_thread.c -pthread

cc: 
	arm-linux-gnueabihf-gcc -pthread -o metronome_tui_thread metronome_tui_thread.c 

clean:
	rm metronome_tui_thread 
