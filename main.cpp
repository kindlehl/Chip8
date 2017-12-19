#include "cpu.h"
#include <iostream>
#include <istream>
#include <math.h>
#include <SDL2/SDL.h>
#include <queue> 
#include <memory>

#define IPS 300          //instructions per second
#define SPI (float)1/IPS       //seconds per instruction
#define HEIGHT 320

using namespace std;


int main(int argc, char** argv){
    if(argc == 1){
        cout << "No ROM supplied. \nUsage is ./Chip8 [-options] ROM_Path\n";
        return 0;
    }
    QApplication app(argc, argv);

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("THIS", 0, 0, 2 * HEIGHT,HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    queue<SDL_Event> eventQueue;
    SDL_Event event;
    clock_t startOfCycle, endOfCycle;
    CPU processor(window, renderer, &app);
    processor.init();
    if(processor.LoadRom(argv[1]) == false){
        return 0;
    }
     
           

    while(processor.running()){
    
             //while there are events in queue
        while(SDL_PollEvent(&event)){    
            //process event is a keypress
            
            //if the event is a key press that isnt space or escapei
            if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE ){
                //if space is pressed
                processor.openWindow();
            }

            else if(event.type == SDL_KEYDOWN && !(event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_ESCAPE)){
                 //toss it into the processor's event queue
                 processor.push_event(event);
            }
           
            else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ){
                //if escape is pressed
                processor.quit();
            }

        }
        startOfCycle = clock(); 
        processor.cycle();
        processor.draw();
        endOfCycle = clock();
            

        if((endOfCycle-startOfCycle) / CLOCKS_PER_SEC < SPI)
            SDL_Delay((SPI - (endOfCycle-startOfCycle) / CLOCKS_PER_SEC) * 1000);
        else{
            cout << "Instruction took longer than expected\n";
        }

        //processor.printScreen();
    }
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    return 0;
}
