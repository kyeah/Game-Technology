#include <iostream>
#include <stdio.h>

class State{
public:
	State(long *cur, char expected) { current_state = cur; expected_state = expected; }
	long* current_state;
	char expected_state;
};

class GamecubeController{
public:
	bool START_PRESSED; 	// 0
	bool Y_PRESSED;		// 1
	bool X_PRESSED;		// 2
	bool B_PRESSED; 	// 3
	bool A_PRESSED;		// 4
	bool L_PRESSED;		// 5
	bool R_PRESSED;		// 6
	bool Z_PRESSED;		// 7
	bool D_UP;		// 8
	bool D_UP_D_RIGHT;	// 9 
	bool D_RIGHT;		// 10
	bool D_RIGHT_D_DOWN;	// 11
	bool D_DOWN;		// 12
	bool D_DOWN_D_LEFT;	// 13
	bool D_LEFT;		// 14
	bool D_LEFT_D_UP;	// 15	
	bool JOYSTICK_X_POS;
	bool JOYSTICK_X_NEG;
	bool JOYSTICK_Y_POS;
	bool JOYSTICK_Y_NEG;

	int JOYSTICK_X;
	int JOYSTICK_Y;
	int CSTICK_X;
	int CSTICK_Y;

	GamecubeController(const char* name);
	~GamecubeController();

	void init();
	void gc_close();
	void capture();
	int blockingCapture();
	bool connected;
        void reset();

private:	
	bool* getItem(int i);
	void setAllToFalse();	

	const char* nameOfDevice;
	int device;
	char data[8];
	State* states[16];
	char base[8];
	
	bool prev_state[16];
	
	int prev_joystick_x;
	int prev_joystick_y;
	int prev_cstick_x;
	int prev_cstick_y;
};
