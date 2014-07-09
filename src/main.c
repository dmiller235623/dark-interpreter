/*

 Oh, eternity with outstretched wings, that broodest over the secret
 truths in whose roots lie the mysteries of man—his whence, his
 whither—have I searched thee, and struck a right key on thy dreadful
 organ!

 [Thomas de Quincey. The Dark Interpreter] 

*/

/*
 PATH=~/sat/bin:$PATH
 PATH=~/stm32f4/stlink/flash:$PATH
 make stlink_flash
*/

#define MAX_EXE_STACK 4
#define VILLAGE_SIZE (STACK_SIZE*2) // 128

#ifdef LACH
#define SETSIZE 36
#define INFECTSIZE 740 
#define SETSHIFT 11
#define SHIFTY 7
#define THREADERR 32
#define VILLAGERR 33
#define POSERR 34
#define POSYERR 35
#else
#define SETSIZE 66
#define INFECTSIZE 770 
#define SETSHIFT 10
#define SHIFTY 6
#define THREADERR 62
#define VILLAGERR 63
#define POSERR 64
#define POSYERR 65
#endif

#ifdef PCSIM
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "audio.h"
#include "simulation.h"
#include "CPUint.h"
#include "CA.h"
#include "settings.h"
uint16_t *adc_buffer;
//typedef float float32_t;
u8 digfilterflag;
//int16_t src[BUFF_LEN], dst[BUFF_LEN];

int16_t *villager,*stacker,*stackery,*settingsarray;
int16_t *src, *dst;
u8 *village_effects;// [VILLAGE_SIZE/2];

void  dohardwareswitch(u8 one,u8 two){
  // nothing
}

void set40106pwm(u16 one){
}

void setmaximpwm(u16 one){
}

void setlmpwm(u16 one, u16 two){
}

#else
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "codec.h"
#include "i2s.h"
#include "adc.h"
#include "audio.h"
#include "hardware.h"
#include "simulation.h"
#include "CPUint.h"
#include "CA.h"
#include "settings.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

u16 villager[VILLAGE_SIZE];
u8 village_effects[STACK_SIZE]; // is half village_size
u16 stackery[STACK_SIZE*4]; // 64*4 MAX now
u16 stacker[STACK_SIZE*4]; // 64*4 MAX
u16 settingsarray[SETSIZE];
#endif

// for knobwork
// TENE: 2,0,3,4,1 // else: 3,0,2,4,1

#ifdef TENE
#define FIRST 2
#define SECOND 0
#define THIRD 3
#define FOURTH 4
#define FIFTH 1
#define UP 6
#define DOWN 8
#define LEFT 5
#define RIGHT 7
#else
#define FIRST 3
#define SECOND 0
#define THIRD 2
#define FOURTH 4
#define FIFTH 1
#define UP 5
#define DOWN 6
#define LEFT 8
#define RIGHT 7
#endif

u8 EFFECTWRITE;
u8 EFFECTREAD;
u8 EFFECTFILTER;

signed char direction[2]={-1,1};
u8 wormflag[10]={0,0,0,0,0,0,0,0,0,0};
//u8 setwalk[8]={239,240,1,17,16,15,254,238}; 
u8 inp;
u16 villagestackpos=0;
u16 *buf16;

#define delay()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#define delayxx()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#ifndef PCSIM
extern int16_t audio_buffer[AUDIO_BUFSZ];
u8* datagenbuffer = (u8*)0x10000000;
#define randi() (adc_buffer[9]) // 12 bits
#else //PCSIM
#define randi() (rand()%4096)
u8* datagenbuffer;
extern int16_t* audio_buffer;
#endif
extern u8 digfilterflag;

//u8 testdirection;
u8 wormdir; // worm direction
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array

u8 exestackpush(u8 exenum, u8* exestack, u8 exetype){
  if (exenum<MAX_EXE_STACK){
    exestack[exenum]=exetype;
    exenum++;
  }
  return exenum;
}

u8 exestackpop(u8 exenum){
  if (exenum>0){
    exenum--;
  }
  return exenum;
  }

u16 villagepush(u16 villagepos, u16 start, u16 wrap,u8 effect){
  if (villagepos<(VILLAGE_SIZE-1)) /// size -2
    {
      village_effects[villagepos/2]=effect;
      villager[villagepos++]=start;
      villager[villagepos++]=wrap;
}
  return villagepos;
}

u16 villagepop(u16 villagepos){
  if (villagepos>2)
    {
      villagepos-=2;
    }
  return villagepos;
}

u8 fingerdir(u8 *speedmod){
  u8 handleft, handright, up=0,down=0,left=0,right=0,upspeed=0,downspeed=0,leftspeed=0,rightspeed=0;
  u8 handupp, handdown;
  u8 result=5;

  for (u8 x=0;x<8;x++){
    handupp=adc_buffer[UP]>>8; // 4 bits=16 
    handdown=adc_buffer[DOWN]>>8;
    handleft=adc_buffer[LEFT]>>8;
    handright=adc_buffer[RIGHT]>>8;
    upspeed+=handupp;
    downspeed+=handdown;
    leftspeed+=handleft;
    rightspeed+=handright;

  if (handupp>4) up++;
  if (handdown>4) down++;
  if (handleft>4) left++;
  if (handright>4) right++;
  if (up>4 && up>down && up>left && up>right) {
    result=0; *speedmod=upspeed;
  }
  else if (down>4 && down>left && down>right) {
    result=2; *speedmod=downspeed;
  }
  else if (left>4 && left>right) {
    result=3; *speedmod=leftspeed;
  }
  else if (right>4) {
    result=1; *speedmod=rightspeed;
  }
  }
  return result;
}

u8 fingerdirr(void){

  u8 handleft, handright, up=0,down=0,left=0,right=0;
  u8 handupp, handdown;
  signed char result=0;

  for (u8 x=0;x<16;x++){
  handupp=adc_buffer[UP]>>8; 
  handdown=adc_buffer[DOWN]>>8;
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handupp>2) up++;
  if (handdown>2) down++;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (up>8 && up>down && up>left && up>right) {
    result=-8;
  }
  else if (down>8 && down>left && down>right) {
    result=+9; 
  }
  else if (left>8 && left>right) {
    result=-1;
  }
  else if (right>8) {
    result=1;
  }
  }
  return result;
}

u8 fingerdirleftright(void){

  u8 handleft, handright, left=0,right=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    return 0;
  }
  else if (right>8 && right>left) {
    return 1;
  }
  }
  return 2;
}

u8 fingerdirupdown(void){

  u8 handleft, handright, left=0,right=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;

  if (left>8 && left>right) {
    return 0;
  }
  else if (right>8 && right>left) {
    return 1;
  }
  }
  return 2;
}

signed char fingerdirupdownn(void){
  u8 handleft, handright, left=0,right=0;
  signed char result=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    result=-1;
  }
  else if (right>8 && right>left) {
    result=1;
  }
  }
  return result;
}

signed char fingerdirleftrightt(void){
  u8 handleft, handright, left=0,right=0;
  signed char result=0;

  for (u8 x=0;x<8;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>4 && left>right) {
    result=-1;
  }
  else if (right>4 && right>left) {
    result=1;
  }
  }
  return result;
}

signed char fingerdirleftrighttx(signed char vall){
  u8 handleft, handright, left=0,right=0;
  static signed char result=0; signed char tmp;

  for (u8 x=0;x<8;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>4 && left>right) {
    tmp=-1;
  }
  else if (right>4 && right>left) {
    tmp=1;
  }
  }
  //  return tmp;
  result+=tmp;
  if (result<=(-vall)) {
    result=0;
    tmp=-1;
  }
  else if (result>=vall) {
    result=0;
    tmp=1;
  }
  else tmp=0;
  return tmp;
}

signed char fingerdirupdownx(signed char vall){
  u8 handleft, handright, left=0,right=0;
  static signed char result=0; signed char tmp;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    tmp=-1;
  }
  else if (right>8 && right>left) {
    tmp=1;
  }
  }
  //  return tmp;
  result+=tmp;
  if (result<=(-vall)) {
    result=0;
    tmp=-1;
  }
  else if (result>=vall) {
    result=0;
    tmp=1;
  }
  else tmp=0;
  return tmp;
}

float32_t fingerdirleftrighttf(void){ 

  u8 handleft, handright, left=0,right=0;
  float32_t result=0.0f;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    result=-0.001f; 
  }
  else if (right>8 && right>left) {
    result=0.001f;
  }
  }
  return result;
}


u16 fingervalup16bits(u16 tmpsetting, u8 inc){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;

  for (x=0;x<16;x++){
  handup=adc_buffer[UP]>>8;
  handdown=adc_buffer[DOWN]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) tmpsetting+=inc;
  else if (ttss<sstt) tmpsetting-=inc;
  return tmpsetting;
}

u8 fingervalup(u8 tmpsetting){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;

  for (x=0;x<16;x++){
  handup=adc_buffer[UP]>>8;
  handdown=adc_buffer[DOWN]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) tmpsetting+=1;
  else if (ttss<sstt) tmpsetting-=1;
  return tmpsetting;
}

u8 fingervaleff(u8 tmpsetting){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;
  for (x=0;x<16;x++){
  handup=adc_buffer[RIGHT]>>8;
  handdown=adc_buffer[LEFT]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) {
    tmpsetting+=1;
    if (tmpsetting>=8) tmpsetting=1;
  }
  else if (ttss<sstt) {
tmpsetting-=1;
 if (tmpsetting==0) tmpsetting=7; // wraps
  }
  return tmpsetting;
}

u8 fingervalright(u8 tmpsetting, u8 wrap){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;
  for (x=0;x<16;x++){ 
    handup=adc_buffer[RIGHT]>>8; // 4bits=16
    handdown=adc_buffer[LEFT]>>8;
    if (handup>2) ttss++;// was 8 - 
    else if (handdown>2) sstt++;
  }
  if (ttss>sstt) {
    tmpsetting+=1;
    tmpsetting=tmpsetting%wrap;
  }
  else if (ttss<sstt) {
tmpsetting-=1;
 if (tmpsetting==255) tmpsetting=wrap-1;
  }
  return tmpsetting;
}

u8 fingervalupwrap(u8 tmpsetting, u8 wrap){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;
  for (x=0;x<16;x++){
  handup=adc_buffer[UP]>>8;
  handdown=adc_buffer[DOWN]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) {
    tmpsetting+=1;
    tmpsetting=tmpsetting%wrap;
  }
  else if (ttss<sstt) {
tmpsetting-=1;
 if (tmpsetting==255) tmpsetting=wrap-1;
  }
  return tmpsetting;
}

////////////////////////////////////////////////////////////////////////////////

void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 coo,x,addr,tmp=0,tmphw=0,tmphardware=0;u8 HARDWARE=0;
  u8 del=0,machine_count=0,tmpacht=0,villagerdest,spd; 
  u8 exestack[MAX_EXE_STACK];
  u16 tmper,foldy;

u8 stackerattached[256];
u8 stackeryattached[256];
u8 villagerattached[128];
u8 villagereffattached[64];
u8 cpuattached[64];

#ifdef PCSIM
u8 *settingsarrayattached; //64
u8 *settingsarrayinfected; //64
#else
u8 settingsarrayinfected[INFECTSIZE][2];
u8 settingsarrayattached[SETSIZE];
#endif

  inittable(3,4,randi());
  //  wormdir=0;
  const float32_t pi= 3.141592;
  float32_t w;
  float32_t yi;
  float32_t phase=0;
  int sign_samp,i;
  inp=0; // STRAIGHT IN
  w= 2*pi;
  w= w/256;
  for (i = 0; i <= 256; i++)
    {
      yi= 32767*sinf(phase); // was 2047???
      phase=phase+w;
      sign_samp=32767+yi;     // dc offset
      sin_data[i]=sign_samp; // write value into array
    }

#ifndef PCSIM
  //	ADC1_Initonce();
  ADC1_Init((uint16_t *)adc_buffer);

#ifndef LACH
  setup_switches();
#endif

  // maintain order
  Audio_Init();
  Codec_Init(48000);
  delay();

#ifndef LACH
  initpwm(); 	
#endif		

  I2S_Block_Init();
  I2S_Block_PlayRec((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);

#ifndef LACH
  dohardwareswitch(0,0);
#endif
#endif // for ifndef PCSIM

  machine *m=(machine *)malloc(sizeof(machine));
  m->m_threadcount=0;
  m->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS); //PROBLEM with _sbrk FIXED

#ifdef PCSIM
  datagenbuffer=(u8*)malloc(65536);
  audio_buffer=(int16_t*)malloc(32768*sizeof(int16_t));
  settingsarray=malloc(SETSIZE*sizeof(int16_t));
  villager=malloc(VILLAGE_SIZE*sizeof(int16_t));
  stacker=malloc(4*64*sizeof(int16_t));
  stackery=malloc(4*64*sizeof(int16_t));
  adc_buffer=malloc(10*sizeof(int16_t));
  initaudio();
  srandom(time(0));
  src=malloc(BUFF_LEN*sizeof(int16_t));
  dst=malloc(BUFF_LEN*sizeof(int16_t));
  village_effects=malloc(VILLAGE_SIZE/2);
  settingsarrayattached=malloc(SETSIZE);
  settingsarrayinfected=malloc(INFECTSIZE);

  for (x=0;x<(BUFF_LEN);x++){
    src[x]=rand()%65536;
    dst[x]=rand()%65536;
  }

  for (x=0;x<32768;x++){
    audio_buffer[x]=rand()%65536;
  }
#endif

  u8 hwdel=0;
  u16 hwpos=0;
  signed char stack_pos=0;
  signed char stack_posy=0;
  u16 start,wrap;
  u8 exenums=0, which=0,other=1;

  struct stackey stackyy[STACK_SIZE];
  struct stackeyyy stackyyy[STACK_SIZE];
  buf16 = (u16*) datagenbuffer;
  u8 leakiness=randi()%255;
  u8 infection=randi()%255;

  for (x=0;x<SETSIZE;x++){
    if ((rand()%255) > (adc_buffer[SECOND]>>4))
      settingsarrayinfected[x][0]=1; // infected
    else settingsarrayinfected[x][0]=0;
  }

  // fill datagenbuffer???

  for (x=0;x<32768;x++){
     buf16[x]=randi()<<4;
    delayxx();
  }

  for (x=0;x<SETSIZE;x++){
    settingsarrayattached[x]=0;
  }

  for (x=0;x<256;x++){
    stackerattached[x]=0;
    stackeryattached[x]=0;
  }

  for (x=0;x<128;x++){
    villagerattached[x]=0;
  }

  for (x=0;x<64;x++){
    villagereffattached[x]=0;
  }

  //****** setup code for walkers
#ifdef LACH
  for (x=0;x<6;x++){
    settingsarray[x]=0;
  }//start

  for (x=6;x<12;x++){
    settingsarray[x]=65535;
  }//wrap

  for (x=12;x<20;x++){
    settingsarray[x]=511; //>>8 now >>12 but we add 1 so???
  }//step

  EFFECTWRITE=0;
  EFFECTREAD=0;

  for (x=20;x<23;x++){
    settingsarray[x]=32768;
  }//fmods

  for (x=23;x<25;x++){
    settingsarray[x]=0; 
  }//expand

  for (x=25;x<32;x++){
    settingsarray[x]=32768;//>>15 = 1
  }//DIR and speed

  // initialise foldoffset and foldtop
  settingsarray[23]=32768;
  settingsarray[24]=0;

  // initialise exestacks

  settingsarray[32]=65535;//to 63
  settingsarray[33]=65535;
  settingsarray[34]=65535;
  settingsarray[35]=65535;

#else
  for (x=0;x<14;x++){
    settingsarray[x]=0;
  }//start

  for (x=14;x<28;x++){
    settingsarray[x]=65535;
  }//wrap


  // new hardware offsets
  for (x=28;x<32;x++){
    settingsarray[x]=2;
  }

  for (x=32;x<42;x++){
    settingsarray[x]=511; //>>8
  }//step

  for (x=42;x<46;x++){
    settingsarray[x]=511;
  }//speed

  EFFECTWRITE=0;
  EFFECTREAD=0;
  EFFECTFILTER=0;

  for (x=46;x<49;x++){
    settingsarray[x]=32768;
  }//fmods

  // initialise foldoffset and foldtop and machinespeed
  settingsarray[49]=65535;
  settingsarray[50]=0;
  settingsarray[51]=32768;

  for (x=52;x<62;x++){
    settingsarray[x]=32768;//>>15 = 1
  }//DIR

  // initialise exestacks

  settingsarray[62]=65535;//to 63
  settingsarray[63]=65535;
  settingsarray[64]=65535;
  settingsarray[65]=65535;


#endif //nolACH

  // CPUintrev3:
  for (x=0; x<64; x++) // was 100
    {
      addr=randi()<<3;
      cpustackpush(m,addr,randi()<<4,randi()%CPU_TOTAL,randi()%24); // was <<3
    }

  //pureleak

  for (x=0;x<100;x++){
    addr=randi()<<3;
    cpustackpushhh(datagenbuffer,addr,randi()<<4,randi()%CPU_TOTAL,randi()%24);// was <<3
  }

  // CA
  for (x=0;x<(STACK_SIZE);x++){
          start=randi()<<4;
          wrap=randi()<4;
	  stack_posy=ca_pushn(stackyyy,randi()<<4,datagenbuffer,stack_posy,randi()<<4,start,wrap); 
  }

  //simulationforstack:	
    for (x=0;x<STACK_SIZE;x++){
                start=randi()<<4;
            wrap=randi()<<4;
      stack_pos=func_pushn(stackyy,randi()<<4,buf16,stack_pos,randi()<<4,start,wrap);
            villagestackpos=villagepush(villagestackpos,start,wrap,randi()%16);
  }

    // execution stack
        for (x=0;x<MAX_EXE_STACK;x++){
	  exenums=exestackpush(exenums,exestack,randi()%5); //exetype=0-3 
	  //	  exenums=exestackpush(exenums,exestack,2); //exetype=0-3 TESTY!
      }

	u8 mainmode,minormode,groupstart,groupwrap;
	u8 xx,dirpos,groupsel,groupstartt,wormstart,wormpos,foldposy,foldpos;
	u16 foldposl,settingsposl,fingerposl;

	    m->m_leakiness=leakiness;
	    m->m_infectprob=infection;
	    m->m_memory=datagenbuffer;

  while(1)
    {

#ifdef TEST_STRAIGHT
      // do nothing
#else

#ifdef TEST_EEG
      //write ADC9 into buf16
  for (x=0;x<32768;x++){
    buf16[x]=randi()<<4; // 16 bits
  }
#else

#ifdef PCSIM
      // randomise adc_buffer
      for (x=0;x<10;x++){
	adc_buffer[x]+=(randi()%4096);
	adc_buffer[x]=adc_buffer[x]%4096;
      }

      I2S_RX_CallBack(src, dst, BUFF_LEN/2); 
#endif
      
      for (x=0;x<exenums;x++){
	switch(exestack[x]){
	case 0:
	  func_runall(stackyy,STACKPOS); // simulations
	  break;
	case 1:
	  ca_runall(stackyyy,STACKPOSY); // CA
	  break;
	case 2:
	  machine_count++;
	  if (machine_count>=MACHINESPEED){
	    machine_run(m); //cpu
	    machine_count=0;
	  }
	  break;
	case 3:
	    machine_runnn(datagenbuffer); // pureleak
	  break;
	case 4:
	  break;
	}
      }

      /////////////////////////////////////
	

#ifdef LACH
      settingsarray[6]=adc_buffer[FIFTH]<<4; // 16 bits SAMPLEWRAP!!!
      settingsarrayattached[6]=0;
#endif

      for (x=0;x<SETSIZE;x++){
	switch(settingsarrayattached[x]){
	case 0:
	  break;
	case 1:
	  settingsarray[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768];
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  settingsarray[x]=adc_buffer[9]<<4;
#else
	  settingsarray[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  settingsarray[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  settingsarray[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<STACKPOS;x++){
	switch(stackerattached[x]){
	case 0:
	  break;
	case 1:
	  stacker[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768];
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  stacker[x]=adc_buffer[9]<<4;
#else
	  stacker[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  stacker[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  stacker[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<STACKPOSY;x++){
	switch(stackeryattached[x]){
	case 0:
	  break;
	case 1:
	  stackery[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768];
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  stackery[x]=adc_buffer[9]<<4;
#else
	  stackery[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  stackery[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  stackery[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<VILLAGESTACKPOS;x++){
	switch(villagerattached[x]){
	case 0:
	  break;
	case 1:
	  villager[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768];
	  village_effects[x/2]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768]>>12;
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  villager[x]=adc_buffer[9]<<4;
	  village_effects[x/2]=adc_buffer[9]>>8;
#else
	  villager[x]=adc_buffer[DOWN]<<4;
	  village_effects[x/2]=adc_buffer[DOWN]>>8;
#endif
	  break;
	case 3:
	  villager[x]=adc_buffer[THIRD]<<4;
	  village_effects[x/2]=adc_buffer[THIRD]>>8;
	  break;
	case 4:
	  villager[x]=adc_buffer[SECOND]<<4; // where?
	  village_effects[x/2]=adc_buffer[SECOND]>>8;
	  break;
	}
      }

      for (x=0;x<THREADCOUNT;x++){
	switch(cpuattached[x]){
	case 0:
	  break;
	case 1:
	  m->m_threads[x].m_CPU=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>10)+1)))%32768]>>11;
	  coo++;
	  break;
	case 2:
#ifdef TENE
	  m->m_threads[x].m_CPU=adc_buffer[9]>>7;//cpu - 5 bits
#else
	  m->m_threads[x].m_CPU=adc_buffer[DOWN]>>7;//cpu - 5 bits

#endif
	  break;
	case 3:
	  m->m_threads[x].m_CPU=adc_buffer[THIRD]>>7;
	  break;
	case 4: // never used...
	  m->m_threads[x].m_CPU=adc_buffer[SECOND]>>7; // where?
	  break;
	}
      }
      
      //MODECODE      /////////////////////////////////////

      mainmode=adc_buffer[FIRST]>>8; // 4 bits=16
      //      mainmode=15; // TESTY!

      switch(mainmode){
#ifdef LACH //  TESTY!
      case 0:
	xx=fingerdir(&spd);
	if (xx!=5) {
	EFFECTWRITE=adc_buffer[FOURTH]>>6;
	EFFECTREAD=adc_buffer[SECOND]>>6;
	// what spd could be? mod? (max 64=6bits<<10) 3 mods:
	settingsarray[20+xx]=spd<<10;
	settingsarrayattached[20+xx]=0;
	}
	break;
#else
      case 0: // up/down/left/right as INPUT
	xx=fingerdir(&spd);
	if (xx!=5) {
	  inp=xx;
	  EFFECTWRITE=adc_buffer[FOURTH]>>6;
	  EFFECTREAD=adc_buffer[SECOND]>>6;
	  EFFECTFILTER=adc_buffer[THIRD]>>6;
	// what spd could be? mod? (max 64=6bits<<10) 3 mods:
	settingsarray[46+xx]=spd<<10;
	settingsarrayattached[46+xx]=0;
	}
	break;
#endif	
      case 1:
	//	  select stackmax (left/right) (knob and set)
	xx=fingerdir(&spd);
	if (xx==0) {
	  settingsarray[THREADERR]=adc_buffer[SECOND]<<4;
	  settingsarrayattached[THREADERR]=0;
	  m->m_threads[THREADCOUNT].m_CPU=spd>>1;
	  //	  VILLAGESTACKPOS=(adc_buffer[FOURTH]>>6)*2; // 6bits
	  settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	  settingsarrayattached[VILLAGERR]=0;
	}
	  else if (xx==1) 
	    {
	      //	      stack_posy=adc_buffer[SECOND]>>6; // 6bits
	  settingsarray[POSYERR]=adc_buffer[SECOND]<<4;
	  settingsarrayattached[POSYERR]=0;
	  stackery[(STACKPOSY*4)+3]=spd%11;
	      //	      VILLAGESTACKPOS=(adc_buffer[FOURTH]>>6)*2; // 6bits
	      settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	      settingsarrayattached[VILLAGERR]=0;
	    }
	  else if (xx==2) {
	    //	    stack_pos=adc_buffer[SECOND]>>6; // 6bits
	  settingsarray[POSERR]=adc_buffer[SECOND]<<4;
	  settingsarrayattached[POSERR]=0;
	  stacker[(STACKPOS*4)+3]=spd%34;
	    //	    VILLAGESTACKPOS=(adc_buffer[FOURTH]>>6)*2; // 6bits
	  settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	  settingsarrayattached[VILLAGERR]=0;
	  }
	  else if (xx==3)
	    {
	      exestack[adc_buffer[SECOND]>>10]=spd%4;
	      //	      VILLAGESTACKPOS=(adc_buffer[FOURTH]>>6)*2; // 6bits
	      settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	      settingsarrayattached[VILLAGERR]=0;
	    }
	break;
	  ///////////////////////////////////
      case 2: // directions - and speed
	dirpos=adc_buffer[SECOND]>>8; // 4 bits
#ifdef LACH
	dirpos=dirpos%6;
	//	dirpos=3;
	xx=fingerdir(&spd);
	// sET Speed!
	if (xx==0) {
	  wormflag[dirpos]=1; // up
	  if (dirpos==2) {
	    settingsarray[19]=spd<<9;
	    settingsarrayattached[19]=0;
	  }
	  else if (dirpos==3) {
	    settingsarray[18]=spd<<9;
	    settingsarrayattached[18]=0;
	  }
	}
	else if (xx==1) { 
	  settingsarray[25+dirpos]=(1<<15); wormflag[dirpos]=0;
	  settingsarrayattached[25+dirpos]=0;
	  if (dirpos==2) {
	    settingsarray[19]=spd<<9;
	    settingsarrayattached[19]=0;
	  }
	  else if (dirpos==3) {
	    settingsarray[18]=spd<<9;
	    settingsarrayattached[18]=0;
	  } 
	}//right
	else if (xx==3) {//left 
	  settingsarray[25+dirpos]=0; wormflag[dirpos]=0;
	  settingsarrayattached[25+dirpos]=0;
	  if (dirpos==2) {
	    settingsarray[19]=spd<<9;
	    settingsarrayattached[19]=0;
	  }
	  else if (dirpos==3) {
	    settingsarray[18]=spd<<9;
	    settingsarrayattached[18]=0;
	  }
	} // left=0
	else if (xx==2) { //down
	  settingsarray[25+dirpos]=adc_buffer[DOWN]<<4; wormflag[dirpos]=0;
	    settingsarrayattached[25+dirpos]=0;
	    if (dirpos==2) {
	      settingsarray[19]=spd<<9;
	      settingsarrayattached[19]=0;
	    }
	    else if (dirpos==3) {
	      settingsarray[18]=spd<<9;
	      settingsarrayattached[18]=0;
	    }
	}
#else
	dirpos=dirpos%10;
	//	dirpos=0; // testy!!!
	xx=fingerdir(&spd);
	if (xx==0) {
	  wormflag[dirpos]=1; // up
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	}
	else if (xx==1) { //right
	  settingsarray[52+dirpos]=(1<<15); wormflag[dirpos]=0;
	  settingsarrayattached[54+dirpos]=0;
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	} //right
	else if (xx==3) { //left
	  settingsarray[52+dirpos]=0; wormflag[dirpos]=0;
	  settingsarrayattached[54+dirpos]=0;
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	} // left=0
	else if (xx==2) { //down
	  settingsarray[52+dirpos]=adc_buffer[DOWN]<<4; wormflag[dirpos]=0;
	  settingsarrayattached[54+dirpos]=0;
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	}
#endif
	break; 

      case 3:
	// set start and wrap for r/w/village_r/w according to fingers
	// speed is step
	// 2 knobs start and wrap
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx==0){ // UP=read= start/wrap/step
	  settingsarray[1]=tmp;
	  settingsarray[7]=tmper;
	  settingsarray[13]=spd<<8;
	  settingsarrayattached[1]=0;
	  settingsarrayattached[7]=0;
	  settingsarrayattached[13]=0;
	}
	else if (xx==2){ // DOWN=write= start/speed/step
	  settingsarray[0]=tmp;
	  settingsarray[18]=tmper;  // changed as is already set // now is speed
	  settingsarray[12]=spd<<8; // step!!!
	  settingsarrayattached[0]=0;
	  settingsarrayattached[18]=0;
	  settingsarrayattached[12]=0;
	}
	else if (xx==3){ // LEFT=village_r= start/wrap/step
	  settingsarray[5]=tmp;
	  settingsarray[11]=tmper;
	  settingsarray[17]=spd<<8;
	  settingsarrayattached[5]=0;
	  settingsarrayattached[11]=0;
	  settingsarrayattached[17]=0;
	}
	else if (xx==1){ // RIGHT=village_w= start/wrap/step
	  settingsarray[4]=tmp;
	  settingsarray[10]=tmper;
	  settingsarray[16]=spd<<8;
	  settingsarrayattached[4]=0;
	  settingsarrayattached[10]=0;
	  settingsarrayattached[16]=0;
	}
#else
	if (xx==0){ // UP=read= start/wrap/step
	  settingsarray[2]=tmp;
	  settingsarray[16]=tmper;
	  settingsarray[34]=spd<<8;
	  settingsarrayattached[2]=0;
	  settingsarrayattached[16]=0;
	  settingsarrayattached[34]=0;
	}
	else if (xx==2){ // DOWN=write= start/wrap/step
	  settingsarray[1]=tmp;
	  settingsarray[15]=tmper;
	  settingsarray[33]=spd<<8;
	  settingsarrayattached[1]=0;
	  settingsarrayattached[15]=0;
	  settingsarrayattached[33]=0;
	}
	else if (xx==3){ // LEFT=village_r= start/wrap/step
	  settingsarray[8]=tmp;
	  settingsarray[22]=tmper;
	  settingsarray[40]=spd<<8;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[22]=0;
	  settingsarrayattached[40]=0;
	}
	else if (xx==1){ // RIGHT=village_w= start/wrap/step
	  settingsarray[7]=tmp;
	  settingsarray[21]=tmper;
	  settingsarray[39]=spd<<8;
	  settingsarrayattached[7]=0;
	  settingsarrayattached[21]=0;
	  settingsarrayattached[39]=0;
	}
#endif	
	break;
      case 4:
	// HW=start,wrap,offsets
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	xx=fingerdir(&spd);
#ifdef LACH
	// 4 sets of settings:
	// ANYSTART/ANYWRAP/ANYSTEP/VILLAGESTEP *2
	if (xx==0){
	  settingsarray[2]=tmp;
	  settingsarray[3]=tmper;
	  settingsarrayattached[2]=0;
	  settingsarrayattached[3]=0;
	}
	else if (xx==1){
	  settingsarray[8]=tmp;
	  settingsarray[9]=tmper;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[9]=0;
	}
	else if (xx==2){
	  settingsarray[14]=tmp;
	  settingsarray[15]=tmper;
	  settingsarrayattached[14]=0;
	  settingsarrayattached[15]=0;
	}
	else if (xx==3){
	  settingsarray[16]=tmp;
	  settingsarray[17]=tmper;
	  settingsarrayattached[16]=0;
	  settingsarrayattached[17]=0;
	}
#else
	//HW start and wrap
	if (xx==0){ // UP=generic HW start/wrap/hdgenercons
	  settingsarray[0]=tmp;
	  settingsarray[14]=tmper;
	  settingsarray[24]=spd<<10;
	  settingsarrayattached[0]=0;
	  settingsarrayattached[14]=0;
	  settingsarrayattached[24]=0;
	}
	else 	if (xx==2){ //DOWN=lmer1AND2---offset/cons/base 
	  settingsarray[28]=tmp;
	  settingsarray[29]=adc_buffer[THIRD]<<4;
	  settingsarray[25]=tmper;
	  settingsarray[10]=spd<<12;
	  settingsarrayattached[28]=0;
	  settingsarrayattached[29]=0;
	  settingsarrayattached[25]=0;
	  settingsarrayattached[10]=0;
	}
	else 	if (xx==3){ //LEFT=1016er-offset/cons/base
	  settingsarray[30]=tmp;
	  settingsarray[26]=tmper;
	  settingsarray[12]=spd<<11;
	  settingsarrayattached[30]=0;
	  settingsarrayattached[26]=0;
	  settingsarrayattached[12]=0;
	}
	else 	if (xx==1){ //RIGHT=maximer
	  settingsarray[31]=tmp; //offset
	  settingsarray[27]=tmper; //cons
	  settingsarray[13]=spd<<12; //base
	  settingsarrayattached[31]=0;
	  settingsarrayattached[27]=0;
	  settingsarrayattached[13]=0;
	}
#endif
	break;
      case 5:
	// anydata stuff
	//for LACH is simply anystart*2,anywrap*2,anystep*2
	//for TENE is *3 (inc filt)
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx==0){ // UP
	  settingsarray[3]=tmp;//anystartread
	  settingsarray[8]=tmper;//wrap
	  settingsarray[15]=spd<<10;//step
	  settingsarrayattached[3]=0;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[15]=0;
	}
	else 	if (xx==2){ //DOWN
	  settingsarray[2]=tmp;//write
	  settingsarray[9]=tmper;
	  settingsarray[14]=spd<<10;
	  settingsarrayattached[2]=0;
	  settingsarrayattached[9]=0;
	  settingsarrayattached[14]=0;
	}
	//#define FOLDTOP (settingsarray[23]) 
	//#define FOLDOFFSET (settingsarray[24]) 
	else if (xx==3){
	  settingsarray[23]=tmp;
	  settingsarray[24]=tmper;
	  settingsarrayattached[23]=0;
	  settingsarrayattached[24]=0;
	}
	// one setting missing TODO
#else
	if (xx==0){ // UP
	  settingsarray[5]=tmp;//anystartread
	  settingsarray[19]=tmper;//wrap
	  settingsarray[37]=spd<<10;//step
	  settingsarrayattached[5]=0;
	  settingsarrayattached[19]=0;
	  settingsarrayattached[37]=0;
	}
	else 	if (xx==2){ //DOWN
	  settingsarray[4]=tmp;//write
	  settingsarray[18]=tmper;
	  settingsarray[36]=spd<<10;
	  settingsarrayattached[4]=0;
	  settingsarrayattached[18]=0;
	  settingsarrayattached[36]=0;
	}
	else 	if (xx==1){ //RIGHT
	  settingsarray[6]=tmp;
	  settingsarray[20]=tmper;
	  settingsarray[38]=spd<<10;
	  settingsarrayattached[6]=0;
	  settingsarrayattached[20]=0;
	  settingsarrayattached[38]=0;
	}
	else if (xx==3){//is=folderSETTTINGS
	  settingsarray[49]=tmp;
	  settingsarray[50]=tmper;
	  settingsarrayattached[49]=0;
	  settingsarrayattached[50]=0;
	}
#endif
	break;
	/////////////////////////////////////////////////////////////////////////////////////////
	////setted as 2 modes now
      case 6: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupwrap=adc_buffer[SECOND]>>SHIFTY; // 6bits=64
	groupstart=adc_buffer[FOURTH]>>SHIFTY;
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	for (x=0;x<groupwrap;x++){
	  settingsarrayattached[(groupstart+x)%SETSIZE]=groupsel;
	}
	}
	break;

      case 7: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupwrap=adc_buffer[SECOND]>>5; // 7bits=128
	groupstart=adc_buffer[FOURTH]>>5;
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	for (x=0;x<groupwrap;x++){
	  villagerattached[(groupstart+x)%VILLAGE_SIZE]=groupsel;
	  villagereffattached[((groupstart+x)/2)%64]=groupsel;
	}
	}
	break;

      case 8:
	groupwrap=adc_buffer[SECOND]>>4; // 8bits
	groupstart=adc_buffer[FOURTH]>>4;
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	for (x=0;x<groupwrap;x++){
	  stackerattached[(groupstart+x)%256]=groupsel;
	}
	}
	break;

      case 9: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupwrap=adc_buffer[SECOND]>>4; // 8bits
	groupstart=adc_buffer[FOURTH]>>4;
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	for (x=0;x<groupwrap;x++){
	  stackeryattached[(groupstart+x)%256]=groupsel;
	}
	}
	break;

      case 10: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupwrap=adc_buffer[SECOND]>>6; // 6bits
	groupstart=adc_buffer[FOURTH]>>6;
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	for (x=0;x<groupwrap;x++){
	  cpuattached[(groupstart+x)%SETSIZE]=groupsel;
	}
	}
	break;

	///////////////////////////////
	//algo-attach into all:
      case 11:
	foldy=adc_buffer[SECOND]>>7; // how many from knob2 =max 32

	for (x=0;x<foldy;x++){
	  settingsposl=adc_buffer[FOURTH]<<3; // 15 bits
	  groupsel=fingerdir(&spd);
	if (groupsel!=5){
	  tmper=buf16[(settingsposl+x)%32768];
#ifdef LACH
	tmper=(tmper>>6)%804;
	if (tmper<36) settingsarrayattached[tmper]=groupsel;
	else 	if (tmper<292) stackerattached[tmper-36]=groupsel; // now 256
	else 	if (tmper<548) stackeryattached[tmper-292]=groupsel; // now 256
	else 	if (tmper<676) villagerattached[tmper-548]=groupsel; // 128
	else 	if (tmper<740) villagereffattached[tmper-676]=groupsel; // 64
	else 	cpuattached[tmper-740]=groupsel; //64
#else
	tmper=(tmper>>6)%834;
	if (tmper<66) settingsarrayattached[tmper]=groupsel;
	else 	if (tmper<322) stackerattached[tmper-66]=groupsel;
	else 	if (tmper<578) stackeryattached[tmper-322]=groupsel;
	else 	if (tmper<706) villagerattached[tmper-578]=groupsel;
	else 	if (tmper<770) villagereffattached[tmper-706]=groupsel;
	else 	cpuattached[tmper-770]=groupsel;
#endif
	}
	}
	break;

	///////////////////////////////////////////
	// swops now 
	// settingsarray<->settingsarray
	// villager<->villager
	// stacker/y<->villager and vice versa

      case 12:
	groupsel=fingerdir(&spd);
	switch(groupsel){
	case 0://UP
	foldy=adc_buffer[SECOND]>>SHIFTY; // howmuch-64
	foldposy=adc_buffer[THIRD]>>SHIFTY; // offset
	foldpos=adc_buffer[FOURTH]>>SHIFTY;
	for (x=0;x<(foldy);x++){
	  settingsarray[(foldpos+x)%SETSIZE]=settingsarray[(foldpos+foldposy+x)%SETSIZE];
	}
	break;
	case 1: // RIGHT
	foldy=adc_buffer[SECOND]>>5; // howmuch=128
	foldposy=adc_buffer[THIRD]>>5; // offset
	foldpos=adc_buffer[FOURTH]>>5;//start

	for (x=0;x<foldy;x++){
	  villager[(foldpos+x)%VILLAGE_SIZE]=villager[(foldpos+foldposy+x)%VILLAGE_SIZE];
	}
	break;

	case 2: //DOWN// various stack and villager exchanges - 
	// starts and ends only of stacks (not CPU) -> villagers
	
	foldy=adc_buffer[SECOND]>>6; // howmuch-64
	foldposy=adc_buffer[THIRD]>>6; // offset-64
	foldpos=adc_buffer[FOURTH]>>5;//128
	///////
	for (x=0;x<foldy;x++){ // 64 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point 
	  villagerdest=((foldpos>>1)+x+foldposy)%(STACK_SIZE); // village entry
	  if (tmper<STACK_SIZE){
	    // deal with stacker
	    villager[villagerdest*2]=stacker[tmper*4];  
	    villager[(villagerdest*2)+1]=stacker[(tmper*4)+1];  
	  }
	  else {
	    tmper-=STACK_SIZE;
	    // deal with stackery
	    villager[villagerdest*2]=stackery[tmper*4];  
	    villager[(villagerdest*2)+1]=stackery[(tmper*4)+1];  
	  }
	}
	break;

	case 3://LEFT // various stack and villager exchanges - 
	// other way round
	
	foldy=adc_buffer[SECOND]>>6; // howmuch-64
	foldposy=adc_buffer[THIRD]>>6; // offset
	foldpos=adc_buffer[FOURTH]>>5;

	for (x=0;x<(foldy);x++){ 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point // 7 bits=128
	  villagerdest=((foldpos>>1)+x+foldposy)%(STACK_SIZE); // village entry

	  if (tmper<STACK_SIZE){
	    // deal with stacker
	    stacker[tmper*4]=villager[villagerdest*2];  
	    stacker[(tmper*4)+1]=villager[(villagerdest*2)+1];  
	  }
	  else {
	    tmper-=STACK_SIZE;
	    // deal with stackery
	    stackery[tmper*4]=villager[villagerdest*2];  
	    stackery[(tmper*4)+1]=villager[(villagerdest*2)+1];  
	  }
	}
	break;
	}
	break;

      case 13: // dump (all) to datagen//back
	foldy=adc_buffer[SECOND]>>2; // howmuch=10 bits=1024
	foldpos=adc_buffer[FOURTH]<<4; // 16 bits

	xx=fingerdirupdown();
	if (xx==1) { //down

	for (x=0;x<foldy;x++){ // 10 bits
#ifdef LACH
	  tmper=((foldpos>>6)+x)%740; // full house//10 bits=1024
	  if (tmper<36) buf16[(foldpos+x)%32768]=settingsarray[tmper];
	  else if (tmper<292) buf16[(foldpos+x)%32768]=stacker[tmper-36];
	  else if (tmper<548) buf16[(foldpos+x)%32768]=stackery[tmper-292];
	  else if (tmper<612) buf16[(foldpos+x)%32768]=m->m_threads[tmper-548].m_CPU<<11;
	    else buf16[(foldpos+x)%32768]=villager[tmper-612];
#else
	  tmper=((foldpos>>6)+x)%770; // full house//10 bits=1024
	  if (tmper<66) buf16[(foldpos+x)%32768]=settingsarray[tmper];
	  else if (tmper<322) buf16[(foldpos+x)%32768]=stacker[tmper-66];
	  else if (tmper<578) buf16[(foldpos+x)%32768]=stackery[tmper-322];
	  else if (tmper<642) buf16[(foldpos+x)%32768]=m->m_threads[tmper-578].m_CPU<<11;
	    else buf16[(foldpos+x)%32768]=villager[tmper-642];
#endif
	}
	}
	else if (xx==0) {
	for (x=0;x<foldy;x++){ // 10 bits
#ifdef LACH
	  tmper=((foldpos>>6)+x)%740; // full house//10 bits=1024
	  if (tmper<36) settingsarray[tmper]=buf16[(foldpos+x)%32768];
	  else if (tmper<292) stacker[tmper-32]=buf16[(foldpos+x)%32768];
	  else if (tmper<548) stackery[tmper-288]=buf16[(foldpos+x)%32768];
	  else if (tmper<612) m->m_threads[tmper-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	    else villager[tmper-612]=buf16[(foldpos+x)%32768];
#else
	  tmper=((foldpos>>6)+x)%770; // full house//10 bits=1024
	  if (tmper<66) settingsarray[tmper]=buf16[(foldpos+x)%32768];
	  else if (tmper<322) stacker[tmper-66]=buf16[(foldpos+x)%32768];
	  else if (tmper<578) stackery[tmper-322]=buf16[(foldpos+x)%32768];
	  else if (tmper<642) m->m_threads[tmper-578].m_CPU=buf16[(foldpos+x)%32768]>>11;
	    else villager[tmper-642]=buf16[(foldpos+x)%32768];
#endif
	}
	}
	break;
	////////////////
      case 14: // infection
	///infection across buffer: knobs; speed,probability,buffer
	//set according to probability

      xx=fingerdir(&spd);

      if (xx!=5) {

	tmp=adc_buffer[SECOND]>>2; // 10 bits//offset
	tmper=adc_buffer[FOURTH]>>2;//amount

	if ((adc_buffer[FOURTH]>>5)==0){
	  for (x=0;x<(tmper%INFECTSIZE);x++){
	    if ((rand()%255) > (spd<<2)) settingsarrayinfected[(tmp+x)%INFECTSIZE][which]=1; // infected
	  else settingsarrayinfected[(tmp+x)%INFECTSIZE][which]=0;
	  } // reset!
	}
	  // run infection at speed SECOND 
	  else {
	    //	  for (x=0;x<INFECTSIZE;x++){
	  for (i=0;i<(tmper%INFECTSIZE);i++){
	    // infection - how many infected (not dead) round each one?
	    x=(i+tmp)%INFECTSIZE;
	    if (++del==spd){ // speed
	      tmpacht=(x-1)%INFECTSIZE;
	      if (settingsarrayinfected[x][which]==0 && ((settingsarrayinfected[tmpacht][which]>=1 && settingsarrayinfected[tmpacht][which]<128) && (settingsarrayinfected[(x+1)%INFECTSIZE][which]>=1 && settingsarrayinfected[(x+1)%INFECTSIZE][which]<128)) && (rand()%255) > (adc_buffer[THIRD]>>4)) settingsarrayinfected[x][other]=1;
	    // inc
	      else if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128) {
		  settingsarrayinfected[x][other]++;
		}
	    del=0;
	    }

#ifdef LACH
	    if (x<36) {
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)	settingsarray[x]-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) settingsarray[x]+=settingsarrayinfected[x][which];
	    }
	    else if (x<292) {
	      //	      stacker[x-32]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)	stacker[x-36]-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) stacker[x-36]+=settingsarrayinfected[x][which];;

	    }
	    else if (x<548) {
	      //	      stackery[x-288]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)	stackery[x-292]-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) stackery[x-292]+=settingsarrayinfected[x][which];;
	    }
	    else if (x<612) {
	      //	      m->m_threads[x-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)	m->m_threads[x-548].m_CPU-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) m->m_threads[x-548].m_CPU+=settingsarrayinfected[x][which];;
	    }
	    else {
	      //	      villager[x-608]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)  villager[x-612]-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) villager[x-612]+=settingsarrayinfected[x][which];;
	    }
#else
	    if (x<66) {
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)	settingsarray[x]-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) settingsarray[x]+=settingsarrayinfected[x][which];;
	    }
	    else if (x<322) {
	      //	      stacker[x-32]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)	stacker[x-66]-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) stacker[x-66]+=settingsarrayinfected[x][which];;

	    }
	    else if (x<578) {
	      //	      stackery[x-288]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)	stackery[x-322]-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) stackery[x-322]+=settingsarrayinfected[x][which];;
	    }
	    else if (x<642) {
	      //	      m->m_threads[x-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)	m->m_threads[x-578].m_CPU-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) m->m_threads[x-578].m_CPU+=settingsarrayinfected[x][which];;
	    }
	    else {
	      //	      villager[x-608]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128)  villager[x-642]-=settingsarrayinfected[x][which];
	      else if (settingsarrayinfected[x][which]>127) villager[x-642]+=settingsarrayinfected[x][which];
	    }
#endif
	  }
	  // swop which and other
	  which^=1;
	  other^=which;
	  }
	}
	break;
    case 15: // fingers in the code... navigate and insert code - no knobs(?)
      // left-right move in datagen
      // down-up into all code values
      xx=fingerdir(&spd);
      
      if (xx==1){ //right
	fingerposl+=spd;
	buf16[fingerposl%32768]=adc_buffer[RIGHT]<<4;
      }
      else if (xx==3){ //left
	fingerposl-=spd;
	buf16[fingerposl%32768]=adc_buffer[LEFT]<<4;
      }
      else if (xx==2){
	fingerposl+=spd;
#ifdef LACH
	  tmper=(fingerposl>>6)%740; // full house//10 bits=1024
	  if (tmper<36) settingsarray[tmper]=adc_buffer[DOWN]<<4;
	  else if (tmper<292) stacker[tmper-36]=adc_buffer[DOWN]<<4;
	  else if (tmper<548) stackery[tmper-292]=adc_buffer[DOWN]<<4;
	  else if (tmper<612) m->m_threads[tmper-548].m_CPU=adc_buffer[DOWN]>>7;
	    else villager[tmper-612]=adc_buffer[DOWN]<<4;
#else
	  tmper=(fingerposl>>6)%770; // full house//10 bits=1024
	  if (tmper<66) settingsarray[tmper]=adc_buffer[DOWN]<<4;
	  else if (tmper<322) stacker[tmper-66]=adc_buffer[DOWN]<<4;
	  else if (tmper<578) stackery[tmper-322]=adc_buffer[DOWN]<<4;
	  else if (tmper<642) m->m_threads[tmper-578].m_CPU=adc_buffer[DOWN]>>7;
	    else villager[tmper-642]=adc_buffer[DOWN]<<4;
#endif
	}
      else if (xx==0){//UP!
#ifdef LACH
	  tmper=(fingerposl>>6)%740; // full house//10 bits=1024
	  if (tmper<36) settingsarray[tmper]=adc_buffer[UP]<<4;
	  else if (tmper<292) stacker[tmper-36]=adc_buffer[UP]<<4;
	  else if (tmper<548) stackery[tmper-292]=adc_buffer[UP]<<4;
	  else if (tmper<612) m->m_threads[tmper-548].m_CPU=adc_buffer[UP]>>7;
	    else villager[tmper-612]=adc_buffer[UP]<<4;
#else
	  tmper=(fingerposl>>6)%770; // full house//10 bits=1024
	  if (tmper<66) settingsarray[tmper]=adc_buffer[UP]<<4;
	  else if (tmper<322) stacker[tmper-66]=adc_buffer[UP]<<4;
	  else if (tmper<578) stackery[tmper-322]=adc_buffer[UP]<<4;
	  else if (tmper<642) m->m_threads[tmper-578].m_CPU=adc_buffer[UP]>>7;
	    else villager[tmper-642]=adc_buffer[UP]<<4;
#endif
	fingerposl-=spd;
      }
      break;
      }

      //END MODECODE      /////////////////////////////////////
      /// DEAL with settingsattach and other attachs....

      /////////////////////////////////////
      // 4-hardware operations

#ifndef LACH
      //      /*      
      tmphardware=0;
      for (x=0;x<256;x++){ // was 256
	tmphardware+=adc_buffer[FIFTH]>>7; // 5 bits now!
      }
      HARDWARE=tmphardware>>8; //was >>8 to divide average
      //*/
      //	HARDWARE=adc_buffer[FIFTH]>>7; // 5 bits now!
      
      /// general HW walk in/as tmp
      if (++hwdel>=HWSPEED){
	if (wormflag[0]) hwpos+=direction[wormdir];
	else hwpos+=(HWSTEP*direction[HWDIR]);
	tmphw=HWSTART+(hwpos%HWWRAP); //to cover all directions
	hwdel=0;

      tmper=adc_buffer[THIRD];
      //      set40106pwm(F0106ERBASE+(buf16[(tmp+F0106EROFFSET)%32768]%F0106ERCONS)); // constrain all to base+constraint
      tmp=F0106ERCONS-F0106ERBASE-tmper;
      set40106pwm(F0106ERBASE+tmper+(buf16[(tmphw+F0106EROFFSET)%32768]%tmp)); // constrain all to base+constraint
      //      set40106pwm(tmper);

      tmp=LMERCONS-LMERBASE;
      if (digfilterflag&4){
	setlmpwm(LMERBASE+(buf16[(tmphw+LMEROFFSET)%32768]%tmp),LMERBASE+(buf16[(tmp+LMEROFFSETTWO)%32768]%tmp)); 
      }
	  
      if (digfilterflag&8){
	tmp=MAXIMERCONS-MAXIMERBASE;
	setmaximpwm(MAXIMERBASE+(buf16[(tmphw+MAXIMEROFFSET)%32768]%tmp));
      }
      }//outside HWSPEED
      
      if (digfilterflag&16){
	//	if (HDGENERCONS==0) settingsarray[24]=256; //SET HDGENERCONS=1
	dohardwareswitch(HARDWARE,datagenbuffer[tmphw]%HDGENERCONS);
      }
      else
	{
	  dohardwareswitch(HARDWARE,0);
	}
#endif //notLACH
#endif //eeg
#endif //straight
      //#endif 
    }
      }

#ifdef  USE_FULL_ASSERT

#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
    {
    }
}
#endif

#if 1
/* exception handlers - so we know what's failing */
void NMI_Handler(void)
{ 
  while(1){};
}

void HardFault_Handler(void)
{ 
  while(1){};
}

void MemManage_Handler(void)
{ 
  while(1){};
}

void BusFault_Handler(void)
{ 
  while(1){};
}

void UsageFault_Handler(void)
{ 
  while(1){};
}

void SVC_Handler(void)
{ 
  while(1){};
}

void DebugMon_Handler(void)
{ 
  while(1){};
}

void PendSV_Handler(void)
{ 
  while(1){};
}
#endif
