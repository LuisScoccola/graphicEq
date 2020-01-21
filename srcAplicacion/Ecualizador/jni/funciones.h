#ifndef __FUNCIONES_H
#define __FUNCIONES_H

#include "tipos.h"
#include "realdft.h"


// a partir de una frecuencia devuelve el lugar en la recta x en que se debe evaluar el plinomio
#define LOGdiezDEcuatro 0.60206
#define GETX( frecuencia ) ( ((frecuencia)==0)? 0 : log10( (frecuencia)/64.0 ) / LOGdiezDEcuatro )



#define CLASSIC_HAMMING 0.54, 0.46
#define HANN 0.5, 0.5
#define CLASSIC_BLACKMAN 0.42659, 0.49656, 0.076849


//////////////// globales para dithering ///////////

float int16_max_float;
//rectangular-PDF random numbers
int16_t r1, r2;
int16_t out;
// tamano de palabra
float w;
float wi;
// amplitud del dither
float d;
// para remover dc offset
float o;
//error feedback buffers
float s1, s2;
// noise shaping (poner en 0. para no shaping)
float s;
float in, tmp;
// para guardar rands
float *randoms;

////////////// fin globales para dithering ////////

void inicializarDither( int longitud_rand );

void destruirDither();

void multiplicarEspectros( COMPLEX *const esp1, const COMPLEX *const esp2, const int tamano );

void sumarSenales( float *const s1, const float *const s2, const int tamano );

void getFlotasInt16( const int16_t *const i, float *const d, const int tamano );

void getIntsInt16( int16_t *const i, const float *const d, const int tamano );

void getIntsInt16_dither( int16_t *const i, const float *const d, const int tamano );

void multiplicarVentanasMono( float *const v1, const float *const v2, int tamano);

void multiplicarVentanasStereo( float *const v1, const float *const v2, int tamano);

void memCopia( void *, const void *, size_t );

void memSeteado( void *, int, size_t );

void crearBartlett( float *const ventana, const int tamano );

void crearHamming( float *const ventana, const int tamano, const float a, const float b );

void crearBlackman( float *const ventana, const int tamano, const float a0, const float a1, const float a2 );

void crearSincFunction( float *const ventana, const float corte, const int tamano );

void crearLPF( const float frecCorte, COMPLEX *const respFrec, const int tamanoFiltro, const int cantidadSamples, const float sampleRate, fft *const transformador );

void crearHPF( const float frecCorte, COMPLEX *const respFrec, const int tamanoFiltro, const int cantidadSamples, const float sampleRate, fft *const transformador );

void crearBPF( const float frecCorte, const float Q, COMPLEX *const respFrec, const int tamanoFiltro, const int cantidadSamples, const float sampleRate, fft *const transformador );

void escalera( const float *const parametros, COMPLEX *const respFrec, const int samplerate, const int cantidadSamples );

void splineCubico( const float *const parametros, COMPLEX *const respFrec, const int samplerate, const int cantidadSamples );


//////////// ASM

void sumarSenalesASM( float *const senal1,
                      const float *const senal2,
                      const int tamano );

void multiplicarVentanasMonoASM( float *const v1,
                                 const float *const v2,
                                 const int tamano);

void multiplicarVentanasStereoASM( float *const v1,
                                   const float *const v2,
                                   const int tamano);

void multiplicarEspectrosASM( COMPLEX *const esp1,
                              const COMPLEX *const esp2,
                              const int tamano );

void getFlotasInt16ASM( const int16_t *const ints,
                        float *const floats,
                        const int tamano );

void getIntsInt16_ditherASM( int16_t *const ints,
                             const float *const flotas,
                             const int tamano );

#endif
