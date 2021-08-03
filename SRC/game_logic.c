#include "game_logic.h"
#include "ws2812b.h"
#include "string.h"

GAMELOGIC_cube_state_e cube_state = GAMELOGIC_STATE_POWER_UP;
GAMELOGIC_game_state_e game_state = GAMELOGIC_GAME_START;

gamelogic_data_struct_t game_data = 
{
    .gameButtonDisplayDuration = 2,
    .gameMaxLives = 3,
    .gameCurrentLives = 3,
    .gameSides = 6,
    .gameLevel = 0    
};

gamelogic_init_struct_t game_init_struct = {0};

void game_init();
void game_update();
bool calibrate(bool newCalibrate);

void gamelogic_init(gamelogic_init_struct_t * init_struct)
{
    GAMELOGIC_cube_state_e cube_state = GAMELOGIC_STATE_POWER_UP;
    game_init_struct.ledArray = init_struct->ledArray;
    game_init_struct.ledCount = init_struct->ledCount;
    game_init_struct.ble_update_callback = init_struct->ble_update_callback;
    game_init_struct.calibrate_callback = init_struct->calibrate_callback;
}

void gamelogic_update() //25ms
{
    switch(cube_state)
    {
        case GAMELOGIC_STATE_POWER_UP:
            calibrate(true); //changes to calibration
            cube_state = GAMELOGIC_STATE_CALIBRATION;
        break;

        case GAMELOGIC_STATE_CALIBRATION:
            if(calibrate(false))
            {
                int i;
                for(int i = 0; i < game_init_struct.ledCount; i++)
                {
                    game_init_struct.ledArray[i].RED = 0;
                    game_init_struct.ledArray[i].BLUE = 0;
                    game_init_struct.ledArray[i].GREEN = 0;
                }
                cube_state = GAMELOGIC_STATE_RESET; // after calibrated start the game logic                
            }
        break;

        case GAMELOGIC_STATE_RESET:
            //game_init();
            cube_state = GAMELOGIC_STATE_GAME;
        break;

        case GAMELOGIC_STATE_GAME:
            //game_update(); 
        break;
    }
}

void gamelogic_newSettings(gamelogic_data_struct_t * new_settings)
{
    cube_state = GAMELOGIC_STATE_RESET;
    memcpy(&game_data,new_settings,sizeof(game_data));
}

gamelogic_data_struct_t gamelogic_getDataStruct(void)
{
    gamelogic_data_struct_t retval = {0};
    retval.gameButtonDisplayDuration = game_data.gameButtonDisplayDuration;
    retval.gameLevel = game_data.gameLevel;
    retval.gameMaxLives = game_data.gameMaxLives;
    retval.gameCurrentLives = game_data.gameCurrentLives;
    retval.gameSides = game_data.gameSides;
    return retval;
}

bool calibrate(bool newCalibrate)
{
    static bool finished = true;
    if(newCalibrate)
    {
        uint8_t i;
        finished = false;
        for(i = 0; i < game_init_struct.ledCount; i++)
        {
            game_init_struct.ledArray[i].BLUE = 0;
            game_init_struct.ledArray[i].RED = 100;
            game_init_struct.ledArray[i].GREEN = 0;
        }
        if(game_init_struct.calibrate_callback != NULL)
        {
            game_init_struct.calibrate_callback(newCalibrate);
        }
        else
        {
            finished = true; //error!!! need callback that does the calibration for the event processing later on
        }
    }
    else
    {
        //wait 150 readings
        //read 150 readings
        //meanwhile set LEDs red
        //when done set finished true
        //so how do I access button data, callback?
        // how about a straight up calibrate from main callback?
        if(game_init_struct.calibrate_callback != NULL)
        {
            finished = game_init_struct.calibrate_callback(newCalibrate); //should be false normally
        }
        else
        {
            finished = true; //error!!! shouldn't happen unless callback gets cleared mid run
        }   
    }
    return finished;
}

void gamelogic_button_state_change_event(uint8_t button_index, uint8_t new_state, uint8_t prev_state)
{
    switch(new_state)
    {
        case 0:
           game_init_struct.ledArray[button_index].BLUE = 100;
           game_init_struct.ledArray[button_index].GREEN = 0;
           game_init_struct.ledArray[button_index].RED = 0;
        break;
        case 1:
           game_init_struct.ledArray[button_index].BLUE = 0;
           game_init_struct.ledArray[button_index].GREEN = 255;
           game_init_struct.ledArray[button_index].RED = 0;
        break;
        case 2:
           game_init_struct.ledArray[button_index].BLUE = 0;
           game_init_struct.ledArray[button_index].GREEN = 150;
           game_init_struct.ledArray[button_index].RED = 150;
        break;
        case 3:
           game_init_struct.ledArray[button_index].BLUE = 0;
           game_init_struct.ledArray[button_index].GREEN = 0;
           game_init_struct.ledArray[button_index].RED = 255;
        break;
    }
}