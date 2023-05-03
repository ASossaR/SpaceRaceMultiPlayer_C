#include <stdint.h>

int8_t maestro_esclavo = 0;
int8_t musgo =0;

//Module Connections
char GLCD_DataPort at PORTD;
sbit GLCD_CS1 at LATB0_bit;
sbit GLCD_CS2 at LATB1_bit;
sbit GLCD_RS  at LATB2_bit;
sbit GLCD_RW  at LATB3_bit;
sbit GLCD_EN  at LATB4_bit;
sbit GLCD_RST at LATB5_bit;

sbit GLCD_CS1_Direction at TRISB0_bit;
sbit GLCD_CS2_Direction at TRISB1_bit;
sbit GLCD_RS_Direction  at TRISB2_bit;
sbit GLCD_RW_Direction  at TRISB3_bit;
sbit GLCD_EN_Direction  at TRISB4_bit;
sbit GLCD_RST_Direction at TRISB5_bit;

bit oldstate_enter;                                    // Old state flag
int8_t oldstate_down[2];                                    // Old state flag
int8_t oldstate_up[2];

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

int delay_asteroids = 40;
int count_delay_asteroids;
int8_t count_active_ast = 0;
int8_t step_asteroid = 3;
int8_t counter_ast = 0;
int8_t len_asteroid = 3;

int8_t delay_net_refresh = 10;
int8_t counter_delay_net_refresh;

int8_t net_counter;
int8_t net_delay_counter = 10;
int8_t long_net = 0;

int warning_collision;

bit flag_reset;
bit flag_point;
bit players;

int8_t i;
int8_t j;
int8_t k;
int8_t l;

int moco; // revisar esto mas abajo
int8_t pix_ast;

char buffer;

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

//void init_asteroids();

void read_button_enter(){
    if (!oldstate_enter && Button(&PORTC, 0, 1, 0)) { // Detect logical one
        oldstate_enter = 0;
        button_enter = 0;                              // Update flag
    }
    else if (oldstate_enter && Button(&PORTC, 0, 1, 1)) {   // Detect one-to-zero transition
        button_enter = 0;                            // Invert PORTC
        oldstate_enter = 1;                              // Update flag
    }
    else if (!oldstate_enter && Button(&PORTC, 0, 1, 1)) {   // Detect one-to-zero transition
        button_enter = 0;                            // Invert PORTC
        oldstate_enter = 1;                              // Update flag
    }
    else if (oldstate_enter && Button(&PORTC, 0, 1, 0)) {   // Detect one-to-zero transition
        button_enter = 1;                            // Invert PORTC
        oldstate_enter = 0;                              // Update flag
    }
}

void read_joystick(){
    Joy_y_Analog = ADC_Read(0);

    if(Joy_y_Analog <400){
        button_up[maestro_esclavo] = 1;
    }else if(Joy_y_Analog < 600){
        button_down[maestro_esclavo] = 1;
    }else {button_up[maestro_esclavo] = 0; button_down[maestro_esclavo] = 0; }
}

void read_button_up(){

    if (!oldstate_up[maestro_esclavo] && Button(&PORTA, 2, 1, 0)) { // Detect logical one
        oldstate_up[maestro_esclavo] = 0;
        button_up[maestro_esclavo] = 0;                              // Update flag
    }
    else if (oldstate_up[maestro_esclavo] && Button(&PORTA, 2, 1, 1)) {   // Detect one-to-zero transition
        button_up[maestro_esclavo] = 0;                            // Invert PORTC
        oldstate_up[maestro_esclavo] = 1;                              // Update flag
    }
    else if (!oldstate_up[maestro_esclavo] && Button(&PORTA, 2, 1, 1)) {   // Detect one-to-zero transition
        button_up[maestro_esclavo] = 0;                            // Invert PORTC
        oldstate_up[maestro_esclavo] = 1;                              // Update flag
    }
    else if (oldstate_up[maestro_esclavo] && Button(&PORTA, 2, 1, 0)) {   // Detect one-to-zero transition
        button_up[maestro_esclavo] = 1;                            // Invert PORTC
        oldstate_up[maestro_esclavo] = 0;                              // Update flag
    }

}

void read_button_down(){
    if (!oldstate_down[maestro_esclavo] && Button(&PORTA, 0, 1, 0)) { // Detect logical one
        oldstate_down[maestro_esclavo] = 0;
        button_down[maestro_esclavo] = 0;                              // Update flag
    }
    if (oldstate_down[maestro_esclavo] && Button(&PORTA, 0, 1, 1)) {   // Detect one-to-zero transition
        button_down[maestro_esclavo] = 0;                            // Invert PORTC
        oldstate_down[maestro_esclavo] = 1;                              // Update flag
    }
    if (!oldstate_down[maestro_esclavo] && Button(&PORTA, 0, 1, 1)) {   // Detect one-to-zero transition
        button_down[maestro_esclavo] = 0;                            // Invert PORTC
        oldstate_down[maestro_esclavo] = 1;                              // Update flag
    }
    if (oldstate_down[maestro_esclavo] && Button(&PORTA, 0, 1, 0)) {   // Detect one-to-zero transition
        button_down[maestro_esclavo] = 1;                            // Invert PORTC
        oldstate_down[maestro_esclavo] = 0;                              // Update flag
    }

}

void draw_ship(){
    for(i = 0; i<2; i++){
        if (ship[i].pasy != ship[i].y){
            for (j = 0; j< 38; j+=2){
                if ((ship[i].pasy - ship_points[j+1] > ymin) && (ship[i].pasy - ship_points[j+1] < ymax)){
                    Glcd_Dot(ship[i].x - ship_points[j], ship[i].pasy - ship_points[j+1], 0);
                }
            }
            for (j = 0; j< 38; j+=2){
                if ((ship[i].y - ship_points[j+1] > ymin) && (ship[i].y - ship_points[j+1] < ymax)){
                    Glcd_Dot(ship[i].x - ship_points[j], ship[i].y - ship_points[j+1], 1);
                }
            }
        }
    }
}

void init_screen(){
    Glcd_Rectangle(0,0,127,63,1);
    Glcd_Set_Font(&System3x5, 3, 5, 32);
    Glcd_Write_Text("INST TECHNOLOGIC COSTA RICA", 4, 0, 1);
    Glcd_Write_Text("    EMBEDDED SYSTEM DESIGN    ", 4, 1, 1);
    Glcd_Write_Text("      GAME: SPACE RACE        ", 4, 2, 1);
    Glcd_Write_Text("  TEACH: ING. RIVERA ALVARADO ", 4, 3, 1);
    Glcd_Write_Text("STUD: ALVARO SOSSA ROJAS      ", 4, 4, 1);
    Glcd_Write_Text("STUD: JEISSON GONZALEZ LEDEZMA", 4, 5, 1);
    Glcd_Write_Text("STUD: GABRIEL QUIROZ GONZALEZ ", 4, 6, 1);
    Glcd_Write_Text("        I-SEMESTER-2022       ", 4, 7, 1);

    //Delay_ms(1000);

    while(1){
        read_button_enter();
        if(button_enter == 1) {
            Glcd_Fill(0x00);                                     // Clear GLCD
            break;
        }
     }

}

void init_screen2(){

    players = 0;
    Glcd_Rectangle(0,0,127,63,1);

    Glcd_Write_Text("       GAME: SPACE RACE       ", 4, 1, 1);
    Glcd_Write_Text("                              ", 4, 2, 1);
    Glcd_Write_Text("SELECT MODE:                  ", 4, 3, 1);
    Glcd_Write_Text("                    1 PLAYER  ", 4, 4, 1);
    Glcd_Write_Text("                    2 PLAYERS ", 4, 5, 1);
    Glcd_Write_Text("__PRESS ENTER TO SELECT MODE__", 4, 6, 1);


    while(1){
        //read_joystick();
        read_button_enter();
        read_button_up();
        read_button_down();

        if (button_up[maestro_esclavo] == 1){
            players = 0;
        }
        else if (button_down[maestro_esclavo] == 1){
            players = 1;
        }
        if (button_enter == 1){
            if(players == 0){maestro_esclavo = 0;}
            break;
        }

        if (players == 1){
            Glcd_Write_Text("                    1 PLAYER  ", 4, 4, 1);
            Glcd_Write_Text("                    2 PLAYERS ", 4, 5, 0);
        }else {
            Glcd_Write_Text("                    1 PLAYER  ", 4, 4, 0);
            Glcd_Write_Text("                    2 PLAYERS ", 4, 5, 1);
        }

    }
}

void init_screen3(){
    if (players != 0){
        int counter_p = 0;
        Glcd_Fill(0x00);
        Glcd_Rectangle(0,0,127,63,1);
        Glcd_Set_Font(&System3x5, 3, 5, 32);
        Glcd_Write_Text(" GAME: SPACE RACE MULTIPLAYER ", 4, 1, 1);
        Glcd_Write_Text(" WAITING F0R CONNECTION...... ", 4, 4, 1);
         masterR = rand()%11;
         while(1){
              if (counter_p == 160){
                  counter_p = 0;
                  Glcd_Write_Text(" WAITING F0R CONNECTION...... ", 4, 4, 1);
              }else if (counter_p == 80){
                  Glcd_Write_Text(" WAITING F0R CONNECTION...... ", 4, 4, 0);
              }
              counter_p++;

              if(maestro_esclavo == 0){
                  if(UART_Data_Ready()){
                      buffer = UART_Read();
                      if(buffer == 0xC2){
                          UART1_Write(0xC3);
                          Glcd_Fill(0x00);                                     // Clear GLCD
                          break;
                      }else if(buffer == 0xC6){
                          buffer = UART_Read();
                          if(masterR>(int)buffer){
                              maestro_esclavo = 1;
                          }else if(masterR==(int)buffer){
                              masterR = rand()%11;
                          }
                      }
                  }else if(UART_Tx_Idle() == 1){
                      UART1_Write(0xC1);
                      UART1_Write(0xC6);
                      UART1_Write(masterR);
                  }
              }else if(UART_Data_Ready()){
                  buffer = UART_Read();
                  if(buffer == 0xC1){
                      UART1_Write(0xC2);
                  }else if(buffer == 0xC3){
                      Glcd_Fill(0x00);                                     // Clear GLCD
                      break;
                  }
              }
         }
         Glcd_Fill(0x00);

         Glcd_Write_Text(" GAME: SPACE RACE MULTIPLAYER ", 4, 1, 1);
         Glcd_Write_Text("     DETECTED CONNECTION.     ", 4, 4, 1);
         Glcd_Write_Text("  START THE GAME IN:          ", 4, 6, 1);
         Delay_ms(1000);
         Glcd_Write_Text("  START THE GAME IN: 3..      ", 4, 6, 1);
         Delay_ms(1000);
         Glcd_Write_Text("  START THE GAME IN: 3..2..   ", 4, 6, 1);
         Delay_ms(1000);
         Glcd_Write_Text("  START THE GAME IN: 3..2..1  ", 4, 6, 1);
         Delay_ms(1000);
         Glcd_Fill(0x00);

    }

}

void draw_score(){
    char char_temp_unid[2];
    char char_temp_dec[2];

    Glcd_Set_Font(&System3x5, 3, 5, 32);

    char_temp_unid[0] = Score[0]/10 + '0';
    char_temp_dec[0]  = Score[0]%10 + '0';

    char_temp_unid[1] = Score[1]/10 + '0';
    char_temp_dec[1]  = Score[1]%10 + '0';

    Glcd_Write_Char(" ", 4, 7, 0);
    Glcd_Write_Char(char_temp_unid[0], 4, 7, 1);
    Glcd_Write_Char(" ", 8, 7, 0);
    Glcd_Write_Char(char_temp_dec[0], 8, 7, 1);

    Glcd_Write_Char(" ", 118, 7, 0);
    Glcd_Write_Char(char_temp_unid[1], 118, 7, 1);
    Glcd_Write_Char(" ", 122, 7, 0);
    Glcd_Write_Char(char_temp_dec[1], 122, 7, 1);
}

void game_over(){
    Glcd_Fill(0x00);                                     // Clear GLCD
    Glcd_Rectangle(0,0,127,63,1);
    Glcd_Set_Font(&System3x5, 3, 5, 32);
    Glcd_Write_Text("                              ", 4, 0, 1);
    Glcd_Write_Text("                              ", 4, 1, 1);
    Glcd_Write_Text("           GAME OVER          ", 4, 2, 1);
    Glcd_Write_Text("           GAME OVER          ", 4, 3, 1);
    Glcd_Write_Text("           GAME OVER          ", 4, 4, 1);
    Glcd_Write_Text("                              ", 4, 5, 1);
    Glcd_Write_Text("                              ", 4, 7, 1);

    while(1){
        read_button_enter(); // Arreglarlo para maestro_esclavo y la comunicación
        if(button_enter == 1) {
            Glcd_Fill(0x00);                                     // Clear GLCD
            break;
        }
    }
    flag_reset = 1;
}

void you_win(){
    Glcd_Fill(0x00);                                     // Clear GLCD
    Glcd_Set_Font(&System3x5, 3, 5, 32);
    Glcd_Write_Text("                              ", 4, 0, 1);
    Glcd_Write_Text("                              ", 4, 1, 1);
    Glcd_Write_Text("           YOU WIN            ", 4, 2, 1);
    Glcd_Write_Text("           YOU WIN            ", 4, 3, 1);
    Glcd_Write_Text("           YOU WIN            ", 4, 4, 1);
    Glcd_Write_Text("                              ", 4, 5, 1);
    Glcd_Write_Text("                              ", 4, 7, 1);

    while(1){
        read_button_enter(); // Arreglarlo para maestro_esclavo y la comunicación
        if(button_enter == 1) {
            Glcd_Fill(0x00);                                     // Clear GLCD
            break;
        }
    }

    flag_reset = 1;
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

int8_t check_ast(int8_t posx, int8_t posy, int8_t u){
    if(u>0){
        for(i=0; i<24;i++){
            if((asteroids_L[i].x > posx - 7) && (asteroids_L[i].x < posx + 7) && (asteroids_L[i].y > posy - 7) && (asteroids_L[i].x <= posy)){
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
             draw_score();
        }
    }
}

void move_ship(){

    ship[0].pasy = ship[0].y;
    ship[1].pasy = ship[1].y;

    if(players != 0){
        //read_joystick();
        read_button_up();
        read_button_down();
        if(button_up[maestro_esclavo] == 1){ // move up ship 1
             if(ship[maestro_esclavo].y - step_ship  >= ymin){
                ship[maestro_esclavo].y -= step_ship;
                if(UART_Tx_Idle() == 1){
                    UART1_Write(0xAA);
                    UART1_Write(ship[maestro_esclavo].y);

                }
             }else{
                 ship[maestro_esclavo].y = ymax + long_ship - 4;
                 Score[maestro_esclavo]++;
                 draw_score();
                 if(UART_Tx_Idle() == 1){
                    UART1_Write(0xAB);
                    UART1_Write(Score[maestro_esclavo]);
                 }
             }
         }
         //
         if(button_down[maestro_esclavo] == 1){ // move down ship 1
             if(ship[maestro_esclavo].y + step_ship < ymax){
                ship[maestro_esclavo].y += step_ship;
                if(UART_Tx_Idle() == 1){
                    UART1_Write(0xAA);
                    UART1_Write(ship[maestro_esclavo].y);
                }
             }
         }
         
     }else{ // one player

          //read_joystick();
          read_button_up();
          read_button_down();

          if(button_up[maestro_esclavo] == 1){ // move up ship 1
             if(ship[maestro_esclavo].y >= ymin){
                 ship[maestro_esclavo].y -= step_ship;
             }
             else{
                 ship[maestro_esclavo].y = ymax + long_ship - 4;
                 Score[maestro_esclavo]++;
                 draw_score();
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

void net_refresh(){
    if (counter_delay_net_refresh >= delay_net_refresh && maestro_esclavo == 0){
        counter_delay_net_refresh = 0;

        Glcd_V_Line(long_net, 63, 63, 1);

        if(net_counter >= net_delay_counter){
            net_counter = 0;

            Glcd_V_Line(long_net, 63, 63, 0);
            long_net +=2;
            UART1_Write(0xE0);
            UART1_Write(long_net);
            Glcd_V_Line(long_net, 63, 63, 1);
        }
        net_counter ++;
    }
    if(long_net >= 64) {flag_point = 0; flag_reset = 0;}
    counter_delay_net_refresh ++;
}

void init_map(){
    Glcd_Fill(0x00);                                     // Clear GLCD
    Score[0] = 0;
    Score[1] = 0;
    long_net = 0;
    net_refresh();
    flag_reset = 1;
    flag_point = 1;
    init_ship();
    draw_ship();
    draw_score();
}

void coneccion(){

    if(UART_Data_Ready()){
        int8_t i;
        buffer = UART_Read();

        if(buffer == 0xAA){

            if(maestro_esclavo==0){

                if(UART_Data_Ready()){
                    buffer = UART_Read();
                    ship[1].y = (int)buffer;
                }

            }

            else if(maestro_esclavo==1){
                if(UART_Data_Ready()){
                    buffer = UART_Read();
                    ship[0].y = (int)buffer;
                }
            }
        }
        
        else if(buffer == 0xAB){

            if(maestro_esclavo==0){

                if(UART_Data_Ready()){
                    buffer = UART_Read();
                    Score[1] = (int)buffer;
                }

            }

            else if(maestro_esclavo==1){
                if(UART_Data_Ready()){
                    buffer = UART_Read();
                    Score[0] = (int)buffer;
                }
            }
            draw_score();
        }
        else if(buffer == 0xB0){
            musgo = 1;
        }
        else if(buffer == 0xB1){
            buffer = UART_Read();
            i = buffer;
            buffer = UART_Read();
            asteroids_L[i].x = buffer;
            buffer = UART_Read();
            asteroids_L[i].y = buffer;
        }
        else if(buffer == 0xB2){
            buffer = UART_Read();
            i = buffer;
            buffer = UART_Read();
            asteroids_R[i].x=buffer;
            buffer = UART_Read();
            asteroids_R[i].y=buffer;
        }
        else if(buffer == 0xE0){
            buffer = UART_Read();
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
            asteroids_L[i].pasx = asteroids_L[i].x-1;
            asteroids_R[i].pasx = asteroids_R[i].x-1;
            asteroids_L[i].y = rand()% 50;
            asteroids_R[i].y = rand()% 50;
        }
        UART1_Write(0xDA);
        
        for(i=0; i< 24; i++){
            UART1_Write(asteroids_L[i].x);
            UART1_Write(asteroids_R[i].x);
            UART1_Write(asteroids_L[i].pasx);
            UART1_Write(asteroids_R[i].pasx);
            UART1_Write(asteroids_L[i].y);
            UART1_Write(asteroids_R[i].y);
        }
        
    }else if(maestro_esclavo == 1){
        
        if(UART_Data_Ready()){
            buffer = UART_Read();
            if(buffer == 0XDA){
                for(i=0; i< 24; i++){
                    buffer = UART_Read();
                    asteroids_L[i].x = buffer;
                    buffer = UART_Read();
                    asteroids_R[i].x = buffer;
                    buffer = UART_Read();
                    asteroids_L[i].pasx = buffer;
                    buffer = UART_Read();
                    asteroids_R[i].pasx = buffer;
                    buffer = UART_Read();
                    asteroids_L[i].y = buffer;
                    buffer = UART_Read();
                    asteroids_R[i].y = buffer;
                }
            }
        }
    }
}

void draw_asteroids(){

    if (count_delay_asteroids >= delay_asteroids){
        count_delay_asteroids = 0;
        for(i=0; i< 24; i++){
            for(j=0; j< len_asteroid; j++){
                    Glcd_Dot(asteroids_L[i].pasx - j , asteroids_L[i].y, 0);
                    Glcd_Dot(asteroids_R[i].pasx + j , asteroids_R[i].y, 0);
            }
        }
        for(i=0; i< 24; i++){
            for(j=0; j< len_asteroid; j++){
                if((asteroids_L[i].x - j >= xmin) && (asteroids_L[i].x - j < xmax)){
                    Glcd_Dot(asteroids_L[i].x - j, asteroids_L[i].y, 1);
                }
            }
            for(j=0; j< len_asteroid; j++){
                if((asteroids_R[i].x + j >= xmin) && (asteroids_R[i].x + j < xmax)){
                    Glcd_Dot(asteroids_R[i].x + j, asteroids_R[i].y, 1);
                }
            }
        }
    }
    count_delay_asteroids++;
}

void move_asteroids(){
    if ((maestro_esclavo == 0)&&(count_delay_asteroids >= delay_asteroids)){
        UART1_Write(0xB0);
        for(i=0; i< 24; i++){
            asteroids_L[i].pasx = asteroids_L[i].x;
            asteroids_R[i].pasx = asteroids_R[i].x;
            asteroids_L[i].x += step_asteroid;
            asteroids_R[i].x -= step_asteroid;

            if(asteroids_L[i].x > xmax +3){
                asteroids_L[i].pasx = asteroids_L[i].x;
                asteroids_L[i].x = xmin;
                asteroids_L[i].y = rand()%50;
                
                if(UART_Tx_Idle() == 1){
                     UART1_Write(0xB1);
                     UART1_Write(i);
                     UART1_Write(asteroids_L[i].x);
                     UART1_Write(asteroids_L[i].y);
                }
            }
            if(asteroids_R[i].x < xmin-5){
                asteroids_R[i].pasx = asteroids_R[i].x;
                asteroids_R[i].x = xmax;
                asteroids_R[i].y = rand()%50;
                
                if(UART_Tx_Idle() == 1){
                    UART1_Write(0xB2);
                    UART1_Write(i);
                    UART1_Write(asteroids_R[i].x);
                    UART1_Write(asteroids_R[i].y);
                }
            }
        }
        count_active_ast++;
    }else if ((maestro_esclavo == 1)&&(musgo == 1)){
        musgo = 0;
        for(i=0; i< 24; i++){
            asteroids_L[i].pasx = asteroids_L[i].x;
            asteroids_R[i].pasx = asteroids_R[i].x;
            asteroids_L[i].x += step_asteroid;
            asteroids_R[i].x -= step_asteroid;
        }
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


void main() {

    UART1_init(19200);
    Delay_ms(100);

    //ADC_Init();

    ADCON1=0x0F;                              // set RB0 pin as input
    TRISA = 0XFF;
    TRISC = 0XFF;

    oldstate_enter = 0;

    Glcd_Init();


    flag_reset = 1;
    while(flag_reset){

        init_screen();
        init_screen2();
        init_screen3();
        init_asteroids();
        init_map();
        flag_point = 1;
        while(flag_point){
            move_ship();
            move_asteroids();
            net_refresh();
            collision_ShipL();
            collision_ShipR();
            coneccion();
            draw_ship();
            draw_asteroids();
        }

        if(maestro_esclavo == 0){ moco = 1;}
        else moco = 0;
        if(Score[maestro_esclavo] > Score[moco]){
            you_win();
        }else{
            game_over();
        }
    }
}