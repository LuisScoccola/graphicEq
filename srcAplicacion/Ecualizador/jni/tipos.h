#ifndef __TIPOS_H
#define __TIPOS_H


#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h> 


#define ASMOPT 1


#define PI 3.1415926535897932384626433832

#define COMPLEJO 0
#define MONO 1
#define STEREO 2


#define AMPLITUD(C) ( sqrt( pow((C).real, 2) + pow((C).imag, 2) ) )
// supongiendo 2^12=4096 samples por ventana hayq eu dividir por 32=sqrt(4096)/2
// suponiendo 2^13=8192 .... dividir por 45.2548
#define AMPLITUDREAL(C) ( sqrt( pow((C).real, 2) + pow((C).imag, 2) ) / 32 )
#define FASE(C) ( atan2( (C).imag, (C).real ) )
#define DB(X) ( 20 * log10(X) )
#define IDB(X) ( pow( 10, (X)/20 ) )


//////// DEBUG ////////
#define DEBUG 1

#if DEBUG

#include <android/log.h>
char BUFFER_PARA_STRING[128];
#define PRINT_LOG(tag,formato,numero) \
	sprintf( BUFFER_PARA_STRING, (formato), (numero) ); \
	__android_log_write(ANDROID_LOG_ERROR, (tag), BUFFER_PARA_STRING)

#endif
///// FIN DEBUG ///////


typedef struct COMPLEX {
	float real;
	float imag;
} COMPLEX;

#endif
