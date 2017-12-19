
#include "cpu.h"
#include <cstdlib>

void CPU::initializeLabels(){

      QFont labelFont("Courier", 24, 5);
      QFont contentFont("Courier", 12);

      for(int i = 0; i < 17; i++){
          QString string((char)i + 48);  
          registerLabels[i] = new QLabel(string, &debugWindow);
          registerLabels[i]->setFont(labelFont);
          registerLabels[i]->setGeometry(30, i*(debugWindow.height()-100)/17 + 50, 200, (debugWindow.height()-100)/17);
          registerContents[i] = new QTextEdit(QString("0"), registerLabels[i]);
          registerContents[i]->setGeometry(20, 0, registerLabels[i]->width(), registerLabels[i]->height());
          registerContents[i]->setFont(contentFont);
          registerContents[i]->setAlignment(Qt::AlignCenter);

      }
    //this include performs all hard-coded settings for labels
    #include "init.list"

}

void CPU::init(){
    stackPointer = 0;
    PC = 0x200;
    srand(time(NULL));
    _running = true; 
    
    //map keys on KB to keypad for Chip8
    keys[SDLK_r] = 0xD;
    keys[SDLK_x] = 0x0;
    keys[SDLK_1] = 0x1;
    keys[SDLK_2] = 0x2;
    keys[SDLK_3] = 0x3;
    keys[SDLK_q] = 0x4;
    keys[SDLK_w] = 0x5;
    keys[SDLK_e] = 0x6;
    keys[SDLK_a] = 0x7;
    keys[SDLK_s] = 0x8;
    keys[SDLK_d] = 0x9;
    keys[SDLK_z] = 0xA;
    keys[SDLK_c] = 0xB;
    keys[SDLK_4] = 0xC;
    keys[SDLK_f] = 0xE;    
    keys[SDLK_v] = 0xF;    

    unsigned char fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,       //0
        0x20, 0x60, 0x20, 0x20, 0x70,       //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,       //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,       //3
        0x90, 0x90, 0xF0, 0x10, 0x10,       //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,       //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,       //6
        0xF0, 0x10, 0x20, 0x40, 0x40,       //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,       //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,       //9
        0xF0, 0x90, 0xF0, 0x90, 0x90,       //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,       //B
        0xF0, 0x80, 0x80, 0x80, 0xF0,       //C
        0xE0, 0x90, 0x90, 0x90, 0xE0,       //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,       //E
        0xF0, 0x80, 0xF0, 0x80, 0x80        //F
    };

//copy all 16 5-byte fonts into the beginning of Chip8 memory
    memcpy(memory, fontset, 80);

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    pixelSize = ( windowHeight / 32 );

    for( int y= 0; y < 32; y++ ){
        for(int x = 0 ; x < 64; x++){
            pixels[y][x].h = pixelSize;
            pixels[y][x].w = pixelSize;
            pixels[y][x].x = x * pixelSize;
            pixels[y][x].y = y * pixelSize;
        }
    }

 
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
}

void CPU::LoadRom(const char* filename){
    std::ifstream File;
    File.open(filename, std::ios_base::binary);
    if(!File.is_open()){
        std::cout << "ERROR OPENING ROM\n";
        return;
    }

    for(int i = 0x200; !File.eof(); i++ ){
        memory[i] = File.get();
    }

    File.close();
}

CPU::~CPU(){
    for(int i = 0; i < 17; i++){
       delete registerLabels[i];
       delete registerContents[i];
    }

}

CPU::CPU(SDL_Window* win, SDL_Renderer* ren, QApplication* application){
    renderer = ren;
    window = win;
    _showDebug = false;
    app = application;
    debugWindow.setFixedSize(debugWindow.size());
    
    initializeLabels();
}

bool CPU::showDebug(){
    return _showDebug;
}

bool CPU::running(){
    return _running;
}

bool CPU::registersUpdated(){
    return _registersUpdated;
}

void CPU::saveRegisterChanges(){
    for(int i = 0; i < 16; i++){
        registers[i] = registerContents[i]->toPlainText().toInt();
    }
    registerI = registerContents[16]->toPlainText().toInt();
}

void CPU::draw(){

    for(int y = 0; y < 32; y++){
        for(int x = 0; x < 64; x++){
            if(screen[y][x]){
                SDL_SetRenderDrawColor(renderer, 255,255,255,255);
            }else{
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            SDL_RenderFillRect(renderer, &(pixels[y][x]));
        }
    }
    SDL_RenderPresent(renderer);
}

std::string toUpper(std::string str){
    for(int i = 0; i < str.length(); i++){
        if(static_cast<int>(str[i]) >= 97){
            str[i] = static_cast<int>(str[i]) - 32;
        }
    }
    return str;
}

void CPU::loadOpcode(){
    std::stringstream stream;
    stream << std::hex << opcode;
    std::cout << "Opcode to be loaded: " << stream.str() << std::endl;
    std::string opcodeString = toUpper(stream.str());
    currentOpcode->setText(QString(opcodeString.c_str())); 
}

void CPU::loadRegisters(){
    //call all signals
    for(int i = 0; i < 16; i++){
        registerContents[i]->setText(QString::number(registers[i]));
        registerContents[i]->setAlignment(Qt::AlignCenter);
    }  
        registerContents[16]->setText(QString::number(registerI));
        registerContents[16]->setAlignment(Qt::AlignCenter);
}

void CPU::printScreen(){
    system("clear");
    for(int i = 0; i < 32; i++){
        for(int j = 0; j < 64; j++){
            std::cout << static_cast<int>(screen[i][j]) << " ";
        }
        std::cout << std::endl;
    }
}

void CPU::connectLabels(){
    for(int i = 0; i < 16; i++){
   //     connect(registerContents[i], SIGNAL( cursorPositionChanged()  ), registerContents[i], SLOT( selectAll() ));
    }
   //     connect(registerContents[16], SIGNAL( cursorPositionChanged()  ), registerContents[16], SLOT( selectAll() ));

}

void CPU::quit(){
    _running = false;
}

void CPU::disconnectLabels(){
    for(int i = 0; i < 16; i++){
        disconnect(registerContents[i], SIGNAL( cursorPositionChanged()  ), registerContents[i], 0);
    }
        disconnect(registerContents[16], SIGNAL( cursorPositionChanged()  ), registerContents[16], 0);


}

void CPU::push_event(SDL_Event event){
    eventQueue.push(event);
}

void CPU::openWindow(){
                loadOpcode();      
                loadRegisters();
                _registersUpdated = true;
                _showDebug = true;
                debugWindow.show();
                connectLabels();
                app->exec();
                disconnectLabels();
                _showDebug = false;
}

int CPU::waitForKeyPress(){
    SDL_Event e;
    bool hang = true;
    while(hang){
        /* this function takes directly from the event queue because it halts the instruction loop in main
            ergo, CPU's event queue would not be filled until this function ends which is an issue
         */
        if(SDL_PollEvent(&e)){
        //if  there is an event
            if(e.type == SDL_KEYDOWN && keys.find(e.key.keysym.sym) != keys.end()){
                //if that event is a keypress for a key that is mapped to the Chip8 keyboard
                return keys.at(e.key.keysym.sym);
            }
        }   
    }
}

int CPU::wasPressed(int HexKey){
    SDL_Event event;
    std::map<int,int>::iterator it;
    while(!eventQueue.empty()){
        event = static_cast<SDL_Event>(eventQueue.front());
        eventQueue.pop();
        if(event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED){
            it = keys.find(event.key.keysym.sym);       //find key in map "SDLK_..."
            if(it->second == HexKey){
                //if the key pressed is mapped to Chip8 keyboard return 1
                return 1; 
            }
        }

    }   
    //no valid keypress was found
    return 0;
}

/*
 *  draws a sprite that is numBytes tall at (x,y). Sprits are in memory[registerI]
 */
void CPU::DrawBytes(int x, int y, int numBytes){
    int collision = 0;
    for(int i = 0; i < numBytes; i++){
        for(int j = 0; j < 8; j++){
            //check each pixel with the bytes beginning at location (registerI) to check for collision
            if(screen[(y+i)%32][(x+j)%64] && ((memory[registerI + i]) & (0x80 >> j)))       //if pixel is on already
                collision = 1;
            //perform XOR writing to screen
            screen[(y+i)%32][(x+j)%64] = screen[(y+i)%32][(x+j)%64] ^ ((memory[registerI + i] & (0x80 >> j )) >> (7-j));
        }   
    }
    registers[0xF] = collision;
}

void CPU::ClearScreen(){
    //sets each pixel state to 0
    for(int i = 0; i < 32; i++){
        for(int j = 0; j < 64; j++){
            screen[i][j] = 0;
        }
    }
}


void CPU::cycle(){
    
    opcode = memory[PC] << 8;    //grab first byte of instruction

    opcode += memory[PC+1];  //shift over byte and add second byte of instruction
    PC +=2;

    switch(opcode & 0xF000){    //check most significant nibble
        case 0x0000:    //most significant nibble is zero
            
            switch(opcode){
                case 0x00E0:        //Clear display
                    ClearScreen();
                    break;
                case 0x00EE:        //00EE return from subroutine
                    PC = stack[stackPointer];
                    stackPointer--;
                    break;
            }
            break;
        case 0x1000: // 1nnn - jump to nnn
            PC = opcode & 0x0FFF;
            break;
        case 0x2000:
            stackPointer++;
            stack[stackPointer] = PC;
            PC = opcode & 0x0FFF;
            break;
        case 0x3000:
            if(registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)){
                PC +=2;
            }
            break;
        case 0x4000:
            if(registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)){
               PC +=2;
            }
            break;

        case 0x5000:
            if(registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4]){
                PC += 2;
            }
            break;
        case 0x6000:
            registers[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            break;

        case 0x7000:
            registers[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            break;
        case 0x8000:

            switch(opcode & 0xF00F){
                case 0x8000:
                    registers[(opcode & 0x0F00)>>8] = registers[(opcode & 0x00F0)>>4];
                    break;
                case 0x8001:
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4] | registers[(opcode & 0x0F00) >> 8]; 
                    break;
                case 0x8002:
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4] & registers[(opcode & 0x0F00) >> 8]; 
                    break;
                case 0x8003:
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4] ^ registers[(opcode & 0x0F00) >> 8]; 
                    break;
                case 0x8004:        //add and store carry
                    if(registers[(opcode & 0x0F00) >> 8] + registers[(opcode & 0x00F0) >> 4] > 255){
                        registers[0xF] = 1;
                    }else{
                        registers[0xF] = 0;
                    }
                    registers[(opcode & 0x0F00) >> 8]= (registers[(opcode & 0x0F00) >> 8] + registers[(opcode & 0x00F0) >> 4]) % 256; //added modulus to ensure proper carry behavior
                    break;
                case 0x8005:
                    if(registers[(opcode & 0x0F00) >> 8] > registers[(opcode & 0x00F0) >> 4]){
                        registers[0xF] = 1;
                    }else{
                        registers[0xF] = 0;
                    }
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x0F00) >> 8] - registers[(opcode & 0x00F0) >> 4];
                    break;
                case 0x8006:
                    if(registers[(opcode & 0x0F00)>>8] % 2 == 1){
                        registers[0xF] = 1;
                    }else{
                        registers[0xF] = 0;
                    }
                    registers[(opcode & 0x0F00) >> 8] >>= 1;
                    break;
                case 0x8007:
                    if(registers[(opcode & 0x00F0) >> 4] > registers[(opcode & 0x0F00) >> 8]){
                        registers[0xF] = 1;
                    }else{
                        registers[0xF] = 0;
                    }
                    registers[(opcode & 0x0F00)>>8] = registers[(opcode & 0x00F0) >> 4] - registers[(opcode & 0x0F00) >> 8];
                    break;
                case 0x800E:
                    if(registers[(opcode & 0x0F00)>>8] & 0x80){
                        registers[0xF] = 1;
                    }else{
                        registers[0xF] = 0;
                    }
                    registers[(opcode & 0x0F00) >> 8] <<= 1;
                    break;
            }
            break;
        case 0x9000:
            if(registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x00F0) >> 4]){
                PC += 2;
            }
            break;
        case 0xA000:
            registerI = (opcode & 0x0FFF);
            break;
        case 0xB000:
            PC = ((opcode & 0x0FFF) + registers[0]);
            break;
        case 0xC000:
            registers[(opcode & 0x0F00) >> 8] = rand()%256 & (opcode & 0x00FF);
            break;
        case 0xD000:
            DrawBytes(registers[((opcode & 0x0F00) >> 8)], registers[((opcode & 0x00F0) >> 4 )], (opcode & 0x000F));
            break;
        case 0xE000:
            switch(opcode & 0xF00F){
                case 0xE00E:
                    if(wasPressed(registers[(opcode & 0x0F00) >> 8])){
                        PC+=2;
                    }
                    break;
                case 0xE001:
                    if(!wasPressed(registers[(opcode & 0x0F00) >> 8])){
                        PC += 2;
                    }
                    break;

            }
            break;
        case 0xF000:
            switch(opcode & 0xF0FF){
                case 0xF007:
                    registers[(opcode & 0x0F00) >> 8] = delayTimer;
                    break;
                case 0xF00A:
                    registers[(opcode & 0x0F00) >> 8] = waitForKeyPress();
                    break;
                case 0xF015:
                    delayTimer = registers[(opcode & 0x0F00) >> 8];
                    break;
                case 0xF018:
                    soundTimer = registers[(opcode & 0x0F00)>>8];
                    break;
                case 0xF01E:
                    registerI += registers[(opcode & 0x0F00)>>8];
                    break;
                case 0xF029:
                    
                    registerI =  5*(registers[(opcode & 0x0F00) >> 8]);
                    break;
                case 0xF033:
                    memory[registerI] = registers[((opcode & 0x0F00) >>8)] / 100;
                    memory[registerI + 1] = (registers[((opcode & 0x0F00) >> 8)] /10)% 10;
                    memory[registerI + 2] = (registers[((opcode & 0x0F00)>>8)]%100)%10;
                    std::cout << "BCD: (" << static_cast<int>(memory[registerI]) << ", " << static_cast<int>(memory[registerI + 1]) << ", " << static_cast<int>(memory[registerI + 2]) << ")" << std::endl;
                    break;
                case 0xF055:
                    for(int i = 0; i < ((opcode & 0x0F00)>>8) ; i++){ //+1 to i
                        memory[registerI] = registers[i];
                        registerI++;
                    }
                    registerI++;
                    break;
                case 0xF065:
                    for(int i = 0; i < ((opcode & 0x0F00)>>8) ; i++){//+1 to i
                        registers[i] = memory[registerI];
                        registerI++;             
                    }
                    registerI++;
                    break;

            }

            break;
    }

  

    if(delayTimer > 0)
        delayTimer--;
    if(soundTimer > 0)
        soundTimer--;
    if(registersUpdated()){
        //if user changes register contents manually
        _registersUpdated = false;
        saveRegisterChanges();
    }
}

