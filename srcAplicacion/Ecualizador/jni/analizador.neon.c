#include "analizador.h"

#define SALTO 32
//raiz sexta de 4, 6 bandas por banda
#define BANDASPORBANDA 1.26


void inicializarAnalizador( analizador *const a,
                            const int longitudVentanaInFrames,
                            const int SR,
                            const int channels,
                            const int cantBandas )
{
    a->longitudfft = longitudVentanaInFrames * 2;
    a->longitudVentanaFrames = longitudVentanaInFrames;
    a->canales = channels;
    a->cantidadEntrada = a->longitudVentanaFrames * a->canales;
    a->sampleRate = SR;
    a->cantidadBandas = cantBandas;
    a->ventanasPromediadas = 0;

    a->ventana = (float *) malloc( sizeof(float) * a->longitudVentanaFrames );
    // padearemos con ceros para obtener mas precision usanod una fft mas larga
    a->temporalSampleada = (float *) malloc( sizeof(float) * a->cantidadEntrada * 2);
    // como no tocamos la segunda mitad este padeo puede hacerse una unica vez:
    memset( (float *) (a->temporalSampleada + a->cantidadEntrada), 0, sizeof(float) * a->cantidadEntrada );

    a->espectroAN_L = (COMPLEX *) malloc( sizeof(COMPLEX) * (a->longitudfft + 1) );
    a->espectroAN_R = (COMPLEX *) malloc( sizeof(COMPLEX) * (a->longitudfft + 1) );
    a->bandas = (float *) malloc( sizeof(float) * a->cantidadBandas );

    memset( (void *) a->bandas, 0, sizeof(float) * a->cantidadBandas );

    inicializarFFT( &(a->transformador), a->longitudfft, a->canales );

    crearHamming( a->ventana, a->longitudVentanaFrames, CLASSIC_HAMMING );
}

void analizar( analizador *const a, void *const sampleada )
{
    int longfft = a->longitudfft;
    int longitudVentanaFrames = a->longitudVentanaFrames;
    int cantidadEntrada = a->cantidadEntrada;
    int cantBandas = a->cantidadBandas;
    float *bands = a->bandas;
    int samplerate = a->sampleRate;
    int j;
    int promediarN;
    float frecuencia;
    float bandaActualTMP;
    int indexEspectro = 0;

    // rangos analizados (cada uno dividido en 1 o mas bandas (cuatro si usamos sqrt(2)))
    // (0, 128), (128, 512), (512, 2048), (2048, 8192), (8192, +inf)
    
 
    // tomamos la entrada
#if ASMOPT
    getFlotasInt16ASM( (int16_t *) sampleada, a->temporalSampleada, cantidadEntrada );
#else
    getFlotasInt16( (int16_t *) sampleada, a->temporalSampleada, cantidadEntrada );
#endif


    if ( a->canales == MONO ) {        // MONO
#if ASMOPT
        multiplicarVentanasMonoASM( a->temporalSampleada, a->ventana, longitudVentanaFrames );
#else
        multiplicarVentanasMono( a->temporalSampleada, a->ventana, longitudVentanaFrames );
#endif
        realFFT( &(a->transformador), a->temporalSampleada, a->espectroAN_L, NULL );

        for ( j=0 ; j<cantBandas-1 ; j++ ) {

            // calculo la frecuencia en que estoy parado
            frecuencia = (float)indexEspectro*(float)samplerate/(float)longfft;

            // por ahora no sume ninguna
            promediarN = 0;
            bandaActualTMP = 0;

            // mientras no me pase a la otra banda
            while ( frecuencia < SALTO*pow(BANDASPORBANDA,j+1) && indexEspectro<longfft/2 ) {
                // sumo la energia de aca
                bandaActualTMP += AMPLITUDREAL( a->espectroAN_L[indexEspectro]);
                // me muevo una posicion del vector espectro
                indexEspectro++;
                // ahora tengo que promediar uno mas
                promediarN++;

                // calculo la frecuencia en que estoy parado
                frecuencia = (float)indexEspectro*(float)samplerate/(float)longfft;
            }
            // hago el promedio de esta banda
            bands[j] += bandaActualTMP / (float) promediarN;
        }

        promediarN = 0;
        bandaActualTMP = 0;

        while ( frecuencia < samplerate/2. && indexEspectro<longfft/2 ) {
            // sumo la energia de aca
            bandaActualTMP += AMPLITUDREAL( a->espectroAN_L[indexEspectro]);
            // me muevo una posicion del vector espectro
            indexEspectro++;
            // ahora tengo que promediar uno mas
            promediarN++;

            // calculo la frecuencia en que estoy parado
            frecuencia = (float)indexEspectro*(float)samplerate/(float)longfft;
        }
        // hago el promedio de esta banda
        bands[cantBandas-1] += bandaActualTMP / (float) promediarN;


    } else {                 // STEREO
#if ASMOPT
        multiplicarVentanasStereoASM( a->temporalSampleada, a->ventana, longitudVentanaFrames );
#else
        multiplicarVentanasStereo( a->temporalSampleada, a->ventana, longitudVentanaFrames );
#endif
        realFFT( &(a->transformador), a->temporalSampleada, a->espectroAN_L, a->espectroAN_R );

        for ( j=0 ; j<cantBandas-1 ; j++ ) {

            // por ahora no sume ninguna
            promediarN = 0;
            bandaActualTMP = 0;

            // calculo la frecuencia en que estoy parado
            frecuencia = (float)indexEspectro*(float)samplerate/(float)longfft;
            // mientras no me pase a la otra banda
            while ( frecuencia < SALTO*pow(BANDASPORBANDA,j+1) && indexEspectro<longfft/2 ) {
                // sumo la energia de aca
                // promedio de amplitud left y right
                bandaActualTMP += (AMPLITUDREAL( a->espectroAN_L[indexEspectro]) + AMPLITUDREAL( a->espectroAN_R[indexEspectro]) ) / 2.0;
                // me muevo una posicion del vector espectro
                indexEspectro++;
                // ahora tengo que promediar uno mas
                promediarN++;

                // calculo la frecuencia en que estoy parado
                frecuencia = (float)indexEspectro*(float)samplerate/(float)longfft;
            }

            // hago el promedio de esta banda
            bands[j] += bandaActualTMP / (float) promediarN;
        }

        promediarN = 0;
        bandaActualTMP = 0;

        while ( frecuencia < samplerate/2. && indexEspectro<longfft/2 ) {
            // sumo la energia de aca
            // promedio de amplitud left y right
            bandaActualTMP += (AMPLITUDREAL( a->espectroAN_L[indexEspectro]) + AMPLITUDREAL( a->espectroAN_R[indexEspectro]) ) / 2.0;
            // me muevo una posicion del vector espectro
            indexEspectro++;
            // ahora tengo que promediar uno mas
            promediarN++;

            // calculo la frecuencia en que estoy parado
            frecuencia = (float)indexEspectro*(float)samplerate/(float)longfft;
        }
        // hago el promedio de esta banda
        bands[cantBandas-1] += bandaActualTMP / (float) promediarN;

    }
    // analice una ventana mas
    a->ventanasPromediadas += 1;
}

void dameAnalisis( analizador *const a, float *const espectro )
{
    int i;
    int cantBandas = a->cantidadBandas;
    int promedio = a->ventanasPromediadas;
    float *bands = a->bandas;

    for( i=0 ; i<cantBandas ; ++i ) {
        espectro[i] = DB( bands[i] / (float) promedio);
    }

    memset( a->bandas, 0, sizeof(float) * cantBandas );
    a->ventanasPromediadas = 0;
}

void limpiarAN( analizador *const a )
{
    // limpio el arreglo del analisis
    memset( a->bandas, 0, sizeof(float) * a->cantidadBandas );
}

void destruirAnalizador( analizador *const a )
{
    free( a->ventana );
    free( a->temporalSampleada );
    free( a->espectroAN_L );
    free( a->espectroAN_R );
    free( a->bandas );

    destruirFFT( &(a->transformador) );
}
