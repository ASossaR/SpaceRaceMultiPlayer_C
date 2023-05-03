#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>

//Includes for UART--------------------------------------------------------
#include <string.h>
#include <stdbool.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <pthread.h>
#include <sys/select.h>
#include <arpa/inet.h> //inet_addr
//------------------------------------------------------------------------

int8_t maestro_esclavo = 0;
int8_t ex = 0;
int8_t musgo = 0;

//Screen dimension
const int SCREEN_WIDTH  = 128; //window height
const int SCREEN_HEIGHT = 64;  //window width

SDL_Window *Window;  	//The window we'll be rendering
SDL_Renderer *Renderer;	//The renderer SDL will use to draw

//Constants for UART--------------------------------------------------------
int puerto_serial,ndfs;
fd_set all_set, r_set; //file descriptors to use on select()
struct timeval tv;
int TamMsj;
uint8_t buffer;

int8_t button_enter = 0;
int8_t button_up[2] = {0};
int8_t button_down[2] = {0};

#define xmax 128
#define xmin 0
#define ymax 64
#define ymin 0

int8_t step_ship = 2;
int8_t long_ship = 8;

int8_t Score[2] = {0};

int delay_asteroids = 0;
int count_delay_asteroids;
int8_t count_active_ast = 0;
int8_t step_asteroid = 3;
int8_t counter_ast = 0;
int8_t len_asteroid = 3;

int8_t delay_net_refresh = 10;
int8_t counter_delay_net_refresh;

int8_t net_counter;
int8_t net_delay_counter = 1;
int8_t long_net = 0;

int warning_collision;

int8_t flag_reset;
int8_t flag_point;
int8_t players;

int8_t i;
int8_t j;
int8_t k;
int8_t l;

int moco; // revisar esto mas abajo
int8_t pix_ast;

int8_t masterR;

unsigned Joy_y_Analog;

//int8_t ship_points[74] = {0,1, 0,2, 0,3, 1,0, 1,1, 1,3, 1,4, 2,0, 2,1, 2,4, 2,5, 2,6, 3,1, 3,2, 3,6, 3,7, 4,1, 4,2, 4,3, 4,7, 4,8, 5,1, 5,2, 5,6, 5,7, 6,0, 6,1, 6,4, 6,5, 6,6, 7,0, 7,1, 7,3, 7,4, 8,1, 8,2, 8,3};
int8_t ship_points[38] = {0,0, 0,1, 0,2, 1,1, 1,2, 1,3, 1,4, 1,5, 2,2, 2,3, 2,6, 3,1, 3,2, 3,3, 3,4, 3,5, 4,0, 4,1, 4,2};

struct ship {
       int8_t x;
       int8_t y;
       int8_t pasy;
       int8_t activo; //Si es un jugador o lola pc
}ship[2];

struct asteroids {
       int8_t x;
       int8_t y;
       int8_t pasx;
       int8_t activo;
}asteroids_L[24], asteroids_R[24];

static void set_color(int r, int g, int b){
	SDL_SetRenderDrawColor(Renderer, r, g, b, 255);
}

static void plot_pixel(int x,int y){	
	SDL_RenderDrawPoint(Renderer,x,y);
}

static void Serial_activation(){

    struct timeval timeout;    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    puerto_serial = open("/dev/ttyUSB0", O_RDWR);  // /dev/ttyS0
    //puerto_serial = open("/dev/ttyS0", O_RDWR);  // /dev/ttyS0
    ndfs = puerto_serial + 1;
    
    //////preparing select()
    FD_ZERO(&all_set);
    FD_SET(puerto_serial, &all_set);
    r_set = all_set;
    tv.tv_sec = 1; 
    tv.tv_usec = 0;
    
    struct termios tty;

    // Read in existing settings, and handle any error
    if(tcgetattr(puerto_serial, &tty) != 0) 
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 1;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 20;

    cfsetispeed(&tty, B19200);
    cfsetospeed(&tty, B19200);

    if (tcsetattr(puerto_serial, TCSANOW, &tty) != 0) 
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }
}

static void Read_button(){
    button_up[maestro_esclavo] = 0;
    button_down[maestro_esclavo] = 0;
    button_enter = 0; 
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

            case SDL_QUIT:
                    ex = 10;
                    flag_reset = 1;
                    flag_point = 0;
            break;

            case SDL_MOUSEBUTTONUP:
                
            break;

            case SDL_KEYDOWN:{
                // keyboard API for key pressed
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        ex = 10;
                        flag_reset = 1;
                        flag_point = 0;
                    break;

                    case SDL_SCANCODE_R:
                        flag_reset = 1;
                        flag_point = 0;
                    break;
                    
                    case SDL_SCANCODE_P:
                        button_enter = 1; 
                    break;

                    case SDL_SCANCODE_RETURN:
                        button_enter = 1; 
                    break;

                    case SDL_SCANCODE_W:
                        button_up[maestro_esclavo] = 1;
                    break;  
                        
                    case SDL_SCANCODE_S:
                        button_down[maestro_esclavo] = 1;
                    break;

                    case SDL_SCANCODE_UP:
                        button_up[maestro_esclavo] = 1;
                    break;  
                        
                    case SDL_SCANCODE_DOWN:
                        button_down[maestro_esclavo] = 1;
                    break;

                    default:

                    break;
                }
            }
            break;

            default:
            
            break;
        }
    }
}

static void Read_Textura(int imagen){
    char bufer[10];
    int R;
    int G;
    int B;
    FILE *archivo;

    switch(imagen){
        case 0: archivo = fopen("Portada.ppm", "r"); // Modo lectura
        break;
        case 1: archivo = fopen("Player0.ppm", "r"); // Modo lectura
        break;
        case 2: archivo = fopen("player1.ppm", "r"); // Modo lectura
        break;
        case 3: archivo = fopen("player2.ppm", "r"); // Modo lectura
        break;
        case 4: archivo = fopen("You_Win.ppm", "r"); // Modo lectura
        break;
        case 5: archivo = fopen("Game_Over.ppm", "r"); // Modo lectura
        break;
        case 6: archivo = fopen("Waiting1.ppm", "r"); // Modo lectura
        break;
        case 7: archivo = fopen("Waiting2.ppm", "r"); // Modo lectura
        break;
        case 8: archivo = fopen("Detected0.ppm", "r"); // Modo lectura
        break;
        case 9: archivo = fopen("Detected1.ppm", "r"); // Modo lectura
        break;
        case 10: archivo = fopen("Detected2.ppm", "r"); // Modo lectura
        break;
        case 11: archivo = fopen("Detected3.ppm", "r"); // Modo lectura
        break;
        //default: archivo = fopen("Mustang.ppm", "r"); // Modo lectura
        //break;
    }
    fgets(bufer, 10, archivo);
    fgets(bufer, 10, archivo);
    fgets(bufer, 10, archivo);
    for (int fila = 0; fila < ymax ; ++fila){
        for (int columna = 0; columna < xmax; ++columna){
            fgets(bufer, 10, archivo);strtok(bufer, "\n");
            R = atoi(bufer);
            fgets(bufer, 10, archivo);strtok(bufer, "\n");
            G = atoi(bufer);
            fgets(bufer, 10, archivo);strtok(bufer, "\n");
            B = atoi(bufer);
            set_color(R,G,B);
            plot_pixel(columna,fila);
        }
    }
    SDL_RenderPresent(Renderer);
    fclose(archivo);
}

void init_screen1(){
    //cargar imagenes

    //Portada = SDL_LoadBMP("Portada.bmp");
    //BlueShapes = SDL_CreateTextureFromSurface(Renderer, Portada);
    //SDL_RenderCopy(Renderer, BlueShapes, NULL, NULL);
    //SDL_RenderPresent(Renderer);

    //Portada = SDL_LoadBMP("Portada.bmp");
    //SDL_BlitSurface(Portada, NULL, screen, NULL);
    //+SDL_UpdateWindowSurface(Window);
    
    //Portada = SDL_LoadBMP("Portada.bmp");
    //SDL_BlitScaled(Portada, NULL, screen, NULL);
    //BlueShapes = SDL_CreateTextureFromSurface(Renderer, screen);
    //SDL_Rect rec = {10, 10, 110, 110};
    //SDL_RenderCopy(Renderer, BlueShapes, NULL, &rec);
    //SDL_RenderPresent(Renderer);

    Read_Textura(0);
    while(ex != 10){
        Read_button();
        if(button_enter == 1) {
            set_color(0,0,0);
            SDL_RenderClear(Renderer);                                     // Clear GLCD
            break;
        }
    }
}

void init_screen2(){

    players = 0;
 
    Read_Textura(1);
    
    while(flag_reset != 1){
        Read_button();

        if (button_up[maestro_esclavo] == 1){
            players = 0;
        }
        else if (button_down[maestro_esclavo] == 1){
            players = 1;
        }
        if (button_enter == 1){
            if(players == 0){
                maestro_esclavo = 0;
            }
            break;
        }
        if (players == 1){
            Read_Textura(3);
        }else {
            Read_Textura(2);
        }
            
        SDL_Delay(500);
        Read_Textura(1);
        SDL_Delay(500);
    }
}

void init_screen3(){
    if (players != 0){
        int counter_p = 0;
        masterR = rand()%11+20;
        while(flag_reset != 1){
            Read_button();
            if (counter_p == 160){
                counter_p = 0;
                Read_Textura(6);
            }else if (counter_p == 80){
                Read_Textura(7);
            }
            counter_p++;
            if(maestro_esclavo == 0){
                buffer=0xC1;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer=0xC6;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer=masterR;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                if(FD_ISSET(puerto_serial, &r_set)){
                    read(puerto_serial, &buffer, sizeof(uint8_t));
                    if(buffer == 0xC2){
                        buffer=0xC3;
                        SDL_Delay(100);
                        write(puerto_serial, &buffer, sizeof(uint8_t));
                        break;
                    }else if(buffer == 0xC6){
                        if(FD_ISSET(puerto_serial, &r_set)){
                            read(puerto_serial, &buffer, sizeof(uint8_t));
                            if(masterR>(int)buffer){
                                maestro_esclavo = 1;
                            }else if(masterR==(int)buffer){
                                masterR = rand()%11+20;
                            }
                        }
                    }
                }
            }else if(FD_ISSET(puerto_serial, &r_set)){
                read(puerto_serial, &buffer, sizeof(uint8_t));
                if(buffer == 0xC1){
                    buffer=0xC2;
                    write(puerto_serial, &buffer, sizeof(uint8_t));
                }else if(buffer == 0xC3){
                    break;
                }
            }
        }
        if (ex == 10){
            return;
        }
        Read_Textura(8);
        SDL_Delay(500);
        Read_Textura(9);
        SDL_Delay(500);
        Read_Textura(10);
        SDL_Delay(500);
        Read_Textura(11);
        SDL_Delay(500);
    }
}

void draw_score(int score,int x,int y){
    char bufer[10];
    int R;
    int G;
    int B;
    FILE *archivo;
    archivo = fopen("Score.ppm", "r"); // Modo lectura
    fgets(bufer, 10, archivo);
    fgets(bufer, 10, archivo);
    fgets(bufer, 10, archivo);
    for (int i = 0; i < 15*3*score; ++i){
        fgets(bufer, 10, archivo);
    }
    for (int fila = 0; fila < 5 ; ++fila){
        for (int columna = 0; columna < 3; ++columna){
            fgets(bufer, 10, archivo);strtok(bufer, "\n");
            R = atoi(bufer);
            fgets(bufer, 10, archivo);strtok(bufer, "\n");
            G = atoi(bufer);
            fgets(bufer, 10, archivo);strtok(bufer, "\n");
            B = atoi(bufer);
            set_color(R,G,B);
            plot_pixel(x+columna,y+fila);
        }
    }
    fclose(archivo);
}   

void ScoreS(){
    draw_score(Score[0]%10,20,59);
    draw_score(Score[1]%10,108,59);

    draw_score(Score[0]/10,16,59);
    draw_score(Score[1]/10,104,59);
}

int8_t check_ast(int8_t posx, int8_t posy, int8_t u){
    if(u>0){
        for(i=0; i<24;i++){
            if((asteroids_L[i].x > posx - 7) && (asteroids_L[i].x < posx + 7) && (asteroids_L[i].y > posy - 7) && (asteroids_L[i].y <= posy)){
                return 1;
            }
        }
    }else{
        for(i=0; i<24;i++){
            if((asteroids_R[i].x > posx - 7) && (asteroids_R[i].x < posx + 7) && (asteroids_R[i].y > posy - 7) && (asteroids_R[i].y <= posy)){
                return 1;
            }
        }
    }
    return 0;
}

void IA_player2(){
    if(count_delay_asteroids >= delay_asteroids){
        if(ship[1].y >= ymin){
             if((check_ast(ship[1].x-5, ship[1].y, 1) == 1)||(check_ast(ship[1].x+10, ship[1].y,-1) == 1)){   //defenza ladoz
                if((check_ast(ship[1].x-2, ship[1].y-5,1) == 1)||(check_ast(ship[1].x+7, ship[1].y-5,-1) == 1)){   //hacia adelante
                    ship[1].y += step_ship;
                }else{
                    ship[1].y -= step_ship;
                }
            }else if((check_ast(ship[1].x-2, ship[1].y-5,1) == 1)||(check_ast(ship[1].x+7, ship[1].y-5,-1) == 1)){   //hacia adelante
            }else{   // si obstaculos
                ship[1].y -= step_ship;
            }
        }else{
            ship[1].y = ymax + long_ship - 4;
            Score[1]++;
        }
    }
}

void move_ship(){
   	Read_button();
    ship[0].pasy = ship[0].y;
    ship[1].pasy = ship[1].y;
    if(players != 0){
        if(button_up[maestro_esclavo] == 1){ // move up ship 1
            if(ship[maestro_esclavo].y - step_ship >= ymin){
                ship[maestro_esclavo].y -= step_ship;
                buffer=0xAA;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer = ship[maestro_esclavo].y;
                write(puerto_serial, &buffer, sizeof(uint8_t));
            }else{
                ship[maestro_esclavo].y = ymax + long_ship - 4;
                Score[maestro_esclavo]++;

                buffer=0xAB;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer = Score[maestro_esclavo];
                write(puerto_serial, &buffer, sizeof(uint8_t));

            }
        }
         //

        if(button_down[maestro_esclavo] == 1){ // move down ship 1
            if(ship[maestro_esclavo].y + step_ship < ymax){
                ship[maestro_esclavo].y += step_ship;

                buffer=0xAA;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                
                buffer = ship[maestro_esclavo].y;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                
                buffer=0xAA;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                
                buffer = ship[maestro_esclavo].y;
                write(puerto_serial, &buffer, sizeof(uint8_t));
            }
        }
    }else{ // one player
        maestro_esclavo = 0; 
        if(button_up[maestro_esclavo] == 1){ // move up ship 1
            if(ship[maestro_esclavo].y >= ymin){
                ship[maestro_esclavo].y -= step_ship;
            }
            else{
                ship[maestro_esclavo].y = ymax + long_ship - 4;
                Score[maestro_esclavo]++;
                //draw_score();
           	}
        }
        if(button_down[maestro_esclavo] == 1){ // move down ship 1
            if(ship[maestro_esclavo].y + step_ship < ymax){
                ship[maestro_esclavo].y += step_ship;
            }
        }
        IA_player2();
    }
}

void draw_ship(){
    for(i = 0; i<2; i++){
        //if (ship[i].pasy != ship[i].y){
            for (j = 0; j< 38; j+=2){
                if ((ship[i].y - ship_points[j+1] > ymin) && (ship[i].y - ship_points[j+1] < ymax)){
                    plot_pixel(ship[i].x - ship_points[j], ship[i].pasy - ship_points[j+1]);
                }
            }
        //}
    }
}

void collision_ShipL(){
    if (count_delay_asteroids >= delay_asteroids){
        for (i=0; i<24; i++){
            for(j=0; j<2; j++){
                if((asteroids_L[i].y <= ship[j].y) && (asteroids_L[i].y >= ship[j].y - 7)){
                    for(k=0; k<len_asteroid; k++){
                        for(l=0; l<38; l+=2){
                            if ((asteroids_L[i].x - k + step_asteroid == ship[j].x + ship_points[l]) && ((asteroids_L[i].y == ship[j].y - ship_points[l+1]))){
                                if(j==0){
                                   ship[j].x = 36;
                                   ship[j].y = 60;
                                }
                                else{
                                   ship[j].x = 92;
                                   ship[j].y = 60;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void collision_ShipR(){
    if (count_delay_asteroids >= delay_asteroids){
        for (i=0; i<24; i++){
            for(j=0; j<2; j++){
                if((asteroids_R[i].y <= ship[j].y) && (asteroids_R[i].y >= ship[j].y - 7)){
                    for(k=0; k<len_asteroid; k++){
                        for(l=0; l<38; l+=2){
                            if ((asteroids_R[i].x + k + step_asteroid== ship[j].x + ship_points[l]) && ((asteroids_R[i].y == ship[j].y - ship_points[l+1]))){
                                if(j==0){
                                   ship[j].x = 36;
                                   ship[j].y = 60;
                                }
                                else{
                                   ship[j].x = 92;
                                   ship[j].y = 60;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void init_ship(){
    ship[0].x = 36;
    ship[0].y = 60;

    ship[1].x = 92;
    ship[1].y = 60;


    ship[0].pasy = ship[0].y-1;
    ship[1].pasy = ship[1].y-1;
    
    draw_ship();   
}

void net_refresh(){
    set_color(0,0,250);
    if ((counter_delay_net_refresh >= delay_net_refresh)&&(maestro_esclavo==0)){
        counter_delay_net_refresh = 0;
        if(net_counter >= net_delay_counter){
            net_counter = 0;
            long_net +=2;
            buffer=0xE0;
            write(puerto_serial, &buffer, sizeof(uint8_t));
            buffer=long_net;
            write(puerto_serial, &buffer, sizeof(uint8_t));
            printf("%i\n",long_net );
            
        }
        net_counter ++;
    }
    SDL_RenderDrawLine(Renderer,63, long_net, 63, 63);
    if(long_net >= 64) {flag_point = 1; flag_reset = 0;}
    counter_delay_net_refresh ++;
}

void init_map(){
    Score[0] = 0;
    Score[1] = 0;
    long_net = 0;
    net_refresh();
    flag_reset = 0;
    flag_point = 0;

    init_ship();
    draw_ship();
}

void coneccion(){
    if(FD_ISSET(puerto_serial, &r_set)){
        read(puerto_serial, &buffer, sizeof(char));
        if(buffer == 0xAA){
            if(maestro_esclavo==0){
                if(FD_ISSET(puerto_serial, &r_set)){
                    read(puerto_serial, &buffer, sizeof(char));
                    
                    ship[1].y = (int)buffer;
                }
            }else if(maestro_esclavo==1){
                if(FD_ISSET(puerto_serial, &r_set)){
                    read(puerto_serial, &buffer, sizeof(char));
                    
                    ship[0].y = (int)buffer;
                }
            }
        }else if(buffer == 0xAB){
            if(maestro_esclavo==0){
                if(FD_ISSET(puerto_serial, &r_set)){
                    read(puerto_serial, &buffer, sizeof(char));
                    Score[1] = (int)buffer;
                }
            }else if(maestro_esclavo==1){
                if(FD_ISSET(puerto_serial, &r_set)){
                    read(puerto_serial, &buffer, sizeof(char));
                    Score[0] = (int)buffer;
                }
            }
        }else if(buffer == 0xB0){
            musgo ++;
        }else if(buffer == 0xB1){
            read(puerto_serial, &buffer, sizeof(char));
            i = buffer;
            read(puerto_serial, &buffer, sizeof(char));
            asteroids_L[i].x = buffer;
            read(puerto_serial, &buffer, sizeof(char));
            asteroids_L[i].y = buffer;
        }else if(buffer == 0xB2){
            read(puerto_serial, &buffer, sizeof(char));
            i = buffer;
            read(puerto_serial, &buffer, sizeof(char));
            asteroids_R[i].x=buffer;
            read(puerto_serial, &buffer, sizeof(char));
            asteroids_R[i].y=buffer;
        }else if(buffer == 0xE0){
            read(puerto_serial, &buffer, sizeof(char));
            long_net = buffer;
        }
    }
}

void init_asteroids(){
    counter_ast = 0;
    count_active_ast = 0;
    if(maestro_esclavo == 0){
        for(i=0; i< 24; i++){
            asteroids_L[i].x = rand()%128;
            asteroids_R[i].x = rand()%128;
            asteroids_L[i].pasx = asteroids_L[i].x;
            asteroids_R[i].pasx = asteroids_R[i].x;
            asteroids_L[i].y = rand()% 50;
            asteroids_R[i].y = rand()% 50;

        }
        buffer=0xDA;
        write(puerto_serial, &buffer, sizeof(uint8_t));
        for(i=0; i< 24; i++){
            buffer=asteroids_L[i].x;
            write(puerto_serial, &buffer, sizeof(uint8_t));
            buffer=asteroids_R[i].x;
            write(puerto_serial, &buffer, sizeof(uint8_t));
            buffer=asteroids_L[i].pasx;
            write(puerto_serial, &buffer, sizeof(uint8_t));
            buffer=asteroids_R[i].pasx;
            write(puerto_serial, &buffer, sizeof(uint8_t));
            buffer=asteroids_L[i].y;
            write(puerto_serial, &buffer, sizeof(uint8_t));
            buffer=asteroids_R[i].y;
            write(puerto_serial, &buffer, sizeof(uint8_t));
        }
    }else if(maestro_esclavo == 1){
        read(puerto_serial, &buffer, sizeof(char));
        if(buffer == 0xDA){
            for(i=0; i< 24; i++){
                read(puerto_serial, &buffer, sizeof(char));
                asteroids_L[i].pasx = buffer;
                read(puerto_serial, &buffer, sizeof(char));
                asteroids_R[i].pasx = buffer;
                read(puerto_serial, &buffer, sizeof(char));
                asteroids_L[i].x = buffer;
                read(puerto_serial, &buffer, sizeof(char));
                asteroids_R[i].x = buffer;
                read(puerto_serial, &buffer, sizeof(char));
                asteroids_L[i].y = buffer;
                read(puerto_serial, &buffer, sizeof(char));
                asteroids_R[i].y = buffer;
            }
        }
    }
}

void draw_asteroids(){
    set_color(250,250,100);
    if (count_delay_asteroids >= delay_asteroids){
        count_delay_asteroids = 0;
        for(i=0; i< 24; i++){
            for(j=0; j< len_asteroid; j++){
                if((asteroids_L[i].x - j >= xmin) && (asteroids_L[i].x - j < xmax)){
                    plot_pixel(asteroids_L[i].x - j, asteroids_L[i].y);
                }
            }
            for(j=0; j< len_asteroid; j++){
                if((asteroids_R[i].x + j >= xmin) && (asteroids_R[i].x + j < xmax)){
                    plot_pixel(asteroids_R[i].x + j, asteroids_R[i].y);
                }
            }
        }
    }
    count_delay_asteroids++;
}

void move_asteroids(){
    if ((maestro_esclavo == 0)&&(count_delay_asteroids >= delay_asteroids)){
        buffer=0xB0;
        write(puerto_serial, &buffer, sizeof(uint8_t));
        for(i=0; i< 24; i++){
            asteroids_L[i].pasx = asteroids_L[i].x;
            asteroids_R[i].pasx = asteroids_R[i].x;
            asteroids_L[i].x += step_asteroid;
            asteroids_R[i].x -= step_asteroid;
            if (asteroids_L[i].x > xmax + len_asteroid){
                asteroids_L[i].x = xmin;
                asteroids_L[i].y = rand()%50;
                buffer=0xB1;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer=i;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer=asteroids_L[i].x;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer=asteroids_L[i].y;
                write(puerto_serial, &buffer, sizeof(uint8_t));
            }
            if(asteroids_R[i].x < xmin - len_asteroid){
                asteroids_R[i].x = xmax;
                asteroids_R[i].y = rand()%50;
                buffer=0xB2;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer=i;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer=asteroids_R[i].x;
                write(puerto_serial, &buffer, sizeof(uint8_t));
                buffer=asteroids_R[i].y;
                write(puerto_serial, &buffer, sizeof(uint8_t));
            }
        }
        count_active_ast++;
    }else if ((maestro_esclavo == 1)&&(musgo != 0)){
        musgo --;
        for(i=0; i< 24; i++){
            asteroids_L[i].pasx = asteroids_L[i].x;
            asteroids_R[i].pasx = asteroids_R[i].x;
            asteroids_L[i].x += step_asteroid;
            asteroids_R[i].x -= step_asteroid;
        }
    }
}

int main(int argc, char *argv[]) {
	//SDL Window setup
    Serial_activation();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf ("Error al inicializar SDL%s", SDL_GetError ());
        return -1;
    }
	
	SDL_CreateWindowAndRenderer(SCREEN_WIDTH*5, SCREEN_HEIGHT*5,  0, &Window, &Renderer); // SDL_WINDOW_FULLSCREEN_DESKTOP
	
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(Renderer, SCREEN_WIDTH, SCREEN_HEIGHT); //Resolución a trabajar, el SDL lo escala a pantalla completa


	if (Window == NULL) {
		printf("No se puede inicializar el modo gráfico: %s \n",SDL_GetError());
		exit(1);
	}

    while(ex != 10){
        
        init_screen1();
        init_screen2();
        init_screen3();
        init_asteroids();
        init_map();

    	//Gameloop
    	while((flag_reset == 0)&&(ex != 10)) {
            r_set = all_set;
            tv.tv_usec = 100000;
            select(ndfs, &r_set, NULL, NULL, &tv);


            //check for new events every frame
    		move_ship();
            move_asteroids();
            collision_ShipL();
            collision_ShipR();
            coneccion();
            coneccion();
            net_refresh();
            draw_ship();
            draw_asteroids();


            //Score
            ScoreS();

            //reset
            while(flag_point==1){
                Read_button();
                if (button_enter==1){
                    break;   
                }
                if(maestro_esclavo == 0){ moco = 1;}
                else moco = 0;
                if(Score[maestro_esclavo] > Score[moco]){
                    Read_Textura(4);
                }else{
                    Read_Textura(5);
                }
            }

            //Rendering
        	SDL_RenderPresent(Renderer);
        	set_color(0,0,0);
        	SDL_RenderClear(Renderer);
            set_color(0,0,250);	
    	}
    }

    
    //free loaded images

	SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);
    SDL_Quit();
	return 0;
}