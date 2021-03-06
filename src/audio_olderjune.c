/*
 * audio.c - just the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#define STEREO_BUFSZ (BUFF_LEN/2) // 64
#define MONO_BUFSZ (STEREO_BUFSZ/2) // 32

#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "simulation.h"
#include <malloc.h>
#include "audio.h"
#include "settings.h"
extern int16_t *adc_buffer;
int16_t *audio_buffer;
extern int16_t *settingsarray;
extern int16_t *villager;
#define  uint32_t int
typedef int int32_t;
#define float32_t float
int16_t	*left_buffer, *right_buffer, *temp_buffer, *mono_buffer;
#define VILLAGE_SIZE (STACK_SIZE*2) // was 64 *2=128 now 96*2=192 STACK_SIZE is 16 // TESTY!
extern u8* village_effects;

void initaudio(void){
  //int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ], temp_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
left_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
right_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
temp_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
mono_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
}


#else
#include "audio.h"
#include "CPUint.h"
#include "settings.h"
#include "simulation.h"
extern __IO uint16_t adc_buffer[10];
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));
extern u16 settingsarray[64];
extern u16 villager[192];
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ], temp_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
#define VILLAGE_SIZE (STACK_SIZE*2) // was 64 *2=128 now 96*2=192 STACK_SIZE is 16 // TESTY!
extern u8 village_effects[VILLAGE_SIZE/2]; 
#endif

u16 newdirection[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768

extern signed char direction[2];

extern u8 eff[3];

extern u8 wormdir;
extern u8 villagestackpos;
extern u8 digfilterflag;
extern u8 *datagenbuffer;

int16_t *audio_ptr;

void runconvforaudio(u8 sz, int16_t *sorc, int16_t *dest, float c0, float c1, float c2){
  unsigned char i=0, tmpp;
  for (i=0; i<sz; i++) {
    //    tmp++;tmpp=(tmp-1)%sz;
    tmpp=(i-1);
    tmpp=tmpp%sz;
    //    printf("sz %d tmpp %d\n",sz,tmpp);
    *dest++ =((float)sorc[tmpp]*c0)+((float)sorc[i]*c1)+((float)sorc[(i+1)%sz]*c2);
  }
}

extern const u16 SAMPLE_FREQUENCY;
extern const float Pi;
extern const float PI_2;

extern u8 www[3],freqyy[3]; // where to INIT?

void runformforaudio(u8 sz, int16_t *src, int16_t *dst){

  float buff[64]; float x; 

  for (u8 f = 0; f < 3; f++ ) {
  u8 ff = freqyy[f]; // the three freqs

  float freq = (float)ff*(50.0f/SAMPLE_FREQUENCY);

  float buf1Res = 0, buf2Res = 0;
  float q = 1.0f - (float)www[f] * (Pi * 10.0f / SAMPLE_FREQUENCY);
  float xp = 0;
  
  for (u8 s = 0; s < sz; s++ ) {
    // x is our float sample
    // Apply formant filter
    x=(float)(src[s])/32768.0f;
    x = x + 2.0f * cosf ( PI_2 * freq ) * buf1Res * q - buf2Res * q * q;
    buf2Res = buf1Res;
    buf1Res = x;
    x = 0.75f * xp + x;
    xp = x;

    buff[s]+=x; // as float
    if (f==2){
    *dst++=(float)buff[s]*32768.0f;
    }
  }
  }
  }

void Audio_Init(void)
{
	uint32_t i;
	
	/* clear the buffer */
	audio_ptr = audio_buffer;
	i = AUDIO_BUFSZ;
	while(i-- > 0)
		*audio_ptr++ = 0;
	
	/* init the pointer */
	audio_ptr = audio_buffer;
}

void audio_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst)
{
	while(sz)
	{
		*ldst++ = *src++;
		sz--;
		*rdst++ = *src++;
		//		*rdst++ = 0;
		sz--;
	}
}

void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
{
	while(sz)
	{
		*dst++ = *lsrc++;
		sz--;
		*dst++ = (*rsrc++);
		sz--;
	}
}

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  u16 tmp=0,tmper;
  int16_t tmp16,count;
  int32_t tmp32;
  u8 x,tmpp;
  static u16 start=0,startfilt=0,wrapfilt=1,wrap=1,samplepos=0,sampleposfilt=0,anyposfilt=0,anypos=0;
  static u8 villr=0,villf=0,villw=0;
  static u8 villagerpos=0,villagefpos=0,villagewpos=0,del=0,delf=0,delread=0;
  u8 VILLAGEREAD,VILLAGEWRITE,VILLAGEFILT;
  int16_t dirry;
  float w0,w1,w2;

  static u16 anyposread=0,sampleposread=0,wrapread=1,startread=0;
  u16 wrapper; 

#ifdef TEST_STRAIGHT
  audio_split_stereo(sz, src, left_buffer, right_buffer);
  audio_comb_stereo(sz, dst, left_buffer, right_buffer);
#else

	u16 *buf16 = (u16*) datagenbuffer;
	int16_t *buf16int =(int16_t*) datagenbuffer;
	int16_t *firstbuf, *secondbuf;

#ifdef TEST_EEG
	// write buf16 into mono
	      	for (x=0;x<sz/2;x++){
	  mono_buffer[x]=buf16[samplepos%32768];
	  samplepos++;
		}
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

	///	///	///	///

	// readin villager processing of left into left and right into audio_buffer

	int16_t *ldst=left_buffer;
	int16_t *rdst=right_buffer;
	float32_t morph_inv,fsum;
	morph_inv = 1.0 - (float32_t)FMOD;

#ifdef LACH

	if (eff[0]&64) {firstbuf=buf16int;secondbuf=audio_buffer;} // top bit now is buffer
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}
	VILLAGEREAD=(eff[0]&3);	

      	for (x=0;x<sz/2;x++){
	  if (VILLAGEREAD==2){
	    tmpp=village_effects[villr/2]%16;
	  }
	  else tmpp=(eff[0]&63)>>2;
	  switch(tmpp){ 
	  case 0:
	  default:
	  *src++;
	  firstbuf[sampleposread%32768]=*src++;
	  break;
	  case 1:
	  *src++;
	  firstbuf[sampleposread%32768]=*src++;
	  secondbuf[sampleposread%32768]=*src;
	  break;
	  case 2:
	  *src++;
	  tmp16=secondbuf[sampleposread%32768];
	  secondbuf[sampleposread%32768]=firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  // effects with/without clipping *, +, -, 
	  case 3:
	  *src++;
	  //	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)secondbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *src++;
	  //	  tmp32=(*src++)*firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)secondbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *src++;
	  //	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)secondbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *src++;
	  //	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)secondbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  *src++;
	  tmp16=(*src++)-secondbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  *src++;
	  tmp16=secondbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  *src++;
	  tmp16=secondbuf[sampleposread%32768] | (*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  *src++;
	  //	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  *src++;
	  tmp16=(*src++)-firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 12:
	  *src++;
	  tmp16=firstbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 13:
	  *src++;
	  tmp16=firstbuf[sampleposread%32768] |(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 14:
	  *src++;
	  //	  tmp32=(*src++)*firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 15:
	  *src++;
	  //	  tmp32=(*src++)*firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  }
	  /////
	  if (++delread>=SAMPLESPEEDREAD){
	    dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;
	    ///
	    count=((sampleposread-startread)+dirry);
	    //	    if (count<wrapread && (sampleposread+dirry)>startread)
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
	    ////
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD+SAMPLEREXPAND;
		    if (SAMPLEDIRR==1) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    // advance to next in array based on new start and wrap

		    tmp=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=tmp;
		    villr=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    startread=villager[villr];
		    wrapread=(villager[villr+1]%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD; 
		  //		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD;
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  if (wrapread==0) wrapread=1;
		  if (SAMPLEDIRR==1) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}

	/////////////////////////////NO____LACH!!!!!!!!!
#else

	if (digfilterflag&32 || digfilterflag&1){

	  ////////////////////////////////////LDST effects also...

	  if (eff[0]&64) firstbuf=buf16int;
	else firstbuf=audio_buffer;
	VILLAGEREAD=eff[0]&3;
      	for (x=0;x<sz/2;x++){
	  if (VILLAGEREAD==2){
	    tmpp=village_effects[villr/2];
	  }
	  else tmpp=(eff[0]&63)>>2;
	  switch(tmpp){ 
	  case 0:
	  default:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  firstbuf[sampleposread%32768]=*src++;
	  break;
	  case 1:
	  firstbuf[sampleposread%32768]=*src;
	  *ldst++ = *src++;
	  *rdst++ = *src++; 
	  break;
	  // Effects with/without clipping *, +, -, 
	  case 2:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  //	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 3:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  //	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  //	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  //	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp16=(*src++)-firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 7:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp16=firstbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  // start of *(src-1)
	  case 8:
	  *ldst++ = *src++;
	  tmp16=*src++;
	  tmp32= *(src) * tmp16;
	  *rdst++ = *src; 
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 9:
	  *ldst++ = *src;
	  //	  tmp32= *(src++) * (*src++);
	  tmp16=*src++;
	  fsum=(float32_t)tmp16 * morph_inv * (float32_t)*src++ * FMOD;
	  tmp32=fsum;
	  *rdst++ = *src; 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 10:
	  *ldst++ = *src;
	  //	  tmp32=firstbuf[samplepos%32768]* *src++;
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  *rdst++ = *src++; 
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  *ldst++ = *src;
	  //	  tmp32=firstbuf[samplepos%32768]* *src++;
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  //	  tmp32=fsum;
	  *rdst++ = *src++; 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 12:
	  *ldst++ = *src;
	  //	  tmp32=*src++ +firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  *rdst++ = *src; 
	  (src++);
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  case 13:
	  *ldst++ = *src++;
	  //repeats???
	  fsum=(float32_t)(float32_t)firstbuf[sampleposread%32768] * morph_inv + (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  	  //	  tmp32=*src++ * *src++;
	  //	  fsum=(float32_t)*src++ * morph_inv * (float32_t)*src++ * FMOD;
	  tmp32=fsum;
	  *rdst++ = *src++; 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	  *ldst++ = *src++;
	  tmper= *src++;
	  tmp16=*src | tmper;
	  *rdst++ = *src; 
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
 	  case 15:
	  *ldst++ = *src++;
	  //	  tmp32=*src++ + *src++;
	  tmp16=*src;
	  fsum=(float32_t)tmp16 * morph_inv + (float32_t)*src++ * FMOD;
	  tmp32=fsum;
	  *rdst++ = *src; 
	  firstbuf[sampleposread%32768]=tmp32;
	  }
	  	  if (++delread>=SAMPLESPEEDREAD){
		    dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;
		    count=((sampleposread-startread)+dirry);
		    //if (count<wrapread && (sampleposread+dirry)>startread)
		    if (count<wrapread && count>0)
		      {
			sampleposread+=dirry;//)%32768;
		      }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD+SAMPLEREXPAND;
		    if (SAMPLEDIRR==1) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=tmp;
		    villr=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    startread=villager[villr];
		    wrapread=(villager[villr+1]%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD; 
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD;
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  if (wrapread==0) wrapread=1;
		  if (SAMPLEDIRR==1) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}
	}
	else  // READIN NO DIG FILTER
	  {
	    if (eff[0]&64) {firstbuf=buf16int;secondbuf=audio_buffer;}
	    else  {secondbuf=buf16int;firstbuf=audio_buffer;}
	    VILLAGEREAD=eff[0]&3;
	    //	    tmpp=0; firstbuf=audio_buffer; // TESTY!!!
	    morph_inv = 1.0 - (float32_t)FMOD,fsum;
	    for (x=0;x<sz/2;x++){
 	  if (VILLAGEREAD==2){
	    tmpp=village_effects[villr/2];
	  }
	  else tmpp=(eff[0]&63)>>2;
	  switch(tmpp){
	  case 0:
	  default:
	  (src++);
	  firstbuf[sampleposread%32768]=*src++;
	  break;
	  case 1:
	    //	  (src++);
	  firstbuf[sampleposread%32768]=*src++;
	  secondbuf[sampleposread%32768]=*src++;
	  break;	    
	  case 2:
	    //*src++;
	  tmp16=secondbuf[sampleposread%32768];
	  secondbuf[sampleposread%32768]=firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  ////////
	  // Effects with/without clipping *, +, -, 
	  case 3:
	    (src++);
	  //	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)secondbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	    (src++);
	  //	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)secondbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  (src++);
	  //	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)secondbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  (src++);
	  //	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)secondbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  (src++);
	  tmp16=(*src++)-secondbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  (src++);
	  tmp16=secondbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  (src++);
	  tmp16=secondbuf[sampleposread%32768]|(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  (src++);
	  //	  tmp32=(*src++)*firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  (src++);
	  //	  	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 12:
	  (src++);
	  //	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)firstbuf[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 13:
	  (src++);
	  tmp32=(*src++)-firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	  (src++);
	  tmp16=firstbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 15:
	  (src++);
	  tmp16=firstbuf[sampleposread%32768]|(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  }
	  //	  VILLAGEREAD=0; // TESTY!
	  if (++delread>=SAMPLESPEEDREAD){
	    dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;
	    //	  dirry=1; // TESTY!
	    count=((sampleposread-startread)+dirry);
	    //	    if (count<wrapread && (sampleposread+dirry)>startread)
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD+SAMPLEREXPAND;
		    //		    startread=0;wrapread=32768; // TESTY!
		    if (SAMPLEDIRR==1) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    tmp=VILLAGERSTEP*direction[VILLAGERDIR];
		    //tmp=1; // TSTER!!
		    villagerpos+=tmp; //???
		    villr=((VILLAGERSTART+(villagerpos%VILLAGERWRAP))*2)%villagestackpos; //to cover all directions
		    startread=villager[villr];
		    wrapread=(villager[villr+1]%SAMPLEWRAPREAD)+SAMPLEREXPAND;;
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD; 
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD;
		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD)+SAMPLEREXPAND;;
		  //		  if (wrapread==0) wrapread=1;
		  if (SAMPLEDIRR==1) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}
	  }
#endif
	///!!!!!////////////////////////////////END OF READINSSS
	///!!!!!////////////////////////////////END OF READINSSS
	///!!!!!////////////////////////////////END OF READINSSS
	morph_inv = 1.0 - (float32_t)FMODW;

#ifdef LACH
	// firstbuf, secondbuf
	  if (eff[1]&64) {firstbuf=buf16int;secondbuf=audio_buffer;}
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}

	VILLAGEWRITE=eff[1]&3;
      	for (x=0;x<sz/2;x++){
 	  if (VILLAGEWRITE==2){
	    tmpp=village_effects[villw/2];
	  }
	  else tmpp=(eff[1]&63)>>2;
	  switch(tmpp){ 
	  case 0:
	  mono_buffer[x]=firstbuf[samplepos%32768];
	  break;
	  case 1:
	    //	  mono_buffer[x]=secondbuf[samplepos%32768]; no sense
	  tmp16=firstbuf[samplepos%32768]%secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  // effects with/without clipping *, +, -, 
	  case 2:
	    //	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv * (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	    //	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv * (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	    //	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv + (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	    //	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv + (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	  tmp16=firstbuf[samplepos%32768]-secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	  tmp16=secondbuf[samplepos%32768]-firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 8:
	  tmp16=secondbuf[samplepos%32768]|firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 9:
	  tmp16=secondbuf[samplepos%32768]&firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	  tmp16=secondbuf[samplepos%32768]|firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 11:
	    tmp16=secondbuf[samplepos%32768]<<(firstbuf[samplepos%32768])&7;
	  mono_buffer[x]=tmp16;
	  break;
	  case 12:
	  break;
	    tmp16=secondbuf[samplepos%32768]>>(firstbuf[samplepos%32768])&7;
 	  default:
	    temp_buffer[x]=firstbuf[samplepos%32768];
	  }

	  	  if (++del>=SAMPLESPEED){
	    dirry=direction[SAMPLEDIRW]*SAMPLESTEP;
	    count=((samplepos-start)+dirry);// samplepos is start or start+wrap++
		    if (count<wrapread && count>0)
	      {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP+SAMPLEEXPAND;
		    if (SAMPLEDIRW==1) samplepos=start; //forwards
		    else samplepos=start+wrap;
		    count=samplepos;
		  }

		  else if (VILLAGEWRITE==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP)+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=tmp;
		    villw=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    start=villager[villw];
		    wrap=(villager[villw+1]%SAMPLEWRAP)+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (SAMPLEDIRW==1) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP; 
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP;
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP)+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  }
	}
	  // process mono_buffer for extra effects 13/14/15
	  if (tmpp>12){
	    switch(tmpp){
	    case 13:
	    runconvforaudio(sz/2,temp_buffer,mono_buffer,0.5,0.5,0.5);
	    break;
	    case 14:
	      // 3 floats!
	      w0=(float32_t) buf16[0]/65536.0f;w1=(float32_t) buf16[1]/65536.0f;w2=(float32_t) buf16[2]/65536.0f;
	    runconvforaudio(sz/2,temp_buffer,mono_buffer,w0,w1,w2);
	      break;
	    case 15:
	      //	      runformforaudio(sz/2,temp_buffer,mono_buffer);
	      break;
	    }
	  } // end of tmpp>12
	

	/////////////////////////////NO____LACH!!!!!!!!!
#else

	if (digfilterflag&32 || digfilterflag&1){
	ldst=left_buffer;

	  ////////////////////////////////////LDST effects also...

	  if (eff[1]&64) {firstbuf=buf16int;secondbuf=audio_buffer;}
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}

	VILLAGEWRITE=eff[1]&3;
      	for (x=0;x<sz/2;x++){

 	  if (VILLAGEWRITE==2){
	    tmpp=village_effects[villw/2];
	  }
	  else tmpp=(eff[1]&63)>>2;

	  switch(tmpp){ 
	  case 0:
	    mono_buffer[x]=firstbuf[samplepos%32768];
	  break;
	  // effects with/without clipping *, +, -, 
	  case 1:
	    //	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv * (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 2:
	    //	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv * (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	    //	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv + (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	    //	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv + (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	  tmp16=firstbuf[samplepos%32768]-secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 6:
	  tmp16=secondbuf[samplepos%32768]-firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	    //	  tmp32=firstbuf[samplepos%32768]* *ldst++;
	    fsum=(float32_t)*ldst++ * morph_inv * (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 8:
	    //	  tmp32=firstbuf[samplepos%32768]* *ldst++;
	    fsum=(float32_t)*ldst++ * morph_inv * (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 9:
	    tmp16=firstbuf[samplepos%32768] - *ldst++;
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	    //	  tmp32=*(ldst++)+secondbuf[samplepos%32768];
	  fsum=(float32_t)*ldst++ * morph_inv + (float32_t)secondbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 11:
	    //	  tmp32=*(ldst++)+secondbuf[samplepos%32768];
	    fsum=(float32_t)*ldst++ * morph_inv + (float32_t)secondbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 12:
	    mono_buffer[x]=(adc_buffer[9]<<3) * firstbuf[samplepos%32768];
	  break;
	  case 13:
	    //	    tmp32=firstbuf[samplepos%32768]+adc_buffer[9]<<3;
	    fsum=(float32_t)(adc_buffer[9]<<3) * morph_inv + (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
	  case 14:
	    //	    tmp32=firstbuf[samplepos%32768]*adc_buffer[9]<<3;
	    fsum=(float32_t)(adc_buffer[9]<<3) * morph_inv * (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
 	  default:
	    temp_buffer[x]=firstbuf[samplepos%32768];
	  }
	  ////////////////////////--->>>>


	  	  if (++del>=SAMPLESPEED){
	    dirry=direction[SAMPLEDIRW]*SAMPLESTEP;
	    count=((samplepos-start)+dirry);
	    //	    if (count<wrap && (samplepos+dirry)>start)
	    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP+SAMPLEEXPAND;
		    if (SAMPLEDIRW==1) samplepos=start; //forwards
		    else samplepos=start+wrap;
		  }

		  else if (VILLAGEWRITE==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP)+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=tmp;
		    villw=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    start=villager[villw];
		    wrap=(villager[villw+1]%SAMPLEWRAP)+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (SAMPLEDIRW==1) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP; 
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP;
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP)+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  }
	}
	  // process mono_buffer for extra effects 13/14/15
	  if (tmpp==15){
	      // 3 floats!
	      w0=(float32_t) buf16[0]/65536.0f;w1=(float32_t) buf16[1]/65536.0f;w2=(float32_t) buf16[2]/65536.0f;
	      runconvforaudio(sz/2,temp_buffer,mono_buffer,w0,w1,w2);
	  } // end of tmpp==15
	
	}
	else
	  { /// STRAIGHT SANS FILTEROPSSS!!!
	    if (eff[1]&64) {firstbuf=buf16int;secondbuf=audio_buffer;}
	    else  {secondbuf=buf16int;firstbuf=audio_buffer;}
	  VILLAGEWRITE=eff[1]&3;
	  VILLAGEWRITE=2; // TESTY!!!!
	  //tmpp=0;firstbuf=buf16int;secondbuf=buf16int; // TESTYYY!!!
		  //	  tmpp=0;firstbuf=audio_buffer;secondbuf=buf16int; // TESTYYY!!!

	  for (x=0;x<sz/2;x++){

	  if (VILLAGEWRITE==2){
	    tmpp=village_effects[villw/2];
	  }
	  else tmpp=(eff[1]&63)>>2;

	  //	     printf("%d\n",samplepos);

	  switch(tmpp){ 
	  case 0:
	     mono_buffer[x]=firstbuf[samplepos%32768];
	     //	     printf("%c",mono_buffer[x]);
	     //	     printf("x: %d %d %c\n",x, samplepos%32768, mono_buffer[x]);
	    break;
	  case 1:
	    // 	    mono_buffer[x]=secondbuf[samplepos%32768]; no sense
	    tmp16=firstbuf[samplepos%32768]%(secondbuf[samplepos%32768]+1);
	  mono_buffer[x]=tmp16;
	  break;
	  // effects with/without clipping *, +, -, 
	  case 2:
	    //tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	    fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv * (float32_t)firstbuf[samplepos%32768] * FMOD;
	    tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	    //	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] * morph_inv + (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	    //	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] + morph_inv + (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	    //	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  fsum=(float32_t)secondbuf[samplepos%32768] + morph_inv + (float32_t)firstbuf[samplepos%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	  tmp16=firstbuf[samplepos%32768]-secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	  tmp16=secondbuf[samplepos%32768]-firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 8:
	  tmp16=secondbuf[samplepos%32768] | firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 9:
	  tmp16=firstbuf[samplepos%32768] & secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	    mono_buffer[x]=(adc_buffer[9]<<3)*firstbuf[samplepos%32768];
	  break;
	  case 11:
	    //	    tmp32=firstbuf[samplepos%32768]+adc_buffer[9]<<3;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv + (float32_t)(adc_buffer[9]<<3) * FMOD;
 	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
	  case 12:
	    //	    tmp32=firstbuf[samplepos%32768]*adc_buffer[9]<<3;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv * (float32_t)(adc_buffer[9]<<3) * FMOD;
	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
 	  default:
	    temp_buffer[x]=firstbuf[samplepos%32768];
	  }
 
	  if (++del>=SAMPLESPEED){ 
	    dirry=direction[SAMPLEDIRW]*SAMPLESTEP; 
	    //	    dirry=newdirection[wormdir];
	    count=((samplepos-start)+dirry);
	    //	    if (count<wrap && (samplepos+dirry)>start)
		    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP+SAMPLEEXPAND;
		    if (SAMPLEDIRW==1) samplepos=start; //forwards
		    else samplepos=start+wrap;
		    //		    printf("here samplepos=%d wrap=%d\n",samplepos,wrap);
		    //		    samplepos=0;start=0;wrap=32767;
		  }
		  else if (VILLAGEWRITE==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP)+SAMPLEEXPAND;
		  //		  samplepos=0;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=tmp;
		    villw=((VILLAGEWSTART+(villagewpos%VILLAGEWWRAP))*2)%villagestackpos; //villagestackpos always +-2
		    start=villager[villw];
		    wrap=(villager[villw+1]%SAMPLEWRAP)+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (SAMPLEDIRW==1) samplepos=start;
		    else samplepos=start+wrap;
		    #ifdef PCSIM
		    //		    printf("villager[tmp+1] %d \n",(villager[tmp+1]));
		    //		    printf("villager %d samplepos: %d wrap: %d\n",tmp,samplepos,wrap);
			   #endif
		    //		    start=0; samplepos=0;
		    //		    wrap=32767;
		  }
		  else {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP; 
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP;
		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  // TODO - prototype for expansion
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP)+SAMPLEEXPAND;

		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  	  }
	  }

	  // process mono_buffer for extra effects 13/14/15
	  if (tmpp>12){

	    switch(tmpp){
	    case 13:
	    runconvforaudio(sz/2,temp_buffer,mono_buffer,0.5,0.5,0.5);
	    break;
	    case 14:
	      // 3 floats!
	      w0=(float32_t) buf16[0]/65536.0f;w1=(float32_t) buf16[1]/65536.0f;w2=(float32_t) buf16[2]/65536.0f;
	    runconvforaudio(sz/2,temp_buffer,mono_buffer,w0,w1,w2);
	      break;
	    case 15:
	      runformforaudio(sz/2,temp_buffer,mono_buffer);
	      break;
	    }
	  } // end of tmpp>12
	  
	  }
      	///!!!!!!////////////////////////////////END OF WRITEOUTSSS

	///	///	///	///

if (digfilterflag&1){ 

	ldst=left_buffer;
	rdst=right_buffer;
	morph_inv = 1.0 - (float32_t)FMODF;

	  ////////////////////////////////////LDST effects also...
	if (eff[2]&64) firstbuf=buf16int; 
	else firstbuf=audio_buffer;
	VILLAGEFILT=eff[2]&3;
	//	tmpp=(eff[2]&63)>>2;
      	for (x=0;x<sz/2;x++){ 

 	  if (VILLAGEFILT==2){
	    tmpp=village_effects[villf/2];
	  }
	  else tmpp=(eff[2]&63)>>2;
	  switch(tmpp){ 
	  case 0:
	  default:
	  *ldst++=firstbuf[sampleposfilt%32768];
	  break;
	  case 1:
	  *ldst++=*rdst++;
	  break;
	  case 2:
	    *ldst++ =0;
	    break;
	  // effects with/without clipping *, +, -, 
	  case 3:
	    //	    tmp32=firstbuf[sampleposfilt%32768]* *ldst++;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv * (float32_t)(*ldst++) * FMOD;
	  tmp32=fsum;
	  *ldst=tmp32;
	  break;
	  case 4:
	    //	  tmp32=firstbuf[sampleposfilt%32768]* *ldst++;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv * (float32_t)(*ldst++) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst=tmp32;
	  break;
	  case 5:
	    //	  tmp32=firstbuf[sampleposfilt%32768]* *rdst++;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv * (float32_t)(*rdst++) * FMOD;
	  tmp32=fsum;
	  *ldst++=tmp32;
	  break;
	  case 6:
	    //	  tmp32=firstbuf[sampleposfilt%32768]* *rdst++;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv * (float32_t)(*rdst++) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst++=tmp32;
	  break;
	  case 7:
	    //	  tmp32=*ldst++ * *rdst++;
	    fsum=(float32_t)(*ldst++) * morph_inv * (float32_t)(*rdst++) * FMOD;
	  tmp32=fsum;
	  *ldst=tmp32;
	  break;
	  case 8:
	    //	  tmp32=*ldst++ * *rdst++;
	    fsum=(float32_t)(*ldst++) * morph_inv * (float32_t)(*rdst++) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst=tmp32;
	  break;
	  case 9:
	    //	  tmp32=firstbuf[sampleposfilt%32768]+ *ldst++;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv * (float32_t)(*ldst++) * FMOD;
	  tmp32=fsum;
	  *ldst=tmp32;
	  break;
	  case 10:
	    //	  tmp32=firstbuf[sampleposfilt%32768]+ *ldst++;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv * (float32_t)(*ldst++) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst=tmp32;
	  break;
	  case 11:
	    //	  tmp32=firstbuf[sampleposfilt%32768]+ *rdst++;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv * (float32_t)(*rdst++) * FMOD;
	  tmp32=fsum;
	  *ldst++=tmp32;
	  break;
	  case 12:
	    //	  tmp32=firstbuf[sampleposfilt%32768]+ *rdst++;
	    fsum=(float32_t)firstbuf[samplepos%32768] + morph_inv * (float32_t)(*rdst++) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst++=tmp32;
	  break;
	  case 13:
	    //	  tmp32=*ldst++ + *rdst++;
	    fsum=(float32_t)(*ldst++) * morph_inv + (float32_t)(*rdst++) * FMOD;
	  tmp32=fsum;
	  *ldst=tmp32;
	  break;
	  case 14:
	    //	  tmp32=*ldst++ + *rdst++;
	    fsum=(float32_t)(*ldst++) * morph_inv + (float32_t)(*rdst++) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst=tmp32;
	  break;
	  case 15:
	  tmp32=firstbuf[sampleposfilt%32768]| *ldst++;
	  *ldst=tmp32;
	  break;
	  }
	  ///	  HERE////////////////////////--->>>>

	  	  if (++delf==SAMPLESPEEDFILT){
	    dirry=direction[SAMPLEDIRF]*SAMPLESTEPFILT;
	    count=((sampleposfilt-startfilt)+dirry);
	    //	    if (count<wrapfilt && (sampleposfilt+dirry)>startfilt)
		    if (count<wrapfilt && count>0)
		  {
		    sampleposfilt+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEFILT==0) {
		    startfilt=SAMPLESTARTFILT;wrapfilt=SAMPLEWRAPFILT+SAMPLEFEXPAND;
		    if (SAMPLEDIRF==1) sampleposfilt=startfilt; //forwards
		    else sampleposfilt=startfilt+wrapfilt;
		  }

		  else if (VILLAGEFILT==1) {
		  tmp=ANYSTEPFILT*direction[DATADIRF];
		  anyposfilt+=tmp;
		  tmp=(ANYSTARTFILT+(anyposfilt%ANYWRAPFILT))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposfilt=SAMPLESTARTFILT+(tmper%SAMPLEWRAPFILT)+SAMPLEFEXPAND;
		  wrapfilt=0;startfilt=0;
		  }
		  else if (VILLAGEFILT==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEFSTEP*direction[VILLAGEFDIR];
		    villagefpos+=tmp;
		    villf=(VILLAGEFSTART+(villagefpos%VILLAGEFWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    startfilt=villager[villf];
		    wrapfilt=(villager[villf+1]%SAMPLEWRAPFILT)+SAMPLEFEXPAND;
		    if (wrapfilt==0) wrapfilt=1;
		    if (SAMPLEDIRF==1) sampleposfilt=startfilt;
		    else sampleposfilt=startfilt+wrapfilt;
		  }
		  else {
		  tmp=ANYSTEPFILT*direction[DATADIRF];
		  anyposfilt+=tmp;
		  wrapper=ANYWRAPFILT; 
		  		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  startfilt=buf16[tmp]>>1;
		  tmp=ANYSTEPFILT*direction[DATADIRF];
		  anyposfilt+=tmp;
		  wrapper=ANYWRAPFILT;
		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  wrapfilt=((buf16[tmp]>>1)%SAMPLEWRAPFILT)+SAMPLEFEXPAND;
		  if (wrapfilt==0) wrapfilt=1;
		  if (SAMPLEDIRF==1) sampleposfilt=startfilt;
		  else sampleposfilt=startfilt+wrapfilt;
		  }
		}
	  delf=0;
		  }
	}

 }

#endif // for LACH

	// 4-out
	//audio_comb_stereo(sz, dst, left_buffer, right_buffer);
 audio_comb_stereo(sz, dst, left_buffer, mono_buffer);

#ifdef PCSIM
 // if (wormdir>=8)  printf("woxxxxxrmdir:%d\n",wormdir);
 //printf("woxxxxxrmdir:%d\n",wormdir);
 //  for (x=0;x<sz/2;x++){
    //    printf("%c",mono_buffer[x]);
 //      }
#endif

#endif // for test eeg
#endif // for straight

}
