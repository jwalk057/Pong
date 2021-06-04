/*	Author: lab
 *  Partner(s) Name: Jeremy Walker
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *		
 *		NOTE: The code provided and .h files worked, but columns 2 and 3 on my keypad seemed to be swapped, so for example i put that when key 2 is pressed, output to B is 3. This is because when i press key 3, the .h thinks its key 2 due to GetBit() function. As i used the .h file provided i cannot change it, so i changed my code in main instead. 
 *
 *		When i use the term "Right" or "Left", 
 *		i also mean "Top" and "Bottom" respectively.
 *
 *		You need to start the game in order to enable 2 players, once the ball is moving, pressing the 2nd menu button sets the AI off and enables player 2 control
 *
 *		Demo link:
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include "timer.h"
#include "scheduler.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


//Enum SMs
enum SwapGame_SM{Idle, G1, G2};
enum Move_SM{Wait, Press, Release};
enum Move2_SM{Wait2, Press2, Release2};
enum Display_SM{Run1, Run2, Run3};
enum Ball_SM{Reset, Go, GameOver};
enum Menu_SM{WaitB, PressB, ReleaseB};
enum AI_SM{WaitBall,MoveRight,MoveLeft};
enum Score_SM{NoGoal,Goal};
enum P2Switch_SM{Wait3, P2, LED_OFF};

//Func Decs
int Move_Tick(int state);
int Move2_Tick(int state);
int AI_Tick(int state);
int Ball_Tick(int state);
int Display_Tick(int state);
int Menu_Tick(int state);
int Game_Tick(int state);
int Score_Tick(int state);
int P2Switch_Tick(int state);

//Global Vars//
//Buttons and Bools
char b1 =0x00;
char b2 =0x00;
bool bPress = false;
char resetB = 0x00;
bool resetBool = false;
char gameMode = 0x00;
char mode = 0x00;
char modeBool = false;
bool ballMovedR = false;
bool ballMovedL = false;
bool p1Score = 0x00;
bool p2Score = 0x00;
bool p1Win = false;
bool p2Win = false;
bool modeP2 = false;
unsigned short LR  = 0;
unsigned short tmpZERO = 0;



//Output Pattern
char pattern = 0x00;	//Output Vars used to right to PORTS
char row = 0x00;
//SM Vars to write to Output Vars
char paddle_P = 0x38;	//User Paddle
char paddle_R = 0xFE;
char wall_P = 0x1C;	//AI Paddle
char wall_R = 0xEF;
char  ball_P = 0x08;	//Ball
char ball_R = 0xFB;

//Temp Vars for Ball POS
char tmpP_p= 0x08;//Past Val
char tmpP_r= 0xFB;
char tmpC_p= 0x08;//Curr Val
char tmpC_r= 0xFB;
char tmpF_p= 0x08;//Future Val
char tmpF_r= 0xFB;

void A2D_init() {
        ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
        // ADEN: Enables analog-to-digital conversion
        // ADSC: Starts analog-to-digital conversion
        // ADATE: Enables auto-triggering, allowing for constant
        //          analog to digital conversions.
        }

int main(void) {

	A2D_init();

	LR  = ADC;
	tmpZERO = LR; // Sets a zero point for joystick

    /* Insert DDR and PORT initializations */
	DDRD = 0xFF; PORTD = 0x00; // D as LED ouput 
	DDRC = 0xFF; PORTC = 0x00; // C as LED output
	DDRB = 0xF0; PORTB = 0x0F; // B as button input
	
	
	
	task MOVE_Task, MOVEP2_Task, SCORE_Task, MENU_Task, GAME_Task, BALL_Task, DIS_Task, AI_Task, P2_Task;
	task tasks[] = {MOVE_Task, MOVEP2_Task, SCORE_Task, MENU_Task, GAME_Task, BALL_Task, DIS_Task, AI_Task, P2_Task};
	const char numTasks = 9;
	int period = 5;

	MOVE_Task.state = 0;
	MOVE_Task.period = 20;
	MOVE_Task.elapsedTime = MOVE_Task.period;
	MOVE_Task.TickFct = &Move_Tick;

	MOVEP2_Task.state = 0;
        MOVEP2_Task.period = 50;
        MOVEP2_Task.elapsedTime = MOVEP2_Task.period;
        MOVEP2_Task.TickFct = &Move2_Tick;

	AI_Task.state = 0;
        AI_Task.period = 50;
        AI_Task.elapsedTime = AI_Task.period;
        AI_Task.TickFct = &AI_Tick;

	SCORE_Task.state = 0;
        SCORE_Task.period = 100;
        SCORE_Task.elapsedTime = SCORE_Task.period;
        SCORE_Task.TickFct = &Score_Tick;

	GAME_Task.state = 1;
        GAME_Task.period = 20;
        GAME_Task.elapsedTime = GAME_Task.period;
        GAME_Task.TickFct = &Game_Tick;

	BALL_Task.state = 0;
        BALL_Task.period = 300;	// Will use for loop for speed, 300 ms for regular, 200 for fast
        BALL_Task.elapsedTime = BALL_Task.period;
        BALL_Task.TickFct = &Ball_Tick;
	
	P2_Task.state = 0;
        P2_Task.period =20;
        P2_Task.elapsedTime = P2_Task.period;
        P2_Task.TickFct = &P2Switch_Tick;

	DIS_Task.state = 0;
        DIS_Task.period = 5;
        DIS_Task.elapsedTime = DIS_Task.period;
        DIS_Task.TickFct = &Display_Tick;

	MENU_Task.state = 0;
        MENU_Task.period =20;
        MENU_Task.elapsedTime = MENU_Task.period;
        MENU_Task.TickFct = &Menu_Tick;

	TimerSet(period);
	TimerOn();
	int i = 0x00;

	srand(time(NULL)); //Initialize srand

    /* Insert your solution below */
while (1) {
	for(i = 0; i < numTasks;i++){
		if(tasks[i].elapsedTime == tasks[i].period){
				tasks[i].state = tasks[i].TickFct(tasks[i].state);
				tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += period;
	}
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}

int hold = 50;
int t = 0;

int P2Switch_Tick(int state){
	switch(state){
		case Wait3:
			if(gameMode){state = P2;}
			else state = Wait3;
			break;
		case P2:
			if(t < hold){ state = P2; t++;}
			else{state = LED_OFF; t =0;}
			PORTB = (PORTB & 0x0F) | 0xF0;
			break;
		case LED_OFF:
			PORTB = PORTB & 0x0F;
			state = Wait3;
			break;
	}
	return state;
}

int Score_Tick(int state){
	switch(state){
		case NoGoal:
			if(p1Score || p2Score){
				state = Goal;
			}
			else {
				state = NoGoal;
			}
			break;
		case Goal:
			if(p2Score && !(PINB&0x10)){
				p2Score = false;
				PORTB = PORTB | 0x10;
				
			}
			else if(p2Score && !(PINB&0x20)){
				p2Score = false;
                                PORTB = PORTB |	0x30;
				p2Win = true;
			}
			else if(p1Score && !(PINB&0x80)){
				p1Score = false;
                                PORTB = PORTB | 0x80;
		
			}
			else if(p1Score && !(PINB&0x40)){
				p1Score = false;
                                PORTB = PORTB | 0xC0;
				p1Win = true;
			}
			break;

	}
	return state;
}

int AI_Tick(int state){
	int brain = rand() % 20;
	switch(state){
		case WaitBall:
			if(ballMovedR&&(wall_P!=0x07)&&!(modeP2)){
				ballMovedR = false;
				state = MoveRight;
			}
			else if(ballMovedL&&!(wall_P&0x80)&&!(modeP2)){
				ballMovedL = false;
				state = MoveLeft;
			}
			else{state = WaitBall;}
			break;
		case MoveRight:
			if(wall_P&0x80){
				if((brain % 4) == 1){state = WaitBall;}
				else{
					state = WaitBall;
					wall_P = 0x70;
				}
			}
			else{
				if((brain % 4)==1){state = WaitBall;}
				else{
					state = WaitBall;
					wall_P >>=1;
				}
			}
			break;
		case MoveLeft:
			if((brain % 4)==1){state = WaitBall;}
			else{
				state = WaitBall;
                        	wall_P <<=1;
			}
			break;
	}
	return state;
}

int Menu_Tick(int state){
	resetB = ~PINB & 0x01;
	gameMode = ~PINB & 0x02;
	switch(state){
	case WaitB:
		if(resetB && !resetBool){
			resetBool = true;
			state = PressB;
		}
		else if(gameMode && !modeBool){
			modeBool = true;
			state = PressB;
		}
		else{
			state = WaitB;
		}
		break;
	case PressB:
		state = ReleaseB;
	       break;	
	case ReleaseB:
	       if(resetB||gameMode){
			state = Press;
	       }
	       else{
			state = WaitB;
	       }
	       break;
	}
	return state;
}

int Display_Tick(int state){
switch(state){
	case Run1:
		state = Run2;
		pattern = paddle_P;
                row = paddle_R;
		break;
	case Run2:
		state = Run3;
		pattern = wall_P;
                row = wall_R;
		break;
	case Run3:
                state = Run1;
                pattern = ball_P;
                row = ball_R;
                break;
}

PORTC = pattern;
PORTD = row;
return state;

}

int Game_Tick(int state){
	// mode -> 0:G1 1:G2
	// starts in G1
	switch(state){
		case Idle:
			if(modeBool && !mode){
				mode = 1;		//Set to 1, will go to G2 when button is pressed
				state = G1;
			}
			else if(modeBool && mode){
                                mode = 0;		//Set to 0, will go to G1 when button is pressed
                                state = G2;
			}
			else{state = Idle;}
			break;
		case G1:	//AI Game
			wall_P = 0x1C;
			modeBool = false;
			modeP2 = false;		//Enables AI
			state = Idle;
			break;
		case G2 :	//Player 2 control
			wall_P = 0x1C;
                        modeBool = false;
			modeP2 = true;		//Disables AI
                        state = Idle;
			break;
	}
	return state;
}

int Ball_Tick(int state){

	switch(state){
		case Reset:
			resetBool=false;
			ball_P = 0x08;
			ball_R = 0xFB;
			if(modeP2){}
			else{
			wall_P = 0x1C;
			}
			if(p1Win||p2Win){
				PORTB = PORTB & 0x0F;
                                p2Win = false;
                                p1Win = false;
			}
			else if(bPress){//Initialize past pos 
				tmpP_p = ball_P;
				tmpP_r = ball_R;
				state = Go;
				ball_R >>=1;
				bPress=0;
			}
			else state = Reset;
			break;
		case Go:
			tmpC_p = ball_P; //Initialize current pos
			tmpC_r = ball_R;
			tmpF_p = ball_P; //Initialize current pos, to be future pos
			tmpF_r = ball_R;
//Checks reset button first
			if(resetBool){
				PORTB = PORTB & 0x0F;
				ball_P = 0x00;
				state = GameOver;
			}
			if(ball_R >=0xFF){
				ball_P = 0x00;
                                state = GameOver;
			}
	//Else play the game
		else{
			if(tmpC_r == tmpP_r>>1){
//Ball Physics, going right
		
			//Check Reset
			 if(resetBool){
                                        ball_P=0x00;
                                        state = GameOver;
					PORTB = PORTB & 0x0F;
                          }
			 else{
				if(tmpC_p == tmpP_p){	//Case: Ball going Straight
					if(resetBool){
						PORTB = PORTB & 0x0F;
                                        	ball_P=0x00;
                                        	state = GameOver;
                                	}
					else{
					tmpF_r >>=1;
					if((tmpF_r == paddle_R)&&(tmpF_p ==(paddle_P & tmpF_p))){		//Collision Paddle 
			//Left
						if(tmpC_p==((paddle_P>>2) & tmpC_p)){
							tmpP_r = ball_R;
							tmpP_p = ball_P;
							ball_R = ball_R << 1 | 0x01;
							ball_P >>= 1;
							ballMovedR = true;
							state=Go;
						}
			//Mid
						else if(tmpC_p==((paddle_P>>1)& tmpC_p)){
							tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R <<1 | 0x01;
                                                        state = Go;
						}
			//Right
						else{
							tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R << 1 | 0x01;
                                                        ball_P <<= 1;
							ballMovedL = true;
							state=Go;
						}
					}
					else if(tmpC_r==paddle_R){		//Collision Game Over
						ball_R =0x00;
						state=GameOver;
						p2Score = true;
					}
					else{
						tmpP_r = ball_R;
                                                tmpP_p = ball_P;
                                                ball_R >>=1;
                                                state = Go;
					}
					}
				}
	//Case: Going Right, from Left 
				else if(tmpC_p == (tmpP_p<<1)){
					
				//Check Reset Again, another odd case
					if(resetBool){
						ball_P = 0x00;
						state = GameOver;
						PORTB = PORTB & 0x0F;
					}
					else{
					if(!(tmpC_p&0x80)){
					tmpF_p <<=1;
                                        tmpF_r = tmpF_r>>1;
					}
					else{
						tmpF_p = 0x40;
						tmpF_r >>=1;
					}
					if(((tmpF_r == paddle_R)&&(paddle_P & tmpF_p))||( (tmpC_p & paddle_P)&&((tmpC_r>>1) == paddle_R))){
						 //Collision Paddle 
                        //Left edge or paddle
                                                if((paddle_P&0x80)&&(tmpC_p&0x40)&&(tmpF_r == paddle_R)){
							tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R << 1 | 0x01;
                                                        ball_P <<= 1;
							ballMovedL = true;
                                                        state=Go;
						}
						else if(((paddle_P>>2) & tmpF_p)||((paddle_P>>1)& tmpF_p)){
                                                        
							tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R << 1 | 0x01;
                                                        ball_P >>= 1;
							ballMovedR=true;
                                                        state=Go;
                                                }
                        //Mid Paddle
						else if(paddle_P & tmpF_p){
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R <<1 | 0x01;
							ball_P =ball_P <<1;
							ballMovedL = true;
                                                        state = Go;
                                                }
                        //Right Paddle
                                                else{	
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R << 1 | 0x01;
                                                       	ball_P <<=1;
							ballMovedL = true;
                                                        state=Go;
                                                }
					
					}
					else if(tmpC_r==paddle_R){         //Collision Game Over
                                                ball_R =0x00;
                                                state=GameOver;
						p2Score = true;
                                        }
					else{	
						tmpP_r = ball_R;
                                                tmpP_p = ball_P;
						ball_P<<=1;
						ballMovedL = true;
                                                ball_R >>=1;
                                                state = Go;
					
					}
					}

				}
	//Case: Going Right, from Right
				else{
					if(resetBool){
						ball_P=0xFF;
						state=GameOver;
						PORTB = PORTB & 0x0F;
					}
					else{
	//Set up Future pos, first case is for top wall Corner
					if((tmpC_p&0x80)&&(tmpC_r == 0xFD)){
						tmpC_p = 0x80;			//This still doesnt assign them the value, it wont work with ==
						tmpC_r = 0xFD;
						tmpF_p = 0x40;
						tmpF_r = 0xFE;
					}
					else{
						tmpF_p >>=1;
						tmpF_r = tmpF_r>>1;
					}

					//special collisons when ball is flush between corner and paddle, and when ball is rebounding off wall, into right corner of paddle in top right
					if(((tmpC_p & 0x80) && (tmpF_r == paddle_R)&&(paddle_P & 0x80))||((tmpC_p & 0x80)&&(tmpF_r ==paddle_R)&&(tmpC_p>>1 & paddle_P))){
						
                                                        
						tmpP_p = 0x80;
                                                        tmpP_r = ball_R;
                                                        ball_P =0x40;
							ballMovedR=true;
                                                        ball_R = ball_R << 1 | 0x01;
                                                        state = Go;
							
                                                }
					else if(((tmpF_r == paddle_R)&&(paddle_P & tmpF_p))||((tmpC_p & paddle_P)&&((tmpF_r == paddle_R)))){               //Collision Paddle
						
                        //Right edge and Paddle
						if(((paddle_P<<2) & tmpF_p)||((paddle_P<<1) & tmpF_p)){
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R << 1 | 0x01;
                                                        ball_P <<= 1;
							ballMovedL = true;
                                                        state=Go;
                                                }
                        //Mid
                                                else if(paddle_P& tmpF_p){
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R <<1 | 0x01;
							ball_P>>=1;
							ballMovedR = true;
                                                        state = Go;
                                                }
                        //Left Paddle
                                                else{
							
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R << 1 | 0x01;
                                                        ball_P >>= 1;
							ballMovedR = true;
                                                        state=Go;
							
                                                }
                                        }
					else if(tmpC_r == paddle_R){         //Collision Game Over
                                        	
					       	ball_R =0x00;
                                                state=GameOver;
						p2Score = true;
                                        }
					else if(tmpC_p &0x80){		//Top wall right edge
						if(tmpF_r ==paddle_R){	// Case were its about to hit right side with no paddle
							ball_P =0x40;	
						       	ball_R =0x00;
                                       			state=GameOver;
							p2Score = true;
							}
						else{				// Else bounce on
							
							tmpP_p=0x80;
                                                	tmpP_r=ball_R;
                                                	ball_P = 0x40;
							ballMovedR = true;
                                                	ball_R = ball_R>>1;
                                                	state=Go;
							
							
						}
					}	
					else if((tmpF_p== 0x00)&&!(tmpF_r == paddle_R)){					//check bottom wall
						tmpP_p=ball_P;
                                                tmpP_r=ball_R;
                                                ball_P<<=1;
						ballMovedL = true;
                                                ball_R = ball_R>>1;
                                                state=Go;

					}
					else{
						
						tmpP_r = ball_R;
                                                tmpP_p = ball_P;
                                                ball_P>>=1;
						ballMovedR = true;
                                                ball_R >>=1;
                                                state = Go;
						
					}
				}
				}
				}
			}
			else{ //Ball Physics, going left
				
				if(resetBool){
					ball_P=0x00;
					state = GameOver;
					PORTB = PORTB & 0x0F;
				}
				else{
				 if(tmpC_p == tmpP_p){   //Case: Ball going Straight
					 
					  if(resetBool){
                                        	ball_P=0x00;
                                        	state = GameOver;
						PORTB = PORTB & 0x0F;
                                		}
					  else{
                                        tmpF_r = tmpF_r << 1 | 0x01;
                                        if((tmpF_r == wall_R)&&(wall_P & tmpF_p)){               //Collision AI Paddle 
                        //Right edge
			
                                                if((wall_P>>2) & tmpC_p){
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P >>= 1;
							ballMovedR = true;
                                                        state=Go;
                                                }
                        //Mid
                                                else if((wall_P>>1)& tmpC_p){
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        state = Go;
                                                }
                        //Left Edge
                                                else{
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P <<= 1;
							ballMovedL = true;
                                                        state=Go;
                                                }
						
                                        }
                                      else if((tmpC_r == wall_R)&&!resetBool){ 
						ball_R =0x00;
                                                state=GameOver;
						p1Score=true;
                                        }
                                        else{		//Else go left straight
						
						tmpP_r = ball_R;
						tmpP_p = ball_P;
						ball_R = ball_R<<1 | 0x01;
						state=Go;
						
                                        }
					  }
                                }
				 //Going Right(hit from Right side of paddle)
                                else if(tmpC_p == (tmpP_p<<1)){
				//Check Reset, this was an odd case so i had to hardwire to balls POS
					if(resetBool){
						ball_P=0x00;
						state = GameOver;
						PORTB = PORTB & 0x0F;
					}
					else{
					if(!(tmpC_p & 0x80)){
						tmpF_p <<=1;
						tmpF_r = tmpF_r <<1 | 0x01;
					}
					else{
						
						tmpF_p =  0x40;
						tmpF_r = tmpF_r << 1 | 0x01;
					}
					if(((tmpC_p &0x80)&&((tmpC_r<<1 |0x01)==wall_R)&&(wall_P&0x80))||((tmpC_p & 0x80)&&((tmpC_r<<1 |0x01)==wall_R)&&(wall_P&0x40))){
							tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P >>= 1;
							ballMovedR = true;
                                                        state=Go;

					}
					else if(((tmpF_r == wall_R)&&(wall_P & tmpF_p))||((tmpC_p&wall_P)&&((tmpC_r <<1 |0x01)==wall_R)&&(tmpF_p&wall_P))){               //Collision AI Paddle 
                        			if((tmpC_p & 0x40)&&(tmpC_r <<1 & wall_R)&&(wall_P&0x40)){
							tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P <<= 1;
							ballMovedL = true;
                                                        state=Go;
						}
						//Right edge
						else if(((wall_P>>2) & tmpF_p)||((wall_P>>1) & tmpF_p)){
                                                   
						     	
							tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P >>= 1;
							ballMovedR = true;
                                                        state=Go;
							
                                                }
                        //Mid
                                                else if(wall_P& tmpF_p){
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
							ball_P <<=1;
							ballMovedL = true;
                                                        state = Go;
                                                }
                        //Left Edge
                                                else{
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P <<= 1;
							ballMovedL = true;
                                                        state=Go;
                                                }

                                        }
					else if(tmpC_r == wall_R){
						ball_R =0x00;
                                                state=GameOver;
						p1Score=true;
					}
					else{
						tmpP_p = ball_P;
						tmpP_r = ball_R;
						ball_P <<=1;
						ballMovedL = true;
						ball_R = ball_R << 1 |0x01;
						state = Go;
					}
					}
					
                                }
				//Going Left(hit from Left side of paddle)
                                else{
					 if(resetBool){
                                        	ball_P=0x00;
                                        	state = GameOver;
						PORTB = PORTB & 0x0F;
                                	}
					else{
					if((tmpC_p&0x80)&&(tmpF_r == wall_R)){  // Top left corner case
						tmpC_p = 0x80;
						tmpC_r = 0xF7;
						tmpF_p = 0x40;
						tmpF_r = 0xEF;
					}
					else{
					tmpF_p >>=1;
                                        tmpF_r = tmpF_r<<1 |0x01;
					}
					//Special Collision with top wall and paddle
					if(((tmpC_p & 0x80) && (tmpF_r == wall_R)&&(wall_P & 0x80))||((tmpC_p & 0x80)&&(tmpF_r == wall_R)&&(tmpC_p>>1 & wall_P))){
							
							tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P = 0x40;
							ballMovedR = true;
                                                        state=Go;
							
					}
					else if(((tmpF_r == wall_R)&&(wall_P & tmpF_p))||((tmpC_p&wall_P)&&((tmpC_r <<1 |0x01)==wall_R))){               //Collision AI Paddle 
                  
						if(((tmpC_p&0x01)&&((tmpC_r <<1 |0x01) == wall_R))||((tmpC_p==0x00)&&(wall_R>>1 & tmpC_p))){	//Case when ball is in bottom left corner					
							tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P <<= 1;
							ballMovedL = true;
                                                        state=Go;

						}			
			//Left edge/Paddle
						else if(((wall_P<<2) & tmpF_p)||((wall_P<<1) & tmpF_p)){
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P <<= 1;
							ballMovedL = true;
                                                        state=Go;
                                                }
                        //Mid
                                                else if(wall_P& tmpF_p){
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
							ball_P >>=1;
							ballMovedR = true;
                                                        state = Go;
                                                }
                        //Right Paddle
                                                else{
                                                        tmpP_r = ball_R;
                                                        tmpP_p = ball_P;
                                                        ball_R = ball_R >>1;
                                                        ball_P >>= 1;
							ballMovedR = true;
                                                        state=Go;
                                                }

                                        }
					else if((tmpC_p & 0x01)&&!(tmpC_r == wall_R)){                           // check side wall
						
                                                tmpP_p=ball_P;
                                                tmpP_r=ball_R;
                                                ball_P<<=1;
						ballMovedL = true;
                                                ball_R = ball_R<<1|0x01;
                                                state=Go;
						
                                        }
                                        else if(tmpC_r==wall_R){                             
 	                                        ball_R =0x00;
                                                state=GameOver;
						p1Score=true;
                                        }
                                	else if((tmpC_p& 0x80)&&(tmpF_r == wall_R)){ 				//check corner case
						if(tmpF_r ==wall_R){       // Case were its about to hit right side with no paddle
                                                        ball_P =0x40;
                                                        ball_R =0x00;
                                                        state=GameOver;
							p1Score = true;
                                                }
                                                else{                           // Else bounce on
                                                
							tmpP_p=ball_P;
                                                        tmpP_r=ball_R;
                                                        ball_P>>=1;
							ballMovedR = true;
                                                        ball_R = ball_R<<1 | 0x01;
                                                        state=Go;
                                                
							}
                                        }
					else if((tmpC_p& 0x80)&&!(tmpF_r == wall_R)){ 
							tmpP_p=0x80;
                                                        tmpP_r=ball_R;
                                                        ball_P=0x40;
							ballMovedR = true;
                                                        ball_R = ball_R<<1 | 0x01;
                                                        state=Go;
					}
                                        else{
						
                                                tmpP_p=ball_P;
                                                tmpP_r=ball_R;
                                                ball_P>>=1;
						ballMovedR = true;
                                                ball_R=ball_R<<1|0x01;
                                                state = Go;
						
                                        }
					}

                                }
				}

			}

			}
			
			break;
		case GameOver:
			do{
				ball_R = 0xFF;
				paddle_P = 0x38;
				wall_P = 0x1C;
				state = GameOver;}while((b1||b2)&& !resetBool);
			if(!(b1||b2)||resetBool){bPress=0;state = Reset; resetBool = false;}
			else state = GameOver;
			break;
	}
	return state;
}

int Move_Tick(int state){
      	b1 = ~PINB & 0x04;
	b2 = ~PINB & 0x08;
	switch(state){
		case Wait:
			
			if((paddle_P&0x80)&&b2){state = Wait;}
			else if((paddle_P&0x80)&&b1){paddle_P = 0x70;state = Press;}// Had an issue with the LEDs all lighting up when leaving the edge of the matrix, this fixed it
			else if((paddle_P == 0x07)&&b1){state = Wait;}
			else if(b1&&!(paddle_P&0x01)){state = Press;paddle_P >>=1;
			bPress=true;}
			else if(b2&&!(paddle_P&0x80)){state = Press; paddle_P <<=1;bPress=true;}
			else state = Wait;
			break;
		case Press:
			state = Release;
	
			break;
		case Release:
			if(b1||b2){state = Release;}
			else state = Wait;
			break;
	}
	switch(state){
		case Wait:
			break;
		case Press:
			break;
		case Release:
			break;
	}
	 
	return state;

}

int Move2_Tick(int state){
        LR = ADC;
        switch(state){
                case Wait2:

                        if((wall_P&0x80)&&(LR<tmpZERO)&&(modeP2)){state = Wait2;}
                        else if((wall_P&0x80)&&(LR>tmpZERO)&&(modeP2)){wall_P = 0x70;state = Press2;}// Had an issue with the LEDs all lighting up when leaving the edge of the matrix, this fixed it
                        else if((wall_P == 0x07)&&(LR>tmpZERO)&&(modeP2)){state = Wait2;}
                        else if((LR>tmpZERO)&&!(wall_P&0x01)&&(modeP2)){state = Press2;wall_P >>=1;
                      }
                        else if((LR<tmpZERO)&&!(paddle_P&0x80)&&(modeP2)){state = Press2; wall_P <<=1;}
                        else state = Wait;
                        break;
                case Press2:
                        state = Release2;
                        break;
                case Release2:
                        if(LR!=tmpZERO){state = Release2;}
                        else state = Wait2;
                        break;
        }
        switch(state){
                case Wait2:
                        break;
                case Press2:
                        break;
                case Release2:
                        break;
        }

        return state;

}



