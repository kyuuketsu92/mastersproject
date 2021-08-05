#include "game_logic.h"
#include "ws2812b.h"
#include "string.h"
#include "stdlib.h"

GAMELOGIC_cube_state_e cube_state = GAMELOGIC_STATE_POWER_UP;
GAMELOGIC_game_state_e game_state = GAMELOGIC_GAME_START;

gamelogic_data_struct_t game_data = {0};

const WS2812B_LED easyDifficultyColours[] = {

{.RED = 255,  .BLUE = 0,    .GREEN = 0    },
{.RED = 0,    .BLUE = 255,  .GREEN = 0    },
{.RED = 0,    .BLUE = 0,    .GREEN = 255  },
{.RED = 255,  .BLUE = 255,  .GREEN = 255  },

{.RED = 255,  .BLUE = 255,  .GREEN = 0    },
{.RED = 0,    .BLUE = 255,  .GREEN = 255  },
{.RED = 255,  .BLUE = 0,    .GREEN = 255  },
{.RED = 51,   .BLUE = 51,   .GREEN = 255  },

{.RED = 156,  .BLUE = 25,   .GREEN = 204  },
{.RED = 247,  .BLUE = 48,   .GREEN = 117  },
{.RED = 125,  .BLUE = 255,  .GREEN = 181  },
{.RED = 148,  .BLUE = 212,  .GREEN = 0    },
//////
{.RED = 255,  .BLUE = 0,    .GREEN = 0    },
{.RED = 0,    .BLUE = 255,  .GREEN = 0    },
{.RED = 0,    .BLUE = 0,    .GREEN = 255  },
{.RED = 255,  .BLUE = 255,  .GREEN = 255  },

{.RED = 255,  .BLUE = 255,  .GREEN = 0    },
{.RED = 0,    .BLUE = 255,  .GREEN = 255  },
{.RED = 255,  .BLUE = 0,    .GREEN = 255  },
{.RED = 51,   .BLUE = 51,   .GREEN = 255  },

{.RED = 148,  .BLUE = 212,  .GREEN = 0    },
{.RED = 247,  .BLUE = 48,   .GREEN = 117  },
{.RED = 125,  .BLUE = 255,  .GREEN = 181  },
{.RED = 156,  .BLUE = 25,   .GREEN = 204  }
};

typedef struct{
   uint8_t action_index;
   uint8_t strength; //when applicable
   WS2812B_LED ledColour;
}action_sequence_element_t;

uint8_t current_animation_counter_seconds = 0;
uint8_t current_animation_second_passed_flag = 0;
uint8_t current_sequence_size = 0;
#define SEQUENCE_MAX_SIZE 50
action_sequence_element_t sequence_indexes[SEQUENCE_MAX_SIZE] = {0};

typedef  enum
{
    SENSE_NONE,
    SENSE_CORRECT,
    SENSE_WRONG
}sense_type_e;

sense_type_e senseType = SENSE_NONE;
action_sequence_element_t waitingForAction = {0};



gamelogic_init_struct_t game_init_struct = {0};

void game_init();
void game_update();
bool calibrate(bool newCalibrate);
bool startAnimation(bool newAnimation);
action_sequence_element_t newSequence(void);
bool playSequence(bool newAnimation);
bool senseSequence(bool newSense);
bool levelSuccessAnimation(bool newAnimation);
bool lifeReductionAnimation(bool newAnimation);
bool levelFailAnimation(bool newAnimation);

void gamelogic_init(gamelogic_init_struct_t * init_struct)
{
    GAMELOGIC_cube_state_e cube_state = GAMELOGIC_STATE_POWER_UP;
    game_init_struct.ledArray = init_struct->ledArray;
    game_init_struct.ledCount = init_struct->ledCount;
    game_init_struct.ble_update_callback = init_struct->ble_update_callback;
    game_init_struct.calibrate_callback = init_struct->calibrate_callback;
    game_init_struct.randseedgen_callback = init_struct->randseedgen_callback;

    game_data.gameButtonDisplayDuration = 3;
    game_data.gameMaxLives = 3;
    game_data.gameCurrentLives = 3;
    game_data.gameSides = 6;
    game_data.gameLevel = 1;
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
            game_init();
            cube_state = GAMELOGIC_STATE_GAME;
        break;

        case GAMELOGIC_STATE_GAME:
            game_update(); 
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
    if(game_state == GAMELOGIC_GAME_USER_INPUT)
    {
        //lighting
        switch(new_state)
        {
            case 0:
               if(game_data.gameLevel == 0)
               {
                   game_init_struct.ledArray[button_index] = easyDifficultyColours[button_index];
               }
               else
               {
                   game_init_struct.ledArray[button_index].BLUE = 0;
                   game_init_struct.ledArray[button_index].GREEN = 0;
                   game_init_struct.ledArray[button_index].RED = 0;
               }

               //on release state
               switch(game_data.gameLevel)
               {
                    case 0:
                    case 1:
                        if(button_index == waitingForAction.action_index)
                        {
                            senseType = SENSE_CORRECT;
                        }
                        else
                        {
                            senseType = SENSE_WRONG;
                        }
                    break;
                    case 2:
                        if(button_index == waitingForAction.action_index)
                        {
                            if(prev_state == waitingForAction.strength)
                            {
                                senseType = SENSE_CORRECT;
                            }
                            else
                            {
                                senseType = SENSE_WRONG;
                            }                        
                        }
                        else
                        {
                            senseType = SENSE_WRONG;
                        }
                    break;
               }
           
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
}

void game_init()
{
    game_state = GAMELOGIC_GAME_START;
    memset(sequence_indexes,0,SEQUENCE_MAX_SIZE);
    current_sequence_size = 0;
    if(game_init_struct.randseedgen_callback != NULL)
    {
        srand(game_init_struct.randseedgen_callback());
    }
    
}

void game_update()
{
    switch(game_state)
    {
        case GAMELOGIC_GAME_START:
            game_state = GAMELOGIC_GAME_LEVEL_INCREASE;
            startAnimation(true);
        break;

        case GAMELOGIC_GAME_LEVEL_INCREASE:
            //generate new sequence element
            //0-23 -> buttons -> when there are buttons we might need strength data as well.
            //24-26 -> angular shaking
            //27-29 -> lateral shaking
            sequence_indexes[current_sequence_size] = newSequence();
            current_sequence_size++;
            game_state = GAMELOGIC_GAME_LEVEL_START_ANIMATION;
            game_data.gameCurrentLives = game_data.gameMaxLives;
            startAnimation(true);
        break;
        
        case GAMELOGIC_GAME_LEVEL_START_ANIMATION:
            if(startAnimation(false))
            {
                game_state = GAMELOGIC_GAME_SEQUENCE_PLAY;                     
                playSequence(true);
            }
        break;

        case GAMELOGIC_GAME_SEQUENCE_PLAY:
            if(playSequence(false))
            {
                game_state = GAMELOGIC_GAME_USER_INPUT;
                if(game_data.gameLevel == 0)
                {
                    memcpy(game_init_struct.ledArray,easyDifficultyColours,sizeof(easyDifficultyColours));                    
                }
                senseSequence(true);
            }
        break;

        case GAMELOGIC_GAME_USER_INPUT:
            if(senseSequence(false))
            {
                game_state = GAMELOGIC_LEVEL_SUCCESS_ANIMATION;
                levelSuccessAnimation(true);
            }
        break;

        case GAMELOGIC_LEVEL_SUCCESS_ANIMATION:
            if(levelSuccessAnimation(false))
            {
                game_state = GAMELOGIC_GAME_LEVEL_INCREASE;
            }
        break;

        case GAMELOGIC_LIFE_REDUCTION_ANIMATION:
            if(lifeReductionAnimation(false))
            {
                  game_state = GAMELOGIC_GAME_LEVEL_START_ANIMATION;
                  startAnimation(true);
            }
        break;

        case GAMELOGIC_LEVEL_FAIL_ANIMATION:
            if(levelFailAnimation(false))
            {
                  game_init();
            }
        break;
    }
}

bool startAnimation(bool newAnimation)
{
    static bool finished = true;
    uint8_t i;
    if(newAnimation)
    {        
        finished = false;
        current_animation_counter_seconds = 0;
        current_animation_second_passed_flag = 0;
        for(i = 0; i < game_init_struct.ledCount; i++)
        {
            game_init_struct.ledArray[i].RED = 0;
            game_init_struct.ledArray[i].GREEN = 0;
            game_init_struct.ledArray[i].BLUE = 0;
        }
    }
    else
    {
        if(current_animation_second_passed_flag)
        {
            current_animation_counter_seconds++;
            switch(current_animation_counter_seconds)
            {
                case 1:
                    for(i = 0; i < game_init_struct.ledCount; i++)
                    {
                        game_init_struct.ledArray[i].RED = 0;
                        game_init_struct.ledArray[i].GREEN = 0;
                        game_init_struct.ledArray[i].BLUE = 255;
                    }
                break;

                case 2:
                    for(i = 0; i < game_init_struct.ledCount; i++)
                    {
                        game_init_struct.ledArray[i].RED = 0;
                        game_init_struct.ledArray[i].GREEN = 0;
                        game_init_struct.ledArray[i].BLUE = 255;
                    }
                    finished = true;
                    clearLeds();
                break;
            }
            current_animation_second_passed_flag = 0;
        }
    }
    return finished;
}

void gamelogic_second_has_passed_flag_set()
{
    current_animation_second_passed_flag = 1;
}

bool isLegitRand(uint8_t num)
{
    bool retval = true;
    uint8_t allowed_button_max_index = 0;
    allowed_button_max_index = (game_data.gameSides * 4)-1;

    if(num > allowed_button_max_index)
    {
        retval = false; //later gotta add valid accelerometer code in here
    }

    return retval;
}

action_sequence_element_t newSequence(void)
{
    uint8_t random_number = 0;
    action_sequence_element_t new_seq = {0};
    do
    {
        random_number = rand() % 30;
    }while(!isLegitRand(random_number));
    
    new_seq.action_index = random_number;
    do
    {
        random_number = rand() % 3; // 0 - 2
    }while(!isLegitRand(random_number));
    new_seq.strength = random_number+1; 

    if(new_seq.action_index < 24)
    {    
        uint8_t random_index;
        switch(game_data.gameLevel)
        {
            case 0:
                new_seq.ledColour.RED = easyDifficultyColours[new_seq.action_index].RED;
                new_seq.ledColour.GREEN = easyDifficultyColours[new_seq.action_index].GREEN;
                new_seq.ledColour.BLUE = easyDifficultyColours[new_seq.action_index].BLUE;
            break;
            case 1:
                random_index = rand() % 24;
                new_seq.ledColour.RED = easyDifficultyColours[random_index].RED;
                new_seq.ledColour.GREEN = easyDifficultyColours[random_index].GREEN;
                new_seq.ledColour.BLUE = easyDifficultyColours[random_index].BLUE;
            break;
            case 2:
                switch(new_seq.strength)
                {
                    case 1:
                        new_seq.ledColour.RED = 0;
                        new_seq.ledColour.GREEN = 255;
                        new_seq.ledColour.BLUE = 0;
                    break;
                    case 2:
                        new_seq.ledColour.RED = 255;
                        new_seq.ledColour.GREEN = 255;
                        new_seq.ledColour.BLUE = 0;
                    break;
                    case 3:
                        new_seq.ledColour.RED = 255;
                        new_seq.ledColour.GREEN = 0;
                        new_seq.ledColour.BLUE = 0;
                    break;
                }
            break;
        }
    }

    return new_seq;
}

void clearLeds(void)
{
    uint8_t i;
    for(i = 0; i < game_init_struct.ledCount; i++)
    {
        game_init_struct.ledArray[i].BLUE = 0;
        game_init_struct.ledArray[i].RED = 0;
        game_init_struct.ledArray[i].GREEN = 0;
    }
}

bool playSequence(bool newAnimation)
{
    static bool finished = true;
    static uint8_t currentSequenceAnimElapsed = 0;
    static int8_t sequenceIndex = 0;
    if(newAnimation)
    {
        finished = false;
        current_animation_second_passed_flag = 0;
        current_animation_counter_seconds = 0;
        currentSequenceAnimElapsed = 255;
        sequenceIndex = -1;
    }
    else
    {
        if(current_animation_second_passed_flag)
        {
            if(currentSequenceAnimElapsed >= game_data.gameButtonDisplayDuration)
            {
                currentSequenceAnimElapsed = 0;
                sequenceIndex++;
                clearLeds();
            }
            else
            {
                currentSequenceAnimElapsed++;
            }

            if(sequenceIndex >= current_sequence_size)
            {
                finished = true;
                clearLeds();
            }
            else
            {
                action_sequence_element_t action = sequence_indexes[sequenceIndex];
                if(action.action_index < 24)
                {
                    game_init_struct.ledArray[action.action_index].RED = action.ledColour.RED;
                    game_init_struct.ledArray[action.action_index].GREEN = action.ledColour.GREEN;
                    game_init_struct.ledArray[action.action_index].BLUE = action.ledColour.BLUE;
                }
                else
                {
                    //accelerometerAnimationNeeded
                }                
            }
            current_animation_second_passed_flag = 0;            
        }
    }
    return finished;
}

void reduceLives(void)
{
    game_data.gameCurrentLives--;
    if(game_data.gameCurrentLives > 0)
    {
        game_state = GAMELOGIC_LIFE_REDUCTION_ANIMATION;
        lifeReductionAnimation(true);
    }
    else
    {
        game_state = GAMELOGIC_LEVEL_FAIL_ANIMATION;
        levelFailAnimation(true);
    }
}

bool senseSequence(bool newSense)
{
    static bool finished = true;
    static uint8_t sequenceIndex = 0;
    if(newSense)
    {
        finished = false;
        sequenceIndex = 0;
        waitingForAction = sequence_indexes[sequenceIndex];
        senseType = SENSE_NONE;
    }
    else
    {
        switch(senseType)
        {
            case SENSE_CORRECT:
                //increment index and check if finished
                sequenceIndex++;
                senseType = SENSE_NONE;
                if(sequenceIndex >= current_sequence_size)
                {
                    finished = true;
                }
                else
                {
                    waitingForAction = sequence_indexes[sequenceIndex];
                }
            break;

            case SENSE_WRONG:
                //wrong animation and restart
                reduceLives();
                senseType = SENSE_NONE;
            break;

            case SENSE_NONE:
            break;
        }
    }
    return finished;
}

bool levelSuccessAnimation(bool newAnimation)
{
    static bool finished = false;
    static uint8_t animationDurationElapsed = 0;
    if(newAnimation)
    {
        int i;
        finished = false;
        current_animation_second_passed_flag = 0;
        animationDurationElapsed = 0;
        for(i = 0; i < game_init_struct.ledCount; i++)
        {
            game_init_struct.ledArray[i].RED = 0;
            game_init_struct.ledArray[i].GREEN = 255;
            game_init_struct.ledArray[i].BLUE = 0;
        }
    }
    else
    {
        if(current_animation_second_passed_flag)
        {
            animationDurationElapsed++;
            current_animation_second_passed_flag = 0;
            if(animationDurationElapsed >= 2)
            {
                finished = true;
                clearLeds();
            }
        }
        
    }
    return finished;
}

bool lifeReductionAnimation(bool newAnimation)
{
    static bool finished = false;
    static uint8_t animationDurationElapsed = 0;
    if(newAnimation)
    {
        int i;
        finished = false;
        current_animation_second_passed_flag = 0;
        animationDurationElapsed = 0;
        for(i = 0; i < game_init_struct.ledCount; i++)
        {
            game_init_struct.ledArray[i].RED = 255;
            game_init_struct.ledArray[i].GREEN = 255;
            game_init_struct.ledArray[i].BLUE = 0;
        }
    }
    else
    {
        if(current_animation_second_passed_flag)
        {
            animationDurationElapsed++;
            current_animation_second_passed_flag = 0;
            if(animationDurationElapsed >= 2)
            {
                finished = true;
                clearLeds();
            }
        }
        
    }
    return finished;
}
bool levelFailAnimation(bool newAnimation)
{
  static bool finished = false;
    static uint8_t animationDurationElapsed = 0;
    if(newAnimation)
    {
        int i;
        finished = false;
        current_animation_second_passed_flag = 0;
        animationDurationElapsed = 0;
        for(i = 0; i < game_init_struct.ledCount; i++)
        {
            game_init_struct.ledArray[i].RED = 255;
            game_init_struct.ledArray[i].GREEN = 0;
            game_init_struct.ledArray[i].BLUE = 0;
        }
    }
    else
    {
        if(current_animation_second_passed_flag)
        {
            animationDurationElapsed++;
            current_animation_second_passed_flag = 0;
            if(animationDurationElapsed >= 2)
            {
                finished = true;
                clearLeds();
            }
        }
        
    }
    return finished;
}
