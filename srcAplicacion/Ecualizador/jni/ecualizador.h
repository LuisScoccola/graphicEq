#ifndef __ECUALIZADOR_H
#define __ECUALIZADOR_H

#include "tipos.h"
#include "realdft.h"
#include "funciones.h"

#define SPLINE_CUBICO 0
#define ESCALERA 1

typedef struct ecualizador {

	int longitudfft;
	int longitudVentana;
	int sampleRate;
	int canales;
	int tipoFiltro;

	int cantidadMitadEntrada;	// cantidad de elementos en la mitad del arreglo de entrada
	size_t bytesMitadEntrada;	// cantidad en bytes de la mitad del arreglo de entrada
	int offsetCuartoFlotas;	// offset de un cuarto de los arreglos usados internamente
	size_t bytesCuartoFlotas;	// cantidad en bytes de ese offset

	fft transAudio;
	fft transFiltro;
	fft transDiseno;
	fft transLPF;

	float *ventana;
	COMPLEX *espectroEQ_L;
	COMPLEX *espectroEQ_R;
	float *temporalAudio;
	float *temporalEcualizado;
	float *temporalFFT1;
	float *temporalFFT2;
	float *temporalSuma;

	int tamanoFiltroDiseno;

    // usamos dos arreglos para poder
    // cuelizar en tiempo real
    // respFrec es solo usado por el ecualizador
    // quien nno sabe si esta suando A o B
	COMPLEX *respFrec;
	COMPLEX *respFrecA;
	COMPLEX *respFrecB;

	COMPLEX *respuestaFrecAuxFiltro;
	COMPLEX *ondaAuxFiltro;
	COMPLEX *ondaFiltro;
	float *ventanaFiltro;

} eq;

void inicializarEQ( eq *const ecualizador, const int longitudfft, const int SR, const int canales );
void actualizarFiltro( eq *const e, const float *const parametros );
void limpiarEQ( eq *const ecualizador );
void ecualizar( eq *const ecualizador, void *const sampleada );
void ultimoCachito( eq *const ecualizador, void *const sampleada );
void ecualizarYEscribirCancion( eq *const e, const char *const archivoDeEntrada );
void destruirEQ( eq *const ecualizador );

#endif
