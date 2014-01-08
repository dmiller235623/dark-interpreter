#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#endif

typedef unsigned char u8;
typedef uint16_t u16;

#define true 1
#define false 0
#define MAX_SAM 65536
#define floor(x) ((int)(x))
#define ONESIXTH 0.1666666666666667
#define BET(A, B, C)  (((A>=B)&&(A<=C))?1:0)    /* a between [b,c] */
#define MAX_GROUPS 16

struct siney{
  u8 del;
  u16 sin_data[256];  // sine LUT Array
  u16 cc;
};

struct generik{
  u8 del;
  u16 cop;
};

struct simpleSIR{
  float beta;//=520.0/365.0;
  float gamm;//=1.0/7.0;
  float S0;//=1.0-1e-6;
  float I0;//=1e-6;
  float step;
  float S,I,R;
  float dPop[3];
};

struct SEIR {
  float beta;
  float step;
  float gamm;
  int n;
  int m;
  float mu;
  float S0,I0;
  float S,I[MAX_GROUPS]; // 4x8x16=512bytes
  float dPop[MAX_GROUPS+1];//4x9=36bytes
};

struct SICR {
  float beta;
  float epsilon;
  float gamm;
  float Gamm; 
  float mu;
  float q;
  float S0;
  float I0;
  float C0;
  float t,S,I,C,R;
  float dPop[3];
  float step;
};

typedef struct{ float x, y; } Point;

struct IFS {
	float prob[5];
	float coeff[4][6];
	Point p1,p2;
	};

struct Rossler{
  float h,a,b,c,lx0,ly0,lz0;
};

struct secondRossler{
  float z0, zn, znm1;
  float x0, y0, xn, yn, xnm1, ynm1;
};

struct Brussel{
    float x,y; 
};

struct Spruce{
    float x, y; 
};

struct Oregon
{
  float x, y, z; 
};

struct Fitz
{
	float u,w;
};
