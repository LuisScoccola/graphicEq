#ifndef __ANALIZADOR_H
#define __ANALIZADOR_H

#include "tipos.h"
#include "realdft.h"
#include "funciones.h"

typedef struct analizadorDeEspectro {

	int longitudfft;
    int longitudVentanaFrames;
	int canales;
    int cantidadEntrada;
	int sampleRate;
	int cantidadBandas;
	int ventanasPromediadas;
	
	fft transformador;
	float *ventana;
	float *temporalSampleada;
	COMPLEX *espectroAN_L;
	COMPLEX *espectroAN_R;
	float *bandas;

} analizador;


void inicializarAnalizador( analizador *const a, const int longitudVentanaInFrames, const int SR, const int canales, int cantidadBandas );
void analizar( analizador *const a, void *const sampleada );
void dameAnalisis( analizador *const a, float *const espectro );
void limpiarAN( analizador *const a );
void destruirAnalizador( analizador *const a );

#endif
