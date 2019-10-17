#include <avr/pgmspace.h>
#include "ht1632.h"

#define  P1WIN 0
#define  P2WIN 1
#define  PONG  2
#define  FIVE  3
#define  FOUR  4
#define  THREE  5
#define  TWO    6
#define  ONE    7
#define  CLS    8
/*
 * Set these constants to the values of the pins connected to the SureElectronics Module
 */
static const byte ht1632_data = 6;     // Data pin (pin 7)
static const byte ht1632_wrclk = 7;    // Write clock pin (pin 5)
static const byte ht1632_cs[] = {8,9};       // Chip Select (1, 2, 3, or 4)
static const byte ht1632_displays = 2; // Number of displays

static const byte bitmaps[][128]={
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,3,12,6,6,12,3,8,1,0,0,15,15,15,15,0,3,0,6,0,12,0,12,0,6,0,3,15,15,15,15,0,0,0,0,15,15,15,15,0,0,0,0,15,15,15,15,6,0,3,0,1,8,0,12,0,6,0,3,15,15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,15,15,15,0,3,0,6,0,12,0,12,0,6,0,3,15,15,15,15,0,0,0,0,15,15,15,15,0,0,0,0,15,15,15,15,6,0,3,0,1,8,0,12,0,6,0,3,15,15,15,15,0,0,8,1,12,3,6,6,3,12,1,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                    {1,2,3,4,15,0,15,0,13,0,8,0,13,0,8,0,13,0,8,0,15,0,8,0,7,0,0,0,0,0,0,0,7,0,14,0,15,0,15,12,3,12,3,12,3,15,15,7,14,0,0,15,15,15,15,6,0,3,0,1,8,0,12,15,15,15,15,0,0,7,14,15,15,12,3,13,11,13,11,13,15,5,14},
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,10,15,11,13,11,13,11,13,11,13,11,13,15,12,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,8,7,8,13,8,13,8,13,8,15,15,15,15,1,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,2,12,3,12,3,13,11,13,11,13,11,15,15,7,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,3,12,7,12,15,12,11,13,11,13,3,15,3,6,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,3,15,15,15,15,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
                };


byte ht1632_shadowram[128];

/*
 * ht1632_writebits
 * Write bits (up to 8) to h1632 on pins ht1632_data, ht1632_wrclk
 * Chip is assumed to already be chip-selected
 * Bits are shifted out from MSB to LSB, with the first bit sent
 * being (bits & firstbit), shifted till firsbit is zero.
 */
void ht1632_chipselect(byte chipno)
{
  digitalWrite(chipno, 0);
}

void ht1632_chipfree(byte chipno)
{
  digitalWrite(chipno, 1);
}

void ht1632_writebits (byte bits, byte firstbit)
{
  while (firstbit) {
    digitalWrite(ht1632_wrclk, LOW);
    if (bits & firstbit) {
      digitalWrite(ht1632_data, HIGH);
    }
    else {
      digitalWrite(ht1632_data, LOW);
    }
    digitalWrite(ht1632_wrclk, HIGH);
    firstbit >>= 1;
  }
}

static void ht1632_sendcmd (byte command,byte cs)
{
  ht1632_chipselect(cs);  // Select chip
  ht1632_writebits(HT1632_ID_CMD, 1<<2);  // send 3 bits of id: COMMMAND
  ht1632_writebits(command, 1<<7);  // send the actual command
  ht1632_writebits(0, 1); 	/* one extra dont-care bit in commands. */
  ht1632_chipfree(cs); //done
}

static void ht1632_senddata (byte address, byte data, byte cs)
{
  ht1632_chipselect(cs);  // Select chip
  ht1632_writebits(HT1632_ID_WR, 1<<2);  // send ID: WRITE to RAM
  ht1632_writebits(address, 1<<6); // Send address
  ht1632_writebits(data, 1<<3); // send 4 bits of data
  ht1632_chipfree(cs); // done
}

static void ht1632_sendshadowram ()
{
  for(byte cs=0;cs<ht1632_displays;cs++){
    ht1632_chipselect(ht1632_cs[cs]);  // Select chip
    ht1632_writebits(HT1632_ID_WR, 1<<2);  // send ID: WRITE to RAM
    ht1632_writebits(0,1<<6); // Send address
    for(byte i=cs*64;i<((cs+1)*64);i++)
      ht1632_writebits(ht1632_shadowram[i], 1<<3); // send 4 bits of data
    ht1632_chipfree(ht1632_cs[cs]); // done
  }
}

void setup ()  // flow chart from page 17 of datasheet
{
  pinMode(ht1632_wrclk, OUTPUT);
  pinMode(ht1632_data, OUTPUT);

  for(byte j=0;j<ht1632_displays;j++){
    pinMode(ht1632_cs[j], OUTPUT);
    digitalWrite(ht1632_cs[j], HIGH); 	/* unselect (active low) */
    ht1632_sendcmd(HT1632_CMD_SYSDIS,ht1632_cs[j]);  // Disable system
    ht1632_sendcmd(HT1632_CMD_COMS10,ht1632_cs[j]);  // 16*32, PMOS drivers
    ht1632_sendcmd(HT1632_CMD_MSTMD,ht1632_cs[j]); 	/* Master Mode */
    ht1632_sendcmd(HT1632_CMD_SYSON,ht1632_cs[j]); 	/* System on */
    ht1632_sendcmd(HT1632_CMD_LEDON,ht1632_cs[j]); 	/* LEDs on */
    ht1632_sendcmd(HT1632_CMD_PWM | 15 ,ht1632_cs[j]); /* pwm off */
    for (byte i=0; i<64; i++)
      ht1632_senddata(i, 15,ht1632_cs[j]);  // clear the display!
    delay(100);
    for (byte i=0; i<64; i++)
      ht1632_senddata(i, 0,ht1632_cs[j]);  // clear the display!
  }
  randomSeed(analogRead(0));
  
}

void plot (char x, char y, char val)
{
  char addr, bitval, offset;
  
  /*
   * The 4 bits in a single memory word go DOWN, with the LSB
   * (last transmitted) bit being on top.  However, writebits()
   * sends the LSB first, so we have to do a sort of bit-reversal
   * somewhere.  Here, this is done by shifting the single bit in
   * the opposite direction from what you might expect.
   */

//  offset = (y&B11111000)<<3; // compute offset (for y>7)
  offset = y>>3;
  bitval = 8>>(y&3);  // compute which bit will need set
  addr = (x<<1) + ((y&B00000111)>>2) + (offset<<6);  // compute which memory word this is in
  if (val) {  // Modify the shadow memory
    ht1632_shadowram[addr] |= bitval;
  }  else {
    ht1632_shadowram[addr] &= ~bitval;
  }
  // Now copy the new memory value to the display
  //for(byte j=0;j<ht1632_displays;j++){
    ht1632_senddata(addr, ht1632_shadowram[addr],ht1632_cs[offset]);
  //}
}

void showbitmap(char bitmap){
  byte i;
  for(i=0;i<128;i++){
    ht1632_shadowram[i]=bitmaps[bitmap][i];
  }
  ht1632_sendshadowram();
}

void game()
{
  // x & y scaled by 8
  
  int X=16*8;
  int Y=4*8;
  int Xo=X;
  int Yo=Y;
  int XRC=8;
  int YRC=2;
  byte p1s=0; //p1 score
  byte p2s=0; //p2 score
  byte i;
  signed char p1b=7;
  signed char p2b=56;
  byte bs=2;
  
  while(p1s<8 && p2s<8) {
    X+=XRC;
    Y=Yo+YRC;
  
    if(X>231){X=231; XRC=-XRC; YRC=random(0,16);}
    if(X<23) {
       if((p1b>Y-13) & (p1b<Y+13)){
         X=23;
         XRC=-XRC;
         YRC=(Y-p1b)>>1;
       } else {
         p2s++;
         delay(500);
         Y=32;
         X=128;
         XRC=8;
       }
    }
    if(Y>127) {Y=127;  YRC=-YRC;}
    if(Y<0) {Y=0;    YRC=-YRC;}
    
    plot(X>>3,Y>>3,1);
    if((X>>3!=Xo>>3) | (Y>>3!=Yo>>3)) plot(Xo>>3,Yo>>3,0);
    Xo=X;
    Yo=Y;

// plot score 
//    if(p1s>0) plot(0,p1s-1,1);
//    if(p2s>0) plot(31,8-p2s,1);
  
    if(random(0,2)==1){
      if(random(0,50)==1) p1s++;
      if(p1s>8){
        p1s=0;
        for(i=0;i<8;i++) {plot(0,i,0);}
      }
    } 
  
  
  
//    for(i=0;i<8;i++) {plot(2,i,0);}
   //    p1b=analogRead(1)>>6;
    
//    if(p1b>Y){
//      p1b+=bs;
//    }else{
//      p1b+=-bs;
//   }

//    if(p1b<8) {plot(2,0,1);plot(2,1,1);plot(2,2,1);p1b=7;}
//    else if(p1b>55) {plot(2,5,1);plot(2,6,1);plot(2,7,1);p1b=56;} 
//    else {plot(2,(p1b>>3)-1,1);plot(2,(p1b>>3),1);plot(2,(p1b>>3)+1,1);}
  
//   for(i=0;i<8;i++) {plot(29,i,0);}
//    if(p2b>Y){
//      p2b+=bs;
//    }else{
//      p2b+=-bs;
//    }
              //  p2b=Y;
 //   if(p2b<8) {plot(29,0,1);plot(29,1,1);plot(29,2,1); p2b=7;}
 //   else if(p2b>55) {plot(29,5,1);plot(29,6,1);plot(29,7,1);p2b=56;} 
 //   else {plot(29,(p2b>>3)-1,1);plot(29,(p2b>>3),1);plot(29,(p2b>>3)+1,1);}
    delay(100);
  }

  if(p1s==8){
    showbitmap(P1WIN);
  } else {
    showbitmap(P2WIN);
  }
  delay(1000);

}
void test(){
   plot(1,0,1);
   plot(1,1,1);
   plot(1,2,1);
   plot(1,3,1);
   plot(1,4,1);
   plot(1,5,1);
   plot(1,6,1);
   plot(1,7,1);
   plot(1,8,1);
   plot(1,9,1);
   plot(2,8,1);
   
}

void loop(){
  showbitmap(PONG);  
//  delay(500);
//  showbitmap(FIVE);  
//  delay(500);
//  showbitmap(FOUR);  
//  delay(500);
//  showbitmap(THREE);  
//  delay(500);
// showbitmap(TWO);  
//  delay(500);
//  showbitmap(ONE);  
//  delay(500);
//  showbitmap(CLS);
//  game();
//   test();
}
