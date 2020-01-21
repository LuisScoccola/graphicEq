#ifndef __REALDFT_H
#define __REALDFT_H

#include "tipos.h"

typedef struct fastFourierTransform {

	int canales;
	int longitud;
	int longitudArreglo;

	COMPLEX *temporalComplex;
	COMPLEX *arregloTemp;

	float *tablaSenos;
	float *tablaCosenos;

	COMPLEX *A;
	COMPLEX *B;
	COMPLEX *IA;
	COMPLEX *IB;

} fft;


void inicializarFFT( fft *const transformador, const int longitudArray, const int canales );
void realFFT( fft *const , float *input, COMPLEX *outputL, COMPLEX *outputR );
void realIFFT( fft *const , float *output, COMPLEX *inputL, COMPLEX *inputR );
void complexFFT( fft *const , COMPLEX *input, COMPLEX *output );
void complexIFFT( fft *const , COMPLEX *output, COMPLEX *input);
void destruirFFT( fft *const transformador );

///// ASM
void bucleMedioFftASM(COMPLEX **pares, int salto, COMPLEX **resAct1, COMPLEX **resAct2, float *twiddleR, float *twiddleL ); 

void splitMonoASM( COMPLEX *X,COMPLEX *A,COMPLEX *B,COMPLEX *espectro,int longitud );

void bucleMedioSplitStereoASM( COMPLEX *eL, COMPLEX *eR, COMPLEX *X, int longitud);

void bucleEspectroMonoASM( COMPLEX *espectro, int longitud );

void rIFFTyaomASM_1( COMPLEX *tempComplex, int longitud );

void rIFFTyaomASM_2( COMPLEX *x, int longitud );

void rIFFTyaosASM_1( COMPLEX *x, COMPLEX *eL, COMPLEX *eR, int longitud );

void rIFFTyaosASM_2( COMPLEX *x, COMPLEX *tmpComplex, int longitud );



#endif
