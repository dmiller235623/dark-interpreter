/*
 * audio.c - justttt the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

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

#ifdef LACH
#define SETSIZE 36
#define INFECTSIZE 740 
#define SETSHIFT 11
#define SHIFTY 7
#else
#define SETSIZE 66
#define INFECTSIZE 770 
#define SETSHIFT 10
#define SHIFTY 6
#endif

#define STEREO_BUFSZ (BUFF_LEN/2) // 64
#define MONO_BUFSZ (STEREO_BUFSZ/2) // 32
#define randi() (adc_buffer[9]) // 12 bits

#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "simulation.h"
#include <malloc.h>
#include "audio.h"
//#include "settings.h"
extern int16_t *adc_buffer;
int16_t *audio_buffer;
#define  uint32_t int
typedef int int32_t;
#define float32_t float
int16_t	*left_buffer, *right_buffer, *mono_buffer;

void initaudio(void){
left_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
right_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
mono_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
}

#else
#include "audio.h"
#include "CPUint.h"
//#include "settings.h"
#include "hardware.h"
#include "simulation.h"
extern __IO uint16_t adc_buffer[10];
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
#define float float32_t
#endif

int16_t newdirection[8]={-256,-255,1,255,256,254,-1,-257};

static const u16 loggy[4096] __attribute__ ((section (".flash"))) ={0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 61, 61, 61, 61, 61, 61, 62, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 64, 64, 64, 64, 64, 64, 64, 65, 65, 65, 65, 65, 65, 66, 66, 66, 66, 66, 66, 67, 67, 67, 67, 67, 68, 68, 68, 68, 68, 68, 69, 69, 69, 69, 69, 69, 70, 70, 70, 70, 70, 71, 71, 71, 71, 71, 71, 72, 72, 72, 72, 72, 73, 73, 73, 73, 73, 73, 74, 74, 74, 74, 74, 75, 75, 75, 75, 75, 76, 76, 76, 76, 76, 77, 77, 77, 77, 77, 78, 78, 78, 78, 78, 79, 79, 79, 79, 79, 80, 80, 80, 80, 80, 81, 81, 81, 81, 81, 82, 82, 82, 82, 82, 83, 83, 83, 83, 83, 84, 84, 84, 84, 85, 85, 85, 85, 85, 86, 86, 86, 86, 87, 87, 87, 87, 87, 88, 88, 88, 88, 89, 89, 89, 89, 89, 90, 90, 90, 90, 91, 91, 91, 91, 92, 92, 92, 92, 92, 93, 93, 93, 93, 94, 94, 94, 94, 95, 95, 95, 95, 96, 96, 96, 96, 97, 97, 97, 97, 98, 98, 98, 98, 99, 99, 99, 99, 100, 100, 100, 100, 101, 101, 101, 101, 102, 102, 102, 102, 103, 103, 103, 103, 104, 104, 104, 105, 105, 105, 105, 106, 106, 106, 106, 107, 107, 107, 107, 108, 108, 108, 109, 109, 109, 109, 110, 110, 110, 111, 111, 111, 111, 112, 112, 112, 113, 113, 113, 113, 114, 114, 114, 115, 115, 115, 115, 116, 116, 116, 117, 117, 117, 118, 118, 118, 118, 119, 119, 119, 120, 120, 120, 121, 121, 121, 121, 122, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 126, 126, 126, 127, 127, 127, 128, 128, 128, 128, 129, 129, 129, 130, 130, 130, 131, 131, 131, 132, 132, 132, 133, 133, 133, 134, 134, 135, 135, 135, 136, 136, 136, 137, 137, 137, 138, 138, 138, 139, 139, 139, 140, 140, 140, 141, 141, 142, 142, 142, 143, 143, 143, 144, 144, 144, 145, 145, 146, 146, 146, 147, 147, 147, 148, 148, 149, 149, 149, 150, 150, 150, 151, 151, 152, 152, 152, 153, 153, 154, 154, 154, 155, 155, 156, 156, 156, 157, 157, 158, 158, 158, 159, 159, 160, 160, 160, 161, 161, 162, 162, 162, 163, 163, 164, 164, 164, 165, 165, 166, 166, 167, 167, 167, 168, 168, 169, 169, 170, 170, 170, 171, 171, 172, 172, 173, 173, 174, 174, 174, 175, 175, 176, 176, 177, 177, 178, 178, 178, 179, 179, 180, 180, 181, 181, 182, 182, 183, 183, 184, 184, 184, 185, 185, 186, 186, 187, 187, 188, 188, 189, 189, 190, 190, 191, 191, 192, 192, 193, 193, 194, 194, 195, 195, 196, 196, 197, 197, 198, 198, 199, 199, 200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207, 208, 208, 209, 210, 210, 211, 211, 212, 212, 213, 213, 214, 214, 215, 215, 216, 217, 217, 218, 218, 219, 219, 220, 220, 221, 222, 222, 223, 223, 224, 224, 225, 226, 226, 227, 227, 228, 228, 229, 230, 230, 231, 231, 232, 233, 233, 234, 234, 235, 236, 236, 237, 237, 238, 239, 239, 240, 240, 241, 242, 242, 243, 243, 244, 245, 245, 246, 247, 247, 248, 248, 249, 250, 250, 251, 252, 252, 253, 254, 254, 255, 256, 256, 257, 257, 258, 259, 259, 260, 261, 261, 262, 263, 263, 264, 265, 265, 266, 267, 267, 268, 269, 270, 270, 271, 272, 272, 273, 274, 274, 275, 276, 276, 277, 278, 279, 279, 280, 281, 281, 282, 283, 284, 284, 285, 286, 286, 287, 288, 289, 289, 290, 291, 292, 292, 293, 294, 295, 295, 296, 297, 298, 298, 299, 300, 301, 301, 302, 303, 304, 305, 305, 306, 307, 308, 308, 309, 310, 311, 312, 312, 313, 314, 315, 316, 316, 317, 318, 319, 320, 320, 321, 322, 323, 324, 325, 325, 326, 327, 328, 329, 329, 330, 331, 332, 333, 334, 335, 335, 336, 337, 338, 339, 340, 341, 341, 342, 343, 344, 345, 346, 347, 348, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 433, 434, 435, 436, 437, 438, 439, 440, 441, 443, 444, 445, 446, 447, 448, 449, 450, 452, 453, 454, 455, 456, 457, 459, 460, 461, 462, 463, 464, 466, 467, 468, 469, 470, 472, 473, 474, 475, 476, 478, 479, 480, 481, 482, 484, 485, 486, 487, 489, 490, 491, 492, 494, 495, 496, 497, 499, 500, 501, 502, 504, 505, 506, 508, 509, 510, 512, 513, 514, 515, 517, 518, 519, 521, 522, 523, 525, 526, 527, 529, 530, 531, 533, 534, 535, 537, 538, 540, 541, 542, 544, 545, 546, 548, 549, 551, 552, 553, 555, 556, 558, 559, 561, 562, 563, 565, 566, 568, 569, 571, 572, 573, 575, 576, 578, 579, 581, 582, 584, 585, 587, 588, 590, 591, 593, 594, 596, 597, 599, 600, 602, 603, 605, 606, 608, 610, 611, 613, 614, 616, 617, 619, 620, 622, 624, 625, 627, 628, 630, 632, 633, 635, 636, 638, 640, 641, 643, 645, 646, 648, 650, 651, 653, 654, 656, 658, 659, 661, 663, 665, 666, 668, 670, 671, 673, 675, 676, 678, 680, 682, 683, 685, 687, 689, 690, 692, 694, 696, 697, 699, 701, 703, 705, 706, 708, 710, 712, 714, 715, 717, 719, 721, 723, 724, 726, 728, 730, 732, 734, 736, 737, 739, 741, 743, 745, 747, 749, 751, 753, 755, 756, 758, 760, 762, 764, 766, 768, 770, 772, 774, 776, 778, 780, 782, 784, 786, 788, 790, 792, 794, 796, 798, 800, 802, 804, 806, 808, 810, 812, 814, 816, 818, 821, 823, 825, 827, 829, 831, 833, 835, 837, 840, 842, 844, 846, 848, 850, 852, 855, 857, 859, 861, 863, 866, 868, 870, 872, 874, 877, 879, 881, 883, 886, 888, 890, 892, 895, 897, 899, 901, 904, 906, 908, 911, 913, 915, 918, 920, 922, 925, 927, 929, 932, 934, 936, 939, 941, 944, 946, 948, 951, 953, 956, 958, 961, 963, 965, 968, 970, 973, 975, 978, 980, 983, 985, 988, 990, 993, 995, 998, 1000, 1003, 1005, 1008, 1011, 1013, 1016, 1018, 1021, 1024, 1026, 1029, 1031, 1034, 1037, 1039, 1042, 1045, 1047, 1050, 1053, 1055, 1058, 1061, 1063, 1066, 1069, 1071, 1074, 1077, 1080, 1082, 1085, 1088, 1091, 1093, 1096, 1099, 1102, 1105, 1107, 1110, 1113, 1116, 1119, 1122, 1124, 1127, 1130, 1133, 1136, 1139, 1142, 1145, 1147, 1150, 1153, 1156, 1159, 1162, 1165, 1168, 1171, 1174, 1177, 1180, 1183, 1186, 1189, 1192, 1195, 1198, 1201, 1204, 1207, 1210, 1213, 1216, 1220, 1223, 1226, 1229, 1232, 1235, 1238, 1241, 1245, 1248, 1251, 1254, 1257, 1261, 1264, 1267, 1270, 1273, 1277, 1280, 1283, 1286, 1290, 1293, 1296, 1300, 1303, 1306, 1309, 1313, 1316, 1319, 1323, 1326, 1330, 1333, 1336, 1340, 1343, 1347, 1350, 1353, 1357, 1360, 1364, 1367, 1371, 1374, 1378, 1381, 1385, 1388, 1392, 1395, 1399, 1402, 1406, 1410, 1413, 1417, 1420, 1424, 1428, 1431, 1435, 1438, 1442, 1446, 1449, 1453, 1457, 1461, 1464, 1468, 1472, 1475, 1479, 1483, 1487, 1491, 1494, 1498, 1502, 1506, 1510, 1513, 1517, 1521, 1525, 1529, 1533, 1537, 1541, 1545, 1548, 1552, 1556, 1560, 1564, 1568, 1572, 1576, 1580, 1584, 1588, 1592, 1596, 1600, 1604, 1609, 1613, 1617, 1621, 1625, 1629, 1633, 1637, 1642, 1646, 1650, 1654, 1658, 1663, 1667, 1671, 1675, 1680, 1684, 1688, 1692, 1697, 1701, 1705, 1710, 1714, 1718, 1723, 1727, 1732, 1736, 1740, 1745, 1749, 1754, 1758, 1763, 1767, 1772, 1776, 1781, 1785, 1790, 1794, 1799, 1803, 1808, 1813, 1817, 1822, 1826, 1831, 1836, 1840, 1845, 1850, 1854, 1859, 1864, 1869, 1873, 1878, 1883, 1888, 1892, 1897, 1902, 1907, 1912, 1917, 1922, 1926, 1931, 1936, 1941, 1946, 1951, 1956, 1961, 1966, 1971, 1976, 1981, 1986, 1991, 1996, 2001, 2006, 2011, 2017, 2022, 2027, 2032, 2037, 2042, 2048, 2053, 2058, 2063, 2068, 2074, 2079, 2084, 2090, 2095, 2100, 2106, 2111, 2116, 2122, 2127, 2132, 2138, 2143, 2149, 2154, 2160, 2165, 2171, 2176, 2182, 2187, 2193, 2198, 2204, 2210, 2215, 2221, 2226, 2232, 2238, 2244, 2249, 2255, 2261, 2266, 2272, 2278, 2284, 2290, 2295, 2301, 2307, 2313, 2319, 2325, 2331, 2337, 2342, 2348, 2354, 2360, 2366, 2372, 2378, 2385, 2391, 2397, 2403, 2409, 2415, 2421, 2427, 2433, 2440, 2446, 2452, 2458, 2465, 2471, 2477, 2483, 2490, 2496, 2502, 2509, 2515, 2522, 2528, 2534, 2541, 2547, 2554, 2560, 2567, 2573, 2580, 2586, 2593, 2600, 2606, 2613, 2619, 2626, 2633, 2639, 2646, 2653, 2660, 2666, 2673, 2680, 2687, 2694, 2700, 2707, 2714, 2721, 2728, 2735, 2742, 2749, 2756, 2763, 2770, 2777, 2784, 2791, 2798, 2805, 2812, 2820, 2827, 2834, 2841, 2848, 2856, 2863, 2870, 2877, 2885, 2892, 2899, 2907, 2914, 2922, 2929, 2937, 2944, 2951, 2959, 2967, 2974, 2982, 2989, 2997, 3004, 3012, 3020, 3027, 3035, 3043, 3051, 3058, 3066, 3074, 3082, 3090, 3097, 3105, 3113, 3121, 3129, 3137, 3145, 3153, 3161, 3169, 3177, 3185, 3193, 3201, 3209, 3218, 3226, 3234, 3242, 3250, 3259, 3267, 3275, 3284, 3292, 3300, 3309, 3317, 3326, 3334, 3343, 3351, 3360, 3368, 3377, 3385, 3394, 3403, 3411, 3420, 3429, 3437, 3446, 3455, 3464, 3472, 3481, 3490, 3499, 3508, 3517, 3526, 3535, 3544, 3553, 3562, 3571, 3580, 3589, 3598, 3607, 3616, 3626, 3635, 3644, 3653, 3663, 3672, 3681, 3691, 3700, 3709, 3719, 3728, 3738, 3747, 3757, 3766, 3776, 3785, 3795, 3805, 3814, 3824, 3834, 3844, 3853, 3863, 3873, 3883, 3893, 3903, 3913, 3922, 3932, 3942, 3952, 3963, 3973, 3983, 3993, 4003, 4013, 4023, 4034, 4044, 4054, 4064, 4075, 4085, 4096, 4106, 4116, 4127, 4137, 4148, 4158, 4169, 4180, 4190, 4201, 4212, 4222, 4233, 4244, 4255, 4265, 4276, 4287, 4298, 4309, 4320, 4331, 4342, 4353, 4364, 4375, 4386, 4397, 4408, 4420, 4431, 4442, 4453, 4465, 4476, 4488, 4499, 4510, 4522, 4533, 4545, 4556, 4568, 4580, 4591, 4603, 4615, 4626, 4638, 4650, 4662, 4674, 4685, 4697, 4709, 4721, 4733, 4745, 4757, 4770, 4782, 4794, 4806, 4818, 4830, 4843, 4855, 4867, 4880, 4892, 4905, 4917, 4930, 4942, 4955, 4967, 4980, 4993, 5005, 5018, 5031, 5044, 5056, 5069, 5082, 5095, 5108, 5121, 5134, 5147, 5160, 5173, 5186, 5200, 5213, 5226, 5239, 5253, 5266, 5279, 5293, 5306, 5320, 5333, 5347, 5360, 5374, 5388, 5401, 5415, 5429, 5443, 5457, 5470, 5484, 5498, 5512, 5526, 5540, 5554, 5569, 5583, 5597, 5611, 5625, 5640, 5654, 5668, 5683, 5697, 5712, 5726, 5741, 5755, 5770, 5785, 5799, 5814, 5829, 5844, 5859, 5874, 5889, 5903, 5918, 5934, 5949, 5964, 5979, 5994, 6009, 6025, 6040, 6055, 6071, 6086, 6102, 6117, 6133, 6148, 6164, 6180, 6195, 6211, 6227, 6243, 6259, 6274, 6290, 6306, 6322, 6338, 6355, 6371, 6387, 6403, 6419, 6436, 6452, 6469, 6485, 6501, 6518, 6535, 6551, 6568, 6585, 6601, 6618, 6635, 6652, 6669, 6686, 6703, 6720, 6737, 6754, 6771, 6788, 6806, 6823, 6840, 6858, 6875, 6892, 6910, 6928, 6945, 6963, 6981, 6998, 7016, 7034, 7052, 7070, 7088, 7106, 7124, 7142, 7160, 7178, 7197, 7215, 7233, 7252, 7270, 7288, 7307, 7326, 7344, 7363, 7382, 7400, 7419, 7438, 7457, 7476, 7495, 7514, 7533, 7552, 7571, 7591, 7610, 7629, 7649, 7668, 7688, 7707, 7727, 7746, 7766, 7786, 7806, 7826, 7845, 7865, 7885, 7905, 7926, 7946, 7966, 7986, 8006, 8027, 8047, 8068, 8088, 8109, 8129, 8150, 8171, 8192, 8212, 8233, 8254, 8275, 8296, 8317, 8338, 8360, 8381, 8402, 8424, 8445, 8466, 8488, 8510, 8531, 8553, 8575, 8596, 8618, 8640, 8662, 8684, 8706, 8728, 8751, 8773, 8795, 8817, 8840, 8862, 8885, 8907, 8930, 8953, 8976, 8998, 9021, 9044, 9067, 9090, 9113, 9137, 9160, 9183, 9206, 9230, 9253, 9277, 9300, 9324, 9348, 9371, 9395, 9419, 9443, 9467, 9491, 9515, 9540, 9564, 9588, 9612, 9637, 9661, 9686, 9711, 9735, 9760, 9785, 9810, 9835, 9860, 9885, 9910, 9935, 9960, 9986, 10011, 10036, 10062, 10088, 10113, 10139, 10165, 10191, 10216, 10242, 10268, 10295, 10321, 10347, 10373, 10400, 10426, 10453, 10479, 10506, 10533, 10559, 10586, 10613, 10640, 10667, 10694, 10721, 10749, 10776, 10803, 10831, 10858, 10886, 10914, 10941, 10969, 10997, 11025, 11053, 11081, 11109, 11138, 11166, 11194, 11223, 11251, 11280, 11309, 11337, 11366, 11395, 11424, 11453, 11482, 11511, 11541, 11570, 11599, 11629, 11659, 11688, 11718, 11748, 11778, 11807, 11837, 11868, 11898, 11928, 11958, 11989, 12019, 12050, 12080, 12111, 12142, 12173, 12204, 12235, 12266, 12297, 12328, 12360, 12391, 12423, 12454, 12486, 12518, 12549, 12581, 12613, 12645, 12677, 12710, 12742, 12774, 12807, 12839, 12872, 12905, 12938, 12971, 13003, 13037, 13070, 13103, 13136, 13170, 13203, 13237, 13270, 13304, 13338, 13372, 13406, 13440, 13474, 13508, 13543, 13577, 13612, 13646, 13681, 13716, 13751, 13785, 13821, 13856, 13891, 13926, 13962, 13997, 14033, 14068, 14104, 14140, 14176, 14212, 14248, 14284, 14321, 14357, 14394, 14430, 14467, 14504, 14540, 14577, 14615, 14652, 14689, 14726, 14764, 14801, 14839, 14877, 14914, 14952, 14990, 15028, 15067, 15105, 15143, 15182, 15220, 15259, 15298, 15337, 15376, 15415, 15454, 15493, 15533, 15572, 15612, 15652, 15691, 15731, 15771, 15811, 15852, 15892, 15932, 15973, 16013, 16054, 16095, 16136, 16177, 16218, 16259, 16301, 16342, 16384, 16425, 16467, 16509, 16551, 16593, 16635, 16677, 16720, 16762, 16805, 16848, 16890, 16933, 16976, 17020, 17063, 17106, 17150, 17193, 17237, 17281, 17325, 17369, 17413, 17457, 17502, 17546, 17591, 17635, 17680, 17725, 17770, 17815, 17861, 17906, 17952, 17997, 18043, 18089, 18135, 18181, 18227, 18274, 18320, 18367, 18413, 18460, 18507, 18554, 18601, 18649, 18696, 18743, 18791, 18839, 18887, 18935, 18983, 19031, 19080, 19128, 19177, 19225, 19274, 19323, 19372, 19422, 19471, 19521, 19570, 19620, 19670, 19720, 19770, 19820, 19871, 19921, 19972, 20023, 20073, 20125, 20176, 20227, 20278, 20330, 20382, 20433, 20485, 20537, 20590, 20642, 20695, 20747, 20800, 20853, 20906, 20959, 21012, 21066, 21119, 21173, 21227, 21281, 21335, 21389, 21443, 21498, 21553, 21607, 21662, 21717, 21773, 21828, 21883, 21939, 21995, 22051, 22107, 22163, 22219, 22276, 22332, 22389, 22446, 22503, 22560, 22618, 22675, 22733, 22791, 22849, 22907, 22965, 23023, 23082, 23141, 23199, 23258, 23318, 23377, 23436, 23496, 23556, 23615, 23675, 23736, 23796, 23856, 23917, 23978, 24039, 24100, 24161, 24223, 24284, 24346, 24408, 24470, 24532, 24595, 24657, 24720, 24783, 24846, 24909, 24972, 25036, 25099, 25163, 25227, 25291, 25355, 25420, 25485, 25549, 25614, 25679, 25745, 25810, 25876, 25942, 26007, 26074, 26140, 26206, 26273, 26340, 26407, 26474, 26541, 26609, 26676, 26744, 26812, 26880, 26949, 27017, 27086, 27155, 27224, 27293, 27362, 27432, 27502, 27571, 27642, 27712, 27782, 27853, 27924, 27995, 28066, 28137, 28209, 28281, 28352, 28424, 28497, 28569, 28642, 28715, 28788, 28861, 28934, 29008, 29081, 29155, 29230, 29304, 29378, 29453, 29528, 29603, 29678, 29754, 29829, 29905, 29981, 30057, 30134, 30210, 30287, 30364, 30441, 30519, 30596, 30674, 30752, 30830, 30909, 30987, 31066, 31145, 31224, 31304, 31383, 31463, 31543, 31623, 31704, 31784, 31865, 31946, 32027, 32109, 32190, 32272, 32354, 32436, 32519, 32602, 32684, 32768};


extern signed char direction[2];
extern u8 EFFECTWRITE, EFFECTREAD,EFFECTFILTER;
extern u8 wormdir;
extern u8 wormflag[10];
extern u8 digfilterflag;
extern u8 *datagenbuffer;

int16_t *audio_ptr;

extern const u16 SAMPLE_FREQUENCY;
extern const float Pi;
extern const float PI_2;

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
		*ldst++ = *(src++);
		sz--;
		*(rdst++) = *(src++);
		//		*(rdst++) = 0;
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


u8 fingerdir(u8 *speedmod);

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  extern u8 inp;
  u8 xx,spd;
  u8 mainmode, whichvillager,step;
  int16_t tmp,tmp16,last,lastt,count;
  float FMODW=0.5f;
  static u8 howmanywritevill=1,howmanyreadvill=1,writeoverlay=0,readoverlay=0;
  static u8 whichw=0,whichr=0,delread=0,delwrite=0,readbit=1,readspeed=1,writespeed=1;
  static int16_t dirryw=1,dirry=1;
  static u16 samplepos=0,sampleposw=0,sampleposread=0,startread=0,startwrite=0,wrapread=32767,wrapwrite=32767;
  static u16 counter=0,counterr=0;
  static u16 readbegin=0,readend=32767,readoffset=32768,writebegin=0,writeend=32768,writeoffset=32768;
  extern u8 howmanydatavill;
  extern villagerr village_write[MAX_VILLAGERS+1];
  extern villagerr village_read[MAX_VILLAGERS+1];
  extern villagerr village_filt[MAX_VILLAGERS+1];
  extern villager_generic village_datagen[MAX_VILLAGERS+1];

  //  howmanywritevill=64; // TESTY!
  //  howmanyreadvill=64; // TESTY!

#ifdef TEST_STRAIGHT
  audio_split_stereo(sz, src, left_buffer, right_buffer);
  audio_comb_stereo(sz, dst, left_buffer, right_buffer);
#else

	u16 *buf16 = (u16*) datagenbuffer;
	int16_t *ldst=left_buffer;
	int16_t *rdst=right_buffer;

#ifdef TEST_EEG
	// write buf16 into mono
	for (x=0;x<sz/2;x++){
	  mono_buffer[x]=buf16[samplepos%32768];//-32768;
	  samplepos++;
	}
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

	// TODO all audio!
	// 1- set villagers

	// if fingerdown we set appropriate villager based on mode
	
	xx=fingerdir(&spd);

	if (xx!=5){
	  // which mode are we in?
	  mainmode=adc_buffer[FIFTH]>>8; // 4 bits=16
	  mainmode=4; //datagen TESTY!

	  switch(mainmode){
	  case 0:// WRITE
	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    howmanywritevill=whichvillager+1;
	    if (adc_buffer[SECOND]>10){
	    village_write[whichvillager].start=loggy[adc_buffer[SECOND]]; //as logarithmic
	    }
	    if (adc_buffer[THIRD]>10){
	    village_write[whichvillager].wrap=loggy[adc_buffer[THIRD]]; //as logarithmic
	    }
	    if (adc_buffer[FOURTH]>10){
	    village_write[whichvillager].offset=loggy[adc_buffer[FOURTH]]; // as logarithmic
	    }
	    //	    village_write[whichvillager].effect=adc_buffer[FOURTH]>>4; // 8 bits so far 
	    village_write[whichvillager].dir=xx;
	    village_write[whichvillager].speed=spd&15; // check how many bits is spd? 8 as changed in main.c 
	    village_write[whichvillager].step=(spd&240)>>4;

	    //added for overlap of grains/villagers:

	    if (village_write[whichvillager].dir==2) village_write[whichvillager].dirry=newdirection[wormdir];
	    else if (village_write[whichvillager].dir==3) village_write[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_write[whichvillager].speed;
	    else village_write[whichvillager].dirry=direction[village_write[whichvillager].dir]*village_write[whichvillager].speed;

	    // and if is running already?
	    if (village_write[whichvillager].running==0){
	    if (village_write[whichvillager].dirry>0) village_write[whichvillager].samplepos=village_write[whichvillager].start;
	    else village_write[whichvillager].samplepos=village_write[whichvillager].start+village_write[whichvillager].wrap;
	    }
	    break;


	  case 1:// READ
	    whichvillager=adc_buffer[FIRST]>>6; // 6 bits=64!!!
	    howmanyreadvill=whichvillager+1;
	    if (adc_buffer[SECOND]>10){
	    village_read[whichvillager].start=loggy[adc_buffer[SECOND]]; //as logarithmic
	    }
	    if (adc_buffer[THIRD]>10){
	    village_read[whichvillager].wrap=loggy[adc_buffer[THIRD]]; // as logarithmic
	    }
	    if (adc_buffer[FOURTH]>10){
	    village_read[whichvillager].offset=loggy[adc_buffer[FOURTH]]; // as logarithmic
	    }

	    //	    village_read[whichvillager].effect=adc_buffer[FOURTH]>>4; // 8 bits so far 
	    village_read[whichvillager].dir=xx;
	    village_read[whichvillager].speed=spd&15; // check how many bits is spd? 8 as changed in main.c 
	    village_read[whichvillager].step=(spd&240)>>4;

	    //added for overlap of grains/villagers:

	    if (village_read[whichvillager].dir==2) village_read[whichvillager].dirry=newdirection[wormdir];
	    else if (village_read[whichvillager].dir==3) village_read[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_read[whichvillager].speed;
	    else village_read[whichvillager].dirry=direction[village_read[whichvillager].dir]*village_read[whichvillager].speed;

	    // and if is running already?
	    if (village_read[whichvillager].running==0){
	    
	    if (village_read[whichvillager].dirry>0) village_read[whichvillager].samplepos=village_read[whichvillager].start;
	    else village_read[whichvillager].samplepos=village_read[whichvillager].start+village_read[whichvillager].wrap;
	    }
	    break;

	  case 2: // WRITEMODE compression???
	    writeoverlay=adc_buffer[FIRST]>>9; // 8 possibles 
	    writebegin=loggy[adc_buffer[SECOND]]; //as logarithmic
	    writeend=loggy[adc_buffer[THIRD]]; //as logarithmic
	    writeoffset=loggy[adc_buffer[FOURTH]];
	    writespeed=spd&15; // check how many bits is spd? 8 as changed in main.c 
	    if (xx==0) dirryw=-((spd&240)>>4);
	    else if (xx==1) dirryw=((spd&240)>>4);
	    else if (xx==2) dirryw=newdirection[wormdir];
	    else dirryw=direction[adc_buffer[DOWN]&1]*((spd&240)>>4);
	    if (dirryw>0) counter=writebegin;
	      else counter=writeend+writebegin;
	    break;

	  case 3: // READMODE compression???
	    readoverlay=adc_buffer[FIRST]>>9; // 8 possibles 
	    readbit=(adc_buffer[FIRST]>>8)&1; // lowest bit
	    readbegin=loggy[adc_buffer[SECOND]]; //as logarithmic
	    readend=loggy[adc_buffer[THIRD]]; //as logarithmic
	    readoffset=loggy[adc_buffer[FOURTH]];
	    readspeed=spd&15; // check how many bits is spd? 8 as changed in main.c 
	    if (xx==0) dirry=-((spd&240)>>4);
	    else if (xx==1) dirry=((spd&240)>>4);
	    else if (xx==2) dirry=newdirection[wormdir];
	    else dirry=direction[adc_buffer[DOWN]&1]*((spd&240)>>4);
	    if (dirry>0) counterr=readbegin;
	    else counterr=readbegin+readend;
	    break;

	    // EXPERIMNET?TESTY!

	  case 4: // DATAGEN villagers at moment just CA 0-10; %11

	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    howmanydatavill=whichvillager+1;
	    if (adc_buffer[SECOND]>10){
	    village_datagen[whichvillager].start=loggy[adc_buffer[SECOND]]; //as logarithmic
	    }
	    if (adc_buffer[THIRD]>10){
	    village_datagen[whichvillager].wrap=loggy[adc_buffer[THIRD]]; //as logarithmic
	    }
	    if (adc_buffer[FOURTH]>10){
	      village_datagen[whichvillager].cpu=adc_buffer[FOURTH]>>8;
	    }
	    village_datagen[whichvillager].dir=xx;
	    village_datagen[whichvillager].speed=spd&15; // check how many bits is spd? 8 as changed in main.c 
	    village_datagen[whichvillager].step=(spd&240)>>4;

	  }
	}
	//////	
	// process villagers - first attempt sans effects

	// READ!
	readoverlay=0; // TESTY!
	readbit=1;

	if (readbit){
	switch(readoverlay){
	case 0:
	  for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){
	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      buf16[village_read[x].samplepos%32768]=tmp+32768;
	      audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	      }
	break;
	case 1:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){
	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      buf16[village_read[x].samplepos%32768]|=tmp+32768;
	      audio_buffer[village_read[x].samplepos%32768]|=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 2:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){
	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      buf16[village_read[x].samplepos%32768]+=tmp+32768;
	      audio_buffer[village_read[x].samplepos%32768]+=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 3:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      buf16[village_read[x].samplepos%32768]^=tmp+32768;
	      audio_buffer[village_read[x].samplepos%32768]^=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 4:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      if (buf16[village_read[x].samplepos%32768]==0) buf16[village_read[x].samplepos%32768]=tmp+32768;
	      else if ((tmp+32768)!=0) buf16[village_read[x].samplepos%32768]%=(tmp+32768);
	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      else if (tmp16!=0) audio_buffer[village_read[x].samplepos%32768]%=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 5:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      if (buf16[village_read[x].samplepos%32768]==0) buf16[village_read[x].samplepos%32768]=tmp+32768;
	      else if ((tmp+32768)!=0) buf16[village_read[x].samplepos%32768]*=(tmp+32768);
	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      else if (tmp16!=0) audio_buffer[village_read[x].samplepos%32768]*=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 6:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      if (buf16[village_read[x].samplepos%32768]==0) buf16[village_read[x].samplepos%32768]=tmp+32768;
	      else if ((tmp+32768)!=0) buf16[village_read[x].samplepos%32768]&=(tmp+32768);
	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      else if (tmp16!=0) audio_buffer[village_read[x].samplepos%32768]&=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 7:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  last=0;lastt=0;
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      if (tmp>lastt) buf16[village_read[x].samplepos%32768]=tmp+32768;
	      lastt=tmp;
	      if (tmp16>last) audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      last=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);

	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	} // end switch
	}
	else{ // readbit
	switch(readoverlay){
	case 0:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      audio_buffer[village_read[x].samplepos%32768]=tmp;

	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 1:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      audio_buffer[village_read[x].samplepos%32768]|=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 2:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      audio_buffer[village_read[x].samplepos%32768]+=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 3:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      audio_buffer[village_read[x].samplepos%32768]^=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 4:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp;
	      else if (tmp!=0) audio_buffer[village_read[x].samplepos%32768]%=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 5:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp;
	      else if (tmp!=0) audio_buffer[village_read[x].samplepos%32768]*=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 6:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp;
	      else if (tmp!=0) audio_buffer[village_read[x].samplepos%32768]&=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 7:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>=readend) {
	    counterr=readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<=readbegin) {
	    counterr=readend+readbegin;
	    for (u8 x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  last=0;
	  for (u8 x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      if (last>tmp) audio_buffer[village_read[x].samplepos%32768]=tmp;
	      last=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	}
	}
	// END of all READS!

   	// WRITE!

	//	writeoverlay=0;
	switch(writeoverlay){// 8 options
	case 0:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>=writeend) {
	    counter=writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<=writebegin) {
	    counter=writeend+writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      mono_buffer[xx]=audio_buffer[village_write[x].samplepos%32768];
	      //village_write[x].samplepos++;

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>=0)
	      {
		village_write[x].samplepos+=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	    }
	    }
	    }
      if (++delwrite>writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 1:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>=writeend) {
	    counter=writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<=writebegin) {
	    counter=writeend+writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      mono_buffer[xx]|=audio_buffer[village_write[x].samplepos%32768];

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos+=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 2:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>=writeend) {
	    counter=writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<=writebegin) {
	    counter=writeend+writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      mono_buffer[xx]+=audio_buffer[village_write[x].samplepos%32768];
	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos+=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 3:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>=writeend) {
	    counter=writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<=writebegin) {
	    counter=writeend+writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      mono_buffer[xx]^=audio_buffer[village_write[x].samplepos%32768];
	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 4:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>=writeend) {
	    counter=writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<=writebegin) {
	    counter=writeend+writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      if (mono_buffer[xx]!=0){
		if (audio_buffer[village_write[x].samplepos%32768]!=0) mono_buffer[xx]%=(audio_buffer[village_write[x].samplepos%32768]);}
		  else mono_buffer[xx]=(audio_buffer[village_write[x].samplepos%32768]);

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 5:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>=writeend) {
	    counter=writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<=writebegin) {
	    counter=writeend+writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      if (mono_buffer[xx]!=0){
		mono_buffer[xx]*=audio_buffer[village_write[x].samplepos%32768];
	      }
		  else mono_buffer[xx]=(audio_buffer[village_write[x].samplepos%32768]);

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 6:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>=writeend) {
	    counter=writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<=writebegin) {
	    counter=writeend+writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (u8 x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      if (mono_buffer[xx]!=0){
		mono_buffer[xx]&=audio_buffer[village_write[x].samplepos%32768];
	      }
		  else mono_buffer[xx]=(audio_buffer[village_write[x].samplepos%32768]);

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 7:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>=writeend) {
	    counter=writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<=writebegin) {
	    counter=writeend+writebegin;
	    for (u8 x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  last=0;
	  for (u8 x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      /*	      if (mono_buffer[xx]!=0){
		mono_buffer[xx]=(float32_t)mono_buffer[xx]*FMODW*(float32_t)audio_buffer[village_write[x].samplepos%32768];
	      }
		  else mono_buffer[xx]=(audio_buffer[village_write[x].samplepos%32768]);
	      */
	      if (audio_buffer[village_write[x].samplepos%32768]>last) mono_buffer[xx]=audio_buffer[village_write[x].samplepos%32768];

	      last=audio_buffer[village_write[x].samplepos%32768];

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;

	}
	/// end of ALL WRITES!

 
	// process HW in same time-scale as samples
	// so step will be 32 sample step (BUFF_SIZE/4)

	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);

#ifdef PCSIM
//    for (x=0;x<sz/2;x++){
//         printf("%c",mono_buffer[x]);
// 	   }
#endif

      /////////////////////////////////////

  // 4-hardware operations shifted here
  // question of granularity

	// TESTY!
	//	inp=2;
	//	dohardwareswitch(0,0);

#endif // for test eeg
#endif // for straight

}
