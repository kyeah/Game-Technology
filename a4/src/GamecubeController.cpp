#include "GamecubeController.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

GamecubeController::GamecubeController(const char* name){
    setAllToFalse();

   
    JOYSTICK_X = 0;
    JOYSTICK_Y = 0;
    CSTICK_X = 0;
    CSTICK_Y = 0;

    connected = false;   
    prev_joystick_x = -999;
    prev_joystick_y = -999;
    prev_cstick_x = -999;
    prev_cstick_y = -999;
 

    for(int i = 0; i < 8; i++){
        prev_state[i] = false;
    }
    
    JOYSTICK_X_POS = false;
    JOYSTICK_X_NEG = false;
    JOYSTICK_Y_POS = false;
    JOYSTICK_Y_NEG = false;
    base[0] = 0x34;
    base[1] = 0xffffffc0;
    base[2] = 0x66;
    base[3] = 0x1;
    base[4] = 0x0;
    base[5] = 0x0;
    base[6] = 0x2;
    base[7] = 0x1;
    
    states[0] = new State((long*)&data[7], 0x9);  //start 
    states[1] = new State((long*)&data[7], 0x1);//y
    states[2] = new State((long*)&data[7], 0x0);//x
    states[3] = new State((long*)&data[7], 0x3);//b
    states[4] = new State((long*)&data[7], 0x2);//a
    states[5] = new State((long*)&data[7], 0x4);//l
    states[6] = new State((long*)&data[7], 0x5);//r
    states[7] = new State((long*)&data[7], 0x6);//z
    nameOfDevice = name;
}

GamecubeController::~GamecubeController(){
    gc_close();
}

void GamecubeController::init(){
    if((device = open(nameOfDevice, O_RDONLY|O_NONBLOCK)) < 0)
        perror("Failed to open the device: ");  
    else
	connected = true;
}

bool* GamecubeController::getItem(int i){
    switch(i){
        case 0: return &START_PRESSED;
        case 1: return &Y_PRESSED;
        case 2: return &X_PRESSED;
        case 3: return &B_PRESSED;
        case 4: return &A_PRESSED;
        case 5: return &L_PRESSED;
        case 6: return &R_PRESSED;
        case 7: return &Z_PRESSED;
        default: break;
    }
    return false;
}

void GamecubeController::reset(){

    gc_close();
    for(int i = 0; i < 8; i++){
        prev_state[i] = false;
    }
   
    connected = false;
    setAllToFalse();

    JOYSTICK_X = 0; 
    JOYSTICK_Y = 0; 
    CSTICK_X = 0; 
    CSTICK_Y = 0; 
    init();
}

void GamecubeController::setAllToFalse(){


    A_PRESSED = false;
    B_PRESSED = false;
    X_PRESSED = false;
    Y_PRESSED = false;
    START_PRESSED = false;
    Z_PRESSED = false;
    L_PRESSED = false;
    R_PRESSED = false;
    D_UP = false;
    D_UP_D_RIGHT = false;
    D_RIGHT = false;
    D_RIGHT_D_DOWN = false;
    D_DOWN = false;
    D_DOWN_D_LEFT = false;
    D_LEFT = false;
    D_LEFT_D_UP = false; 
 
}

int GamecubeController::blockingCapture(){
    bool waiting = true;
    int retval = -1;
    while(waiting){
        capture();
        for(int i = 0; i < 8; i++){
            if(*getItem(i)){
                retval = i; 
                waiting = false;
            }
        }   
    }
    return retval;
}

void GamecubeController::capture(){
    setAllToFalse();
    if(connected)
    	read(device, data, sizeof(data));
    else
	return;

    if((data[0] == base[0]) && (data[1] == base[1]) && (data[2] == base[2]) && (data[3] == base[3]) 
	&& (data[4] == base[4]) && (data[5] == base[5]) && (data[6] == base[6]) && data[7] == base[7])
	return;

    for(int i = 0; i < 8; i++){
        State* temp = states[i];
        if(((*(temp->current_state) & 0xff) == (temp->expected_state&0xff)) && (data[4] == 0x1) && (data[6] == 0x1)){
	   prev_state[i] = true;
	}
        else {
            if(prev_state[i]){
                prev_state[i] = false;
                bool* var = getItem(i);
                *var = true;
            }
        }
    }

  if((prev_joystick_x != data[5]) && (data[7] == 0x0) && (data[6] == 0x2)){
	prev_joystick_x = data[5];
        JOYSTICK_X = data[5];
    }

    if((prev_joystick_y != data[5]) && (data[7] == 0x1) && (data[6] == 0x2)){
	prev_joystick_y = data[5];
        JOYSTICK_Y = -data[5];
    } 

    if((prev_cstick_x != data[5]) && (data[7] == 0x3)  && (data[6] == 0x2)){
        prev_cstick_x = data[5];
        CSTICK_X = data[5];
    }  
    
    if((prev_cstick_y != data[5]) && (data[7] == 0x4) && (data[6] == 0x2)){
        prev_cstick_y = data[5];
        CSTICK_Y = data[5];
    }  
}

void GamecubeController::gc_close(){
    close(device);
    connected = false;
}
