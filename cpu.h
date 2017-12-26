#ifndef CPU_H
#define CPU_H

#include <memory>
#include <sstream>
#include <QTextEdit>
#include <QFont>
#include <QWidget>
#include <QApplication>
#include <QLabel>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <map>
#include <queue>

class CPU : public QObject{
    Q_OBJECT
    public:
        CPU(SDL_Window*,SDL_Renderer*, QApplication*);
        void init();
        void initializeLabels();
        bool LoadRom(const char*);
        void loadOpcode();
        void openWindow();
        void cycle();
        void ClearScreen();
        void push_event( const SDL_Event& );
        int wasPressed(int);
        int waitForKeyPress() const;
        void draw() const;
        void printScreen() const;
        void DrawBytes(int, int, int);
        bool registersUpdated() const;
        bool running() const;
        ~CPU();
//        void playSound() const ;
        void loadRegisters();
        void saveRegisterChanges();
        void quit();

    private:
        
        QApplication* app;
        QWidget debugWindow;
        QTextEdit* registerContents[17];
        QLabel* OpcodeLabel;
        QTextEdit* currentOpcode;
        QLabel* registerLabels[17];
        QLabel* RegisterLabel;
        std::queue<SDL_Event> eventQueue;
        int windowHeight, windowWidth;
        int pixelSize;
        unsigned short int opcode;
        unsigned char memory[4096] = {0};
        unsigned char registers[16] = {0};
        unsigned char delayTimer = 0;
        unsigned char soundTimer = 0;
        unsigned short int registerI = 0;
        unsigned short int PC = 0;
        unsigned char stackPointer = 0;
        unsigned int stack[17] = {0};
        unsigned char screen[32][64] = {{0}};
        std::map<int,int> keys;
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Rect pixels[32][64];
//        SDL_AudioDeviceID AudioDeviceID;
//        SDL_AudioSpec AudioSpec;
//        Uint32 beepLength;
//        Uint8* beepData;
        bool _registersUpdated;
        bool _running;
};
#endif
