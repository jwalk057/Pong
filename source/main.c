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
 *		Demo link:
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <stdbool.h>
#include "timer.h"
#include "scheduler.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


//Enum States
enum SwapGame_SM{Idle, G1, G2};
enum Move_SM{Wait, Press, Release};
enum Display_SM{Run1, Run2, Run3};
enum Ball_SM{Reset, Go, GameOver};
enum Menu_SM{WaitB, PressB, ReleaseB};
enum AI_SM{WaitBall,MoveRight,MoveLeft};

//Func Decs
int Move_Tick(int state);
int AI_Tick(int state);
int Ball_Tick(int state);
int Display_Tick(int state);
int Menu_Tick(int state);
int Game_Tick(int state);

//Global Vars//
//Buttons
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



int main(void) {
    /* Insert DDR and PORT initializations */
	DDRD = 0xFF; PORTD = 0x00; // D as LED ouput 
	DDRC = 0xFF; PORTC = 0x00; // C as LED output
	DDRA = 0x00; PORTA = 0xFF; // A as user input
	DDRB = 0x00; PORTB = 0xFF; // B as button input
	
	task MOVE_Task, MENU_Task, GAME_Task, BALL_Task, DIS_Task, AI_Task;
	task tasks[] = {MOVE_Task, MENU_Task, GAME_Task, BALL_Task, DIS_Task, AI_Task};
	const char numTasks = 6;
	int period = 5;

	MOVE_Task.state = 0;
	MOVE_Task.period = 20;
	MOVE_Task.elapsedTime = MOVE_Task.period;
	MOVE_Task.TickFct = &Move_Tick;

	AI_Task.state = 0;
        AI_Task.period = 100;
        AI_Task.elapsedTime = AI_Task.period;
        AI_Task.TickFct = &AI_Tick;

	GAME_Task.state = 1;
        GAME_Task.period = 20;
        GAME_Task.elapsedTime = GAME_Task.period;
        GAME_Task.TickFct = &Game_Tick;

	BALL_Task.state = 0;
        BALL_Task.period =300;
        BALL_Task.elapsedTime = BALL_Task.period;
        BALL_Task.TickFct = &Ball_Tick;


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

int AI_Tick(int state){
	switch(state){
		case WaitBall:
			if(ballMovedR){
				ballMovedR = false;
				state = MoveRight;
			}
			else if(ballMovedL){
				ballMovedL = false;
				state = MoveLeft;
			}
			else{state = WaitBall;}
			break;
		case MoveRight:
			state = WaitBall;
			wall_P >>=1;
			break;
		case MoveLeft:
			state = WaitBall;
                        wall_P <<=1;
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
				modeBool = false;
                                mode = 0;		//Set to 0, will go to G1 when button is pressed
                                state = G2;
			}
			else{state = Idle;}
			break;
		case G1:	//AI Game
			wall_P = 0x1C;
			modeBool = false;
			state = Idle;
			break;
		case G2 :	//Player 2 eventually, wall for now
			wall_P = 0xFF;
                        modeBool = false;
                        state = Idle;
			break;
	}
	return state;
}

int Ball_Tick(int state){

	switch(state){
		case Reset:
			ball_P = 0x08;
			ball_R = 0xFB;
			wall_P = 0x1C;
			
			if(bPress){//Initialize past pos 
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
			if(resetBool||ball_R >=0xFF){
				ball_P = 0x00;
				state = GameOver;
				resetBool =false;
			}
	//Else play the game
		else{
			if(tmpC_r == tmpP_r>>1){
//Ball Physics, going right
		
			//Check Reset
			 if(resetBool){
                                        ball_P=0x00;
                                        state = GameOver;
					resetBool =false;
                          }
			 else{
				if(tmpC_p == tmpP_p){	//Case: Ball going Straight
					if(resetBool){
                                        	ball_P=0x00;
                                        	state = GameOver;
						resetBool =false;
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
					//ball_P=0xFF;
				//Check Reset Again, another odd case
					if(resetBool){
						ball_P = 0x00;
						state = GameOver;
						resetBool =false;
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

                                        }
					else if(tmpC_p &  0x80){	
			//	ball_P=0xFF;		
						/*tmpP_p = ball_P;
                                                tmpP_r = ball_R ;
                                                ball_P<<=1;
                                                ball_R>>=1;
                                                state=Go;
						*/
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
				//ball_P=0xFF;	
				
					if(resetBool){
						ball_P=0xFF;
						state=GameOver;
						resetBool =false;
					}
					else{
	//Set up Future pos, first case is for top wall Corner
					if((tmpC_p&0x80)&&(tmpC_r == 0xFD)){
						tmpC_p = 0x80;			//This still doesnt assign them the value, it wont work with ==
						tmpC_r = 0xFD;
						tmpF_p = 0x40;
						tmpF_r = 0xFE;
					}
					/*else if(tmpC_p &0x80){
						tmpC_p=0x80;
						tmpC_r=ball_R;
						tmpF_p=0x40;
						tmpF_r = tmpF_r>>1;
					}*/
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

                                        }
				/*	else if((tmpC_p&0x80)&&(tmpF_r == paddle_R)){ //Odd case where its at top wall corner, goes to POS R:0xFE P:0x40
						
						 tmpF_p = 0x40;
						 tmpF_r = 0xFE;
						if(tmpF_r == paddle_R&&!(paddle_P & 0xC0)){	
								//Game over
			
                                       				ball_P =0x40;
								ball_R = 0x00;
								state = GameOver;	
								
							}
					}
						*/
					else if(tmpC_p &0x80){		//Top wall right edge
						if(tmpF_r ==paddle_R){	// Case were its about to hit right side with no paddle
							ball_P =0x40;	
						       	ball_R =0x00;
                                       			state=GameOver;
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
					resetBool =false;
				}
				else{
				 if(tmpC_p == tmpP_p){   //Case: Ball going Straight
					 
					  if(resetBool){
                                        	ball_P=0x00;
                                        	state = GameOver;
						resetBool =false;
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
                                      else if(tmpC_r == wall_R){ //Will be NPC GameOver
						ball_R =0x00;
                                                state=GameOver;
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
						resetBool =false;
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
					}
					else if(tmpC_p&0x80){

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
						resetBool =false;
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

                                        }
                                	else if((tmpC_p& 0x80)&&(tmpF_r == wall_R)){ 				//check corner case
						if(tmpF_r ==wall_R){       // Case were its about to hit right side with no paddle
                                                        ball_P =0x40;
                                                        ball_R =0x00;
                                                        state=GameOver;
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
					//	ball_P=0xFF;
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
				state = GameOver;}while((b1||b2)&& !resetBool);
			if(!(b1||b2)||resetBool){bPress=0;state = Reset; resetBool = false;}
			else state = GameOver;
			break;
	}
	return state;
}

int Move_Tick(int state){
	// Local Variables
              // Row(s) displaying pattern.
                                                        // 0: display pattern on row
                                                        // 1: do NOT display pattern on row
	b1 = ~PINA & 0x01;
	b2 = ~PINA & 0x02;
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

