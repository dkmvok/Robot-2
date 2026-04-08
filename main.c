//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************


#include <stdint.h>
#include "msp.h"
#include "..\CortexM.h"
#include "..\Clock.h"

typedef enum{

   SERVO1_PULSE,
   SERVO2_PULSE,
   SERVO3_PULSE,
   SERVO4_PULSE,
   SERVO_WAIT

}servoStates_t;

void rotate(servoStates_t state);
unsigned int get_duty(unsigned int pos, unsigned int timer);
void delay1ms(uint32_t dt);
void pwmInitA0(void);
void pwmInitA2(void);

#define MAX_TIME  (7500)
#define MAX_COUNT (40)
#define CLK (48000000)
#define PER (20)

int dutyA=500, dutyB=100,dutyC=500;
int count = 0;

const int lookup_table[3][2]={{750,-90}, {1125,0}, {1500,90}};
servoStates_t state = SERVO_WAIT;

void main(void) {

    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
    pwmInitA0();
    pwmInitA2();    //no

    while(1) {
        rotate(SERVO1_PULSE);
        rotate(SERVO_WAIT);
        rotate(SERVO2_PULSE);
        rotate(SERVO_WAIT);
        rotate(SERVO3_PULSE);
        rotate(SERVO_WAIT);
        rotate(SERVO4_PULSE);
        rotate(SERVO_WAIT);
       }

       for(;;){};
       return 0;
}

void delay(unsigned long ulCount){
  __asm ( "pdloop:  subs    r0, #1\n" "    bne    pdloop\n");
}

//ccr0=4000-> 4000/(10^6)
void rotate(servoStates_t state) {
    switch(state) {
        case SERVO1_PULSE: TIMER_A0->CCR[1] = get_duty(90,0);
                           break;
        case SERVO2_PULSE: TIMER_A0->CCR[2] = get_duty(0,0);
                           break;
        case SERVO3_PULSE: TIMER_A0->CCR[4] = get_duty(-90,0);
                           break;
        case SERVO4_PULSE: TIMER_A2->CCR[1]  = get_duty(90,1);
                           break;
        case SERVO_WAIT: break; delay1ms(10);
    }
}

unsigned int get_duty(unsigned int pos, unsigned int timer) {
    unsigned int j, i;
    unsigned retVal, retVal2;
    j=1;
    for(i = 0; i <3; i++) {
        if(lookup_table[i][j]==pos && timer == 0) {
               retVal = lookup_table[i][0]*((50)/100);
        }
        if(lookup_table[i][j]==pos && timer == 1) {
               retVal = lookup_table[i][0]*((100)/100);
        }
    }
    return retVal;
}

void delay1ms(uint32_t dt){
  while(dt){
    delay(CLK/9162);
    dt--;
  }
}

void TA2_0_IRQHandler(void) {
    count++;
    if(count >= MAX_COUNT) {
         count=0;
    }
    TIMER_A2->CCTL[0] &= ~0x0001;
}

//slau597c,slau365h
void pwmInitA2(void) { // works
    P5->DIR |= 0x40;  // P5.6, output 0100_0000
    P5->SEL0 |= 0x40; // P5.6,TimerA2.1 function
    P5->SEL1 &= ~0x40;
    TIMER_A2->CTL |= 0x02F0;      // reset and start Timer A2 in up mode 0000_0000_0001_0100 //works

    TIMER_A2->CCTL[0] = 0x0080;  // CCIO toggle 0000_0000_1001_0000,
    TIMER_A2->CCR[0] = MAX_TIME;  // 
    //TIMER_A2->EX0 = 0x0001;    // divder /1
    TIMER_A2->EX0 = 0x0000;    // input clock divider to 6

    TIMER_A2->CCTL[1] = 0x0040;    // CCR1 toggle/reset 0000_0000_0100_0000

    TIMER_A2->CTL |= 0x0014;      // reset and start Timer A2 in up mode 
}

void pwmInit5(void) {
    P5->DIR |= 0x40;  // P5.6, P5.7 output 1100_0000 0x40
    P5->SEL0 |= 0x40; // P5.6,P5.7 Timer0A function
    P5->SEL1 &= ~0x40;

    TIMER_A2->CCTL[0] = 0x0090;   // CCIO toggle 0000_0000_1001_0000
    TIMER_A2->CCR[0] = MAX_TIME;   
 
    TIMER_A2->CCTL[1] = 0x0040;    // CCR1 toggle/reset 0000_0000_0100_0000

    TIMER_A2->CTL = 0x02F0;   // 0000_0010_1111_0000

    TIMER_A2->EX0 = 0x0000;    // input clock divider to 6
    NVIC->IP[3] = (NVIC->IP[3]&0xFFFFFF00)|0x00000040;    //priority 2 11111111_00000000, 0100_0000
    NVIC->ISER[0] = 0x00001000;   // enable interrupt 12 in NVIC

    TIMER_A2->CTL |= 0x0014;      // reset and start Timer A2 in up mode 0000_0000_0001_0100 works
}

void pwmInitA0(void) { //works
    P2->DIR |= 0xB0;  // P2.4, P2.5 output  1011_0000= 8+2+1=11=B0 
    P2->SEL0 |=0xB0; // P2.4,P2.5 Timer0A function
    P2->SEL1 &= ~0xB0;
    TIMER_A2->CTL = 0x0280;   // 0000_0010_1000_0000// 

    TIMER_A0->CCTL[0] = 0x0080;  // CCIO toggle 0000_0000_1000_0000, bit7-5
    TIMER_A0->CCR[0] = MAX_TIME; 
    TIMER_A0->EX0 = 0x0000;    // divder /1

    TIMER_A0->CCTL[1] = 0x0040;    // CCR1 toggle/reset 0000_0000_0100_0000 

    TIMER_A0->CCTL[2] = 0x0040;   // CCR1 toggle/reset 0000_0000_0100_0000 

    TIMER_A0->CCTL[4] = 0x0040;   // CCR1 toggle/reset 0000_0000_0100_0000

    TIMER_A0->CTL = 0x02F0;    // works
}
