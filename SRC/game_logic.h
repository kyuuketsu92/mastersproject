//Location for the main state machine and game logic system

//CUBE STATE
/* 
POWER UP -> CALIBRATION -> GAME_LOGIC at power up
GAME_LOGIC -> GAME_LOGIC on ble config change (game restart with new settings)
*/

//GAME STATE
/*
GAME_START -> LEVEL_ANIMATION -> PLAY_SEQUENCE -> USER_IN -> LEVEL_INC -> LEVEL_ANIMATION                       for success
GAME_START -> LEVEL_ANIMATION -> PLAY_SEQUENCE -> USER_IN -> MISS -> LIFE_DECREASE -> LEVEL_ANIMATION when      failure with retries left
GAME_START -> LEVEL_ANIMATION -> PLAY_SEQUENCE -> USER_IN -> MISS -> LIFE_DECREASE -> GAME_START                when all lives lost
*/

//Module prefix: GAMELOGIC_

//singe inclusion definition
#ifndef __GAMELOGIC_H__
#define __GAMELOGIC_H__

#include "stdint.h"
#include "ws2812b.h"

typedef enum{
    GAMELOGIC_STATE_POWER_UP,
    GAMELOGIC_STATE_CALIBRATION,
    GAMELOGIC_STATE_GAME,
    GAMELOGIC_STATE_RESET
}GAMELOGIC_cube_state_e;

typedef enum{
    GAMELOGIC_GAME_START,
    GAMELOGIC_GAME_LEVEL_START_ANIMATION,
    GAMELOGIC_GAME_SEQUENCE_PLAY,
    GAMELOGIC_GAME_SEQUENCE_WAIT,
    GAMELOGIC_GAME_USER_INPUT,
    GAMELOGIC_GAME_USER_MISS,
    GAMELOGIC_LEVEL_SUCCESS_ANIMATION,
    GAMELOGIC_LIFE_REDUCTION_ANIMATION,
    GAMELOGIC_LEVEL_FAIL_ANIMATION,
    GAMELOGIC_GAME_LEVEL_INCREASE
}GAMELOGIC_game_state_e;

typedef struct{
    uint8_t gameLevel;
    uint8_t gameMaxLives;
    uint8_t gameCurrentLives;
    uint8_t gameSides;
    uint8_t gameButtonDisplayDuration;
}gamelogic_data_struct_t;

typedef void (* gamelogic_alert_user_curr_life_changed_callback) (uint8_t currLife);
typedef bool (* gamelogic_external_calibration_process_callback) (bool newCalibrate);

typedef  struct{
    WS2812B_LED * ledArray;
    uint8_t ledCount;
    gamelogic_alert_user_curr_life_changed_callback ble_update_callback;
    gamelogic_external_calibration_process_callback calibrate_callback;
}gamelogic_init_struct_t; 

//I need a way to tell main what LED state this module wants
//Need of course a way to access timing and button press events

gamelogic_data_struct_t gamelogic_getDataStruct(void);
void gamelogic_init(gamelogic_init_struct_t * init_struct);
void gamelogic_newSettings(gamelogic_data_struct_t * new_settings);
void gamelogic_update(void);
void gamelogic_button_state_change_event(uint8_t button_index, uint8_t new_state, uint8_t prev_state);


#endif