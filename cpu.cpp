
#include "cpu.h"
#include <cstdlib>

#define KK(code) ((code & 0x00FF))
#define xNibble(code) ((code & 0x0F00) >> 8)
#define yNibble(code) ((code & 0x00F0) >> 4)
#define Vx(code) (registers[xNibble(code)])
#define Vy(code) (registers[yNibble(code)])


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
            pixels[y][x].h = pixels[y][x].w = pixelSize;
            pixels[y][x].x = x * pixelSize;
            pixels[y][x].y = y * pixelSize;
        }
    }
//    SDL_AudioSpec want;
//    want.format = AUDIO_S8;
//    want.channels = 2;
//    want.freq = 48000;
//    want.samples = 4096;

//	AudioDeviceID = SDL_OpenAudioDevice(NULL, 0, &want, &AudioSpec, 0);
//    if(SDL_LoadWAV("AUDIO/beep.wav", &AudioSpec, &beepData, &beepLength) == NULL){
//        std::cout << "Could not open audio file" << std::endl;
//        std::cout << SDL_GetError() << std:: endl;
//    }
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
}
/*
 *  Takes in a file, tries to open it. If it cannot open filename, return false.
 *  If it reads the filename and loads the ROM into memory, return true.
 */
bool CPU::LoadRom(const char* filename){
    std::ifstream File;
    File.open(filename, std::ios_base::binary);
    if(!File.is_open()){
        //if file failed to open
        std::cout << "ERROR OPENING ROM\n";
        return false;
    }

    for(int i = 0x200; !File.eof(); i++ ){
        //load ROM into memory
        memory[i] = File.get();
    }

    File.close();
    return true;
}


/*
 *  All members that are pointers in CPU belong to QT widgets, which are 
 *  automatically deleted when their parent object goes out of scope. 
 *  No smart pointers or deletes necessary
 */
CPU::~CPU(){
    //SDL_FreeWAV(beepData);
}

/*
 *  Takes window and renderer from main. In future, it is possible to derive the renderer using just 
 *  the window from main. QApplication remains as its ctor takes main's arguments
 *
 */
CPU::CPU(SDL_Window* win, SDL_Renderer* ren, QApplication* application){
    renderer = ren;
    window = win;
    app = application;
    debugWindow.setFixedSize(debugWindow.size());   //lock down window size
    initializeLabels();
}


bool CPU::running() const {
    return _running;
}

bool CPU::registersUpdated() const {
    return _registersUpdated;
}

/*
 *  Takes in contents from registers in GUI and writes them to the corresponding registers
 */
void CPU::saveRegisterChanges(){
    for(int i = 0; i < 16; i++){
        registers[i] = registerContents[i]->toPlainText().toInt();
    }
    registerI = registerContents[16]->toPlainText().toInt();
}

/*
 *  Loops through pixel array, drawing black rects if pixel value is 0
 *  draws white rects if pixel value is 1
 */
void CPU::draw() const {
    unsigned char pixelState;     //1 or 0
    unsigned char pixelColorScale; //255 or 0
    for(int y = 0; y < 32; y++){
        for(int x = 0; x < 64; x++){
            pixelState = screen[y][x];
            pixelColorScale = 255 * static_cast<int>(pixelState);
            SDL_SetRenderDrawColor(renderer, pixelColorScale, pixelColorScale, pixelColorScale, 255);
            SDL_RenderFillRect(renderer, &(pixels[y][x]));
        }
    }
    SDL_RenderPresent(renderer);
}

/*
 *  takes in a string as parameter, returns uppercase version of string 
 */
std::string toUpper(std::string str){
    for(int i = 0; i < str.length(); i++){
        if(static_cast<int>(str[i]) >= 97){
            str[i] = static_cast<int>(str[i]) - 32;
        }
    }
    return str;
}

/*
 *  Loads current instruction's OPCODE into its text box in GUI
 */
void CPU::loadOpcode(){
    std::stringstream stream;
    stream << std::hex << opcode;
    std::string opcodeString = toUpper(stream.str());
    currentOpcode->setText(QString(opcodeString.c_str())); 
}

/*
 *  Loads current contents of registers into text boxes in GUI
 */
void CPU::loadRegisters(){
    //call all signals
    for(int i = 0; i < 16; i++){
        registerContents[i]->setText(QString::number(registers[i]));
        registerContents[i]->setAlignment(Qt::AlignCenter);
    }  
        registerContents[16]->setText(QString::number(registerI));
        registerContents[16]->setAlignment(Qt::AlignCenter);
}
/*
 *  prints out integer representation of screen. Used for debugging purposes
 */
void CPU::printScreen() const {
    system("clear");
    for(int i = 0; i < 32; i++){
        for(int j = 0; j < 64; j++){
            std::cout << static_cast<int>(screen[i][j]) << " ";
        }
        std::cout << std::endl;
    }
}

//disrupts loop in main
void CPU::quit(){
    _running = false;
}

/*
 *  Takes events that are moderated by main and pushes them onto the Chip8's private event queue.
 *  This is useful because it allows for special keys to be set so that Chip8's "keypress detection"
 *  functions do not interfere with those special keys. 
 */
void CPU::push_event(const SDL_Event& event){
    eventQueue.push(event);
}

void CPU::openWindow(){
    loadOpcode();      
    loadRegisters();
    _registersUpdated = true;
    debugWindow.show();
    app->exec();
}
/*
 *  hangs program until a valid Chip8 interpreter key is pressed
 */
int CPU::waitForKeyPress() const {
    SDL_Event event;
    while(1){
        /* this function takes directly from the event queue because it halts the instruction loop in main
            ergo, CPU's event queue would not be filled until this function ends which is an issue
         */
        if(SDL_PollEvent(&event)){
        //if  there is an event
            if(event.type == SDL_KEYDOWN && keys.find(event.key.keysym.sym) != keys.end()){
                //if that event is a keypress for a key that is mapped to the Chip8 keyboard
                keys.find(event.key.keysym.sym)->second;
                //return the symbol for that key
            }
        }   
    }
}

/*
 *  Checks CPU's internal event queue to see if a keypress identical to HexKey in the parameter list
 */
int CPU::wasPressed(int HexKey){
    SDL_Event event;
//    std::cout << "wasPressed() was called just now for " << std::hex << HexKey << std::dec << std::endl; 




    while(!eventQueue.empty()){
        event = eventQueue.front();
        eventQueue.pop();
        if(event.type == SDL_KEYDOWN){
//            std::cout << "Detected keypress of " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
            auto it = keys.find(event.key.keysym.sym);       //find key in map "SDLK_..."

            if(it->second == HexKey){
                std::cout << "Iterator points to second-" << std::hex << it->second << std::dec <<std::endl;

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

//void CPU::playSound() const {
//    if(SDL_GetQueuedAudioSize(AudioDeviceID) < 100){
//        SDL_QueueAudio(AudioDeviceID, beepData, beepLength);
//    }
//    SDL_PauseAudioDevice(AudioDeviceID, 1);    
//}

void CPU::ClearScreen(){
    //sets each pixel state to 0
    for(int i = 0; i < 32; i++){
        for(int j = 0; j < 64; j++){
            screen[i][j] = 0;
        }
    }
}

/*
 *  Referenced Cowgod's Chip8 Technical Reference: evernay.free.fr/hacks/chip8/C8TECH10.HTM 
 */
void CPU::cycle(){
    
    opcode = memory[PC] << 8;    //grab first byte of instruction

    opcode += memory[PC+1];  //shift over byte and add second byte of instruction
    PC +=2;                 //set program counter to next instruction

    switch(opcode & 0xF000){    //check most significant nibble
        case 0x0000:    //most significant nibble is zero
            
            switch(opcode){
                case 0x00E0:        //Clear display
                    ClearScreen();
                    break;
                case 0x00EE:        //00EE return from subroutine
                    PC = stack[stackPointer--];
                    break;
            }
            break;
        case 0x1000: // 1nnn - jump to nnn
            PC = opcode & 0xFFF;
            break;
        case 0x2000:
            stack[++stackPointer] = PC;       //0x0nnn; call subroutine at nnn
            PC = opcode & 0x0FFF;
            break;
        case 0x3000:    //3xkk; skip next instruction if Vx == kk
            if(Vx(opcode) == KK(opcode)){
                PC +=2;
            }
            break;
        case 0x4000:    //4xkk; skip next instruction if Vx != kk
            if(Vx(opcode) != KK(opcode)){
               PC +=2;
            }
            break;

        case 0x5000:    //5xy0; Skip next instruction if Vx == Vy
            if(Vx(opcode) == Vy(opcode)){
                PC += 2;
            }
            break;
        case 0x6000:    //6xkk; set Vx = kk
            Vx(opcode) = KK(opcode);
            break;

        case 0x7000:    //7xkk; set Vx = Vx + kk
            Vx(opcode) += KK(opcode);
            break;
        case 0x8000:    

            switch(opcode & 0xF00F){
                case 0x8000:    //8xy0; set Vx = Vy
                    Vx(opcode) = Vy(opcode);
                    break;
                case 0x8001:    //8xy1; set Vx = Vx | Vy
                    Vx(opcode) |= Vy(opcode); 
                    break;
                case 0x8002:    //8xy2; set Vx = Vx & Vy
                    Vx(opcode) &= Vy(opcode); 
                    break;
                case 0x8003:    //8xy3; set Vx = Vx ^ Vy
                    Vx(opcode) ^= Vy(opcode); 
                    break;
                case 0x8004:    //8xy4; Vx = Vx + Vy, store carry in VF
                    registers[0xF] = Vx(opcode) + Vy(opcode) > 255 ? 1 : 0;
                    Vx(opcode) += Vy(opcode);
                    break;
                case 0x8005:    //8xy5 Vx = Vx - Vy, VF = !borrow
                    registers[0xF] = Vx(opcode) > Vy(opcode) ? 1 : 0;
                    Vx(opcode) = Vx(opcode) - Vy(opcode);
                    break;
                case 0x8006:    //8xy6 Vx = Vx shifted Right once, set truncation
                    registers[0xF] = Vx(opcode) % 2 == 1 ? 1 : 0;
                    Vx(opcode) >>= 1;
                    break;
                case 0x8007:    //8xy7; Vx = Vy - Vx, VF = NOT Borrow
                    registers[0xF] = Vy(opcode) > Vx(opcode) ? 1 : 0;
                    Vx(opcode) = Vy(opcode) - Vx(opcode);
                    break;
                case 0x800E:    //8xyE; shift Vx left. if msb of Vx is 1, set VF
                    registers[0xF] = Vx(opcode) >= 128 ? 1 : 0;
                    Vx(opcode) <<= 1;
                    break;
            }
            break;
        case 0x9000:    //9xy0; skip next instruction if Vx != Vy
            if(Vx(opcode) != Vy(opcode)){
                PC += 2;
            }
            break;
        case 0xA000:    //Annn; set I = nnn
            registerI = (opcode & 0x0FFF);
            break;
        case 0xB000:    //Bnnn; jump to location nnn + V0
            PC = ((opcode & 0x0FFF) + registers[0]) & 0xFFF;
            break;
        case 0xC000:    //Cxkk; Vx = random 0-255&kk
            {
                int random = rand()%256;
                std::cout << "Randomly generated num: " << random << std::endl;
                Vx(opcode) = random & KK(opcode);
            }
            break;
        case 0xD000:    //Dxyn; draw n-byte sprite at location I at location (Vx, Vy) on screen, set VF = collision
            DrawBytes(Vx(opcode), Vy(opcode), (opcode & 0x000F));
            break;
        case 0xE000: 
            switch(opcode & 0xF00F){
                case 0xE00E: //Ex9E: skips next instruction if key wth value of Vx is pressed
                    if(wasPressed(Vx(opcode))){
                        std::cout << "The Key " << xNibble(opcode) << " was pressed; Skip next instruction";
                        PC+=2;
                    }
                    break;
                case 0xE001:
                    if(!wasPressed(Vx(opcode))){
                        PC += 2;
                    }
                    break;

            }
            break;
        case 0xF000:
            switch(opcode & 0xF0FF){
                case 0xF007: //Fx07; Vx = delay timer
                    Vx(opcode) = delayTimer;
                    break;
                case 0xF00A: //Fx0A; wait for key press, store value in Vx
                    Vx(opcode) = waitForKeyPress();
                    std::cout << "Waited for key press, got back: " << Vx(opcode) << std::endl;
                    break;
                case 0xF015: //Fx15 set delay timer to Vx
                    delayTimer = Vx(opcode);
                    break;
                case 0xF018:  //Fx18; set sound timer to Vx
                    soundTimer = Vx(opcode);
                    break;
                case 0xF01E:  //Fx1E; set I to I + Vx
                    registerI += Vx(opcode);
                    break;
                case 0xF029:  //Fx29; Set I to address of digit in Vx
                    
                    registerI =  5*(Vx(opcode));
                    break;
                case 0xF033:
                    memory[registerI] = Vx(opcode) / 100;
                    memory[registerI + 1] = (Vx(opcode) /10)% 10;
                    memory[registerI + 2] = (Vx(opcode)%100)%10;
                    std::cout << "BCD: (" << static_cast<int>(memory[registerI]) << ", " << static_cast<int>(memory[registerI + 1]) << ", " << static_cast<int>(memory[registerI + 2]) << ")" << std::endl;
                    break;
                case 0xF055: //Fx55; store registers V0 through Vx in memory at address I
                    for(int i = 0; i <= Vx(opcode) ; i++){ //+1 to i
                        memory[registerI+i] = registers[i];
                    }
                    break;
                case 0xF065: //Fx65; read registers V0 through Vx from memory starting at I
                    for(int i = 0; i <= Vx(opcode) ; i++){//+1 to i
                        registers[i] = memory[registerI+i];
                                     
                    }
                    break;

            }

            break;
        default:
            std::cout << "Missed instruction: " << std::hex << opcode << std::endl;
    }

  

    if(delayTimer > 0)
        delayTimer--;
    if(soundTimer > 0){
 //       playSound(); enable in header
        soundTimer--;
    }else{
 //       playSound();
    }
    if(registersUpdated()){
        //if user changes register contents manually
        _registersUpdated = false;
        saveRegisterChanges();
    }
}

