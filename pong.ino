#include <EEPROM.h>
#include "ht1632.h"
#include "font.h" 

#define  WIN 0
#define  PONG  1
#define  CLS    2
/* 
 * Set these constants to the values of the pins connected to the SureElectronics Module
 */
static const byte ht1632_data = 3;     // Data pin (pin 7)
static const byte ht1632_wrclk = 4;    // Write clock pin (pin 5)
static const byte ht1632_cs[] = {5,6};       // Chip Select (1, 2, 3, or 4)
static const byte ht1632_displays = 2; // Number of displays

static const byte p1control = 0; 
static const byte p2control = 1;
static const byte button = 2;
static const byte speaker = 12;

unsigned long prevmillis = 0;

byte sbatsize=5;
byte ssound=1;
byte sballspeed=80;
byte sballspeedup=5;

static const byte bitmaps[][128] ={
                    {12,12,9,9,3,3,6,6,12,12,9,9,3,3,6,6,12,12,9,9,3,3,6,6,12,12,9,9,3,3,6,6,12,12,9,9,3,3,6,6,12,12,9,9,3,3,6,6,12,12,9,9,3,3,6,6,12,12,9,9,3,3,6,6,3,3,9,9,12,12,6,6,3,3,9,9,12,12,6,6,3,3,9,9,12,12,6,6,3,3,9,9,12,12,6,6,3,3,9,9,12,12,6,6,3,3,9,9,12,12,6,6,3,3,9,9,12,12,6,6,3,3,9,9,12,12,6,6},
                    {0,0,15,15,15,15,13,8,13,8,15,8,7,0,0,0,7,14,15,15,12,3,12,3,15,15,7,14,0,0,15,15,7,15,3,0,1,8,15,15,15,15,0,0,7,15,15,15,12,3,13,11,13,15,13,14,0,0,15,11,15,11,0,0,0,0,13,15,13,15,0,0,7,11,15,11,13,11,12,3,15,15,7,14,0,0,15,15,15,15,1,8,0,12,15,14,15,15,0,0,7,14,15,15,12,3,12,3,15,15,7,14,0,0,0,14,1,15,1,11,1,11,15,15,15,15,0,0},
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
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
  Serial.begin(115200);
  pinMode(button,INPUT_PULLUP);
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
//    for (byte i=0; i<64; i++)
//      ht1632_senddata(i, 15,ht1632_cs[j]);  // clear the display!
//    delay(10000);
    for (byte i=0; i<64; i++)
      ht1632_senddata(i, 0,ht1632_cs[j]);  // clear the display!
  }
  randomSeed(analogRead(3));
  sbatsize=EEPROM.read(0);
  ssound=EEPROM.read(1);
  sballspeed=EEPROM.read(2);
  sballspeedup=EEPROM.read(3);
  
  if(digitalRead(button)==0){
      gamesetup();
  }
}

void sound(int freq, int duration){
  if (ssound==1){
    tone(speaker,freq,duration);
  }
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

void bat(byte x, byte y, byte bs, byte value){
  for(bs; bs>0; bs--){
    plot(x,y+bs-1,value);
  }
}


int game() {
  // x & y scaled by 8 zodat er interger math gedaan kan worden voor de rc
  signed int X=16*8; // ball x 
  signed int Y=8*8;  // ball y 
  signed int Xo=X;   // ball x previous
  signed int Yo=Y;   // ball y previos
  signed int XRC;  // X direction 
  signed int YRC=0;  // y direction
  signed int p1b=300;  // player 1 bat position
  signed int p2b=300; // player 2 bat position
  signed int p1bo=400;  // player 1 bat old position
  signed int p2bo=400; // player 2 bat old position
  unsigned int ballspeed=(20-sballspeed)*10; // ball speed
  byte batsize=sbatsize;
  byte gamecountdown=1;
  byte countdown=7;

  if(random(2)==1){
    XRC=8;
  }else{
    XRC=-8;
  }
  YRC=random(-4,4);
  
   while(true) {
    // pre game beeping
    if(gamecountdown==1){
      if((millis()-prevmillis)>150){
        prevmillis=millis();
        if(countdown>0){
          if(countdown%2==1){
            plot(16,8,1);
            sound(1000,10);          
          } else {
            plot(16,8,0);
          }
          countdown--;
        }
      }
      if(countdown==0){
        sound(1400,20);
//      bat(16,0,16,1); 
        gamecountdown=0;
      }
    } else {

      // ball shizzle
      if ((millis()-prevmillis)>ballspeed){
        prevmillis=millis();
        X+=XRC;
        Y+=YRC;

        // p1 scores 
        if(X>255){
          sound(100,10);
          return(1);
        }

        // p2 scores 
        if(X<1){
          sound(100,10);
          return(2);
        }
  
        
        // bounce onderkant
        if(Y>127) {
          sound(500,1);
          Y=127;
          YRC=-YRC;
        }
        // bounce bovenkant
        if(Y<0) {
          sound(500,1);
          Y=0;
          YRC=-YRC;
        }
      
        // plot bal, en verwijder de oude
        plot(X>>3,Y>>3,1);
        if((X>>3!=Xo>>3) | (Y>>3!=Yo>>3)) {
          plot(Xo>>3,Yo>>3,0);
          Xo=X;
          Yo=Y;
        } 

        // p1 paddle check 
        if((X>>3) == 3) {
          if(((Y>>3)>=(p1b>>3)) & ((Y>>3)<((p1b>>3)+batsize))){
            sound(400,5);
             XRC=-XRC;
             if(ballspeed>0){
               ballspeed-=sballspeedup;
             }
             if(YRC>=0){
               YRC=map(Y-p1b,0,batsize<<3,0,16);
             } else {
               YRC=map(Y-p1b,0,batsize<<3,-16,0);
             }
          } 
        }
 
        // p2 paddle check 
        if((X>>3) == 28) {
          if(((Y>>3)>=(p2b>>3)) & ((Y>>3)<((p2b>>3)+batsize))){
            sound(400,5);
             XRC=-XRC;
             if(ballspeed>0){
               ballspeed-=sballspeedup;
            }  
             if(YRC>=0){
               YRC=map(Y-p2b,0,batsize<<3,0,16);
             } else {
               YRC=map(Y-p2b,0,batsize<<3,-16,0);
             }
          } 
        }
      }
    }
    // player 1 pad
    p1b=map(analogRead(p1control)>>1,0,512,0,128-((batsize-1)<<3));
    if((p1b>>3)!=(p1bo>>3)){
      bat(2,p1bo>>3,batsize,0);
      bat(2,p1b>>3,batsize,1);
      p1bo=p1b;
    }

    // player 2 pad
    p2b=map(analogRead(p2control)>>1,0,512,0,128-((batsize-1)<<3));
//    p2b=0;
    if((p2b>>3)!=(p2bo>>3)){
      bat(29,p2bo>>3,batsize,0);
      bat(29,p2b>>3,batsize,1);
      p2bo=p2b;
    }
  }
}

void p1winanim(byte loops, int dely ){
  for (loops ; loops>0 ; loops--){
    for (byte offset=0;offset<8;offset+=2){
      for (byte i=0;i<64;i++){
        ht1632_shadowram[i]=bitmaps[WIN][(i+offset)&63];
        ht1632_shadowram[i+64]=bitmaps[WIN][64+((i+offset)&63)];
      }
      ht1632_sendshadowram();
      delay(dely);
    }
  }
}

void p2winanim(byte loops, int dely){
  for (loops ; loops>0 ; loops--){
    for (byte offset=8;offset>0;offset-=2){
      for (byte i=0;i<64;i++){
        ht1632_shadowram[i+64]=bitmaps[WIN][(i+offset)&63];
        ht1632_shadowram[i]=bitmaps[WIN][64+((i+offset)&63)];
      }
      ht1632_sendshadowram();
      delay(dely);
    }
  }
}

void gamesetup(){
  char buffer[6];
  byte input;
  
  displaytext("Setup","");

  delay(2000);
  displaytext("Bat:","");
  while(digitalRead(button)==1){
    input=map(analogRead(p2control)>>1,0,512,1,16);
    snprintf(buffer,6,"%i",input);
    displaytext("",buffer);
  }
  sbatsize=input;
  
  delay(500);
  displaytext("Sound","");
  while(digitalRead(button)==1){
    input=map(analogRead(p2control)>>1,0,512,0,2);
    if(input==0){
      snprintf(buffer,6,"off");
    } else {
      snprintf(buffer,6,"on");
    }
    displaytext("",buffer);
  }
  ssound=input;
  
  delay(500);
  displaytext("Speed:","");
  while(digitalRead(button)==1){
    input=map(analogRead(p2control)>>1,0,512,20,1);
    snprintf(buffer,6,"%i",input);
    displaytext("",buffer);
  }
  sballspeed=input;

  delay(500);
  displaytext("Spdup:","");
  while(digitalRead(button)==1){
    input=map(analogRead(p2control)>>1,0,512,0,21);
    snprintf(buffer,6,"%i",input);
    displaytext("",buffer);
  }
  sballspeedup=input;

  delay(500);
  displaytext("Save","");
  while(digitalRead(button)==1){
    input=map(analogRead(p2control)>>1,0,512,0,2);
    if(input==0){
      snprintf(buffer,6,"no");
    } else {
      snprintf(buffer,6,"yes");
    }
    displaytext("",buffer);
  }
  if(input==1){
    displaytext("Saving","Data");
    EEPROM.write(0,sbatsize);
    EEPROM.write(1,ssound);
    EEPROM.write(2,sballspeed);
    EEPROM.write(3,sballspeedup);
    delay(500);
  }
}

void displaytext(char *upper, char *lower){
  int i=0;
  unsigned char lw=0;
  unsigned char offset=0;
  unsigned char j=0;
  unsigned char t=0;
  unsigned int index;
 
  if(strlen(upper)>0){
    for(i=0;i<64;i++){
      ht1632_shadowram[i]=0;
    } 
    for(i=0;i<strlen(upper);i++){
      index=(byte)upper[i]-32;
      lw=pgm_read_word_near(f_width+index);
      for(j=0;j<lw;j++){
        ht1632_shadowram[offset]=pgm_read_word_near(f_upper+index*8+j);
        ht1632_shadowram[offset+1]=pgm_read_word_near(f_lower+index*8+j);
        offset+=2;
      } 
    }
  }
  if(strlen(lower)>0){
    offset=64;
    for(i=64;i<128;i++){
      ht1632_shadowram[i]=0;
    } 
    for(i=0;i<strlen(lower);i++){
      index=(byte)lower[i]-32;
      lw=pgm_read_word_near(f_width+index);
      for(j=0;j<lw;j++){
        ht1632_shadowram[offset]=pgm_read_word_near(f_upper+index*8+j);
        ht1632_shadowram[offset+1]=pgm_read_word_near(f_lower+index*8+j);
        offset+=2;
      } 
    }
  }
  ht1632_sendshadowram();
}

void loop(){
  byte p1s;
  byte p2s;
  p1s=0;
  p2s=0;

  showbitmap(PONG);  
  while(digitalRead(button)==1){
    delay(10);
  }

  while(true){
    showbitmap(CLS);
    bat(0,0,p1s,1);
    bat(0,16-p1s,p1s,1);
    bat(31,0,p2s,1);
    bat(31,16-p2s,p2s,1);
   
    if(game()==1){
      p1s++;
    }else{
      p2s++;
    }
    if(p1s==9){
      p1winanim(10,100);
      break;
    }
    if(p2s==9){
      p2winanim(10,100);
      break;
    }
  }
  }
