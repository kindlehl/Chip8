# __TO BUILD__  
*dependencies*  
* SDL2  
* QT5   

I have included a pre-built executable on the Linux platform, kernel 4.13.0. If this does not work you will have to 
build from source.

To build, just run `make -B`. This will create an executable name "Scheduler" in the current working directory.

# __CONTROLS__  

1234    
qwer    *&nbsp&nbsp Virtual Chip8 key layout.*  
asdf    
zxcv

Control setup depends on ROM, but they only use the aforementioned keys as specified by the Chip8 Docs.  

# __TO RUN__  

`$ ./Chip8 [-OPTIONS] PATH/TO/ROM`  

I have included a collection of ROMs to try. Debugging an emulator is extremely difficult, so this 
is full of bugs in the instruction set. I recommend PONG2, or MAZE. Press the SPACE key at any time to 
get a birds-eye view of the registers. You may even change the contents of each register.
