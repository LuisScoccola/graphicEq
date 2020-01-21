#include "ecualizador.h"

    /*    TODO
     *
     *    guardar todas las multiplicaciones en variables al inicializar
     *    el ecualizador
     *
     *    en actualizar ventana hay algo que se pude sustituir por
     *    mutliplicar ventanas????
     *
     */


void inicializarEQ( eq *const e, const int longitudVentanaInFrames, const int SR, const int channels )
{
    e->longitudfft = longitudVentanaInFrames*2;
    e->longitudVentana = longitudVentanaInFrames;
    e->sampleRate = SR;
    e->canales = channels;

    e->ventana = (float *) malloc( sizeof(float) * e->longitudVentana);
    // ESTOS "+ 1" hay que ver si van!!
    e->espectroEQ_L = (COMPLEX *) malloc( sizeof(COMPLEX) * (e->longitudfft + 1) );
    e->espectroEQ_R = (COMPLEX *) malloc( sizeof(COMPLEX) * (e->longitudfft + 1) );

    e->cantidadMitadEntrada = e->longitudVentana * e->canales / 2;
    e->bytesMitadEntrada = sizeof(int16_t) * e->cantidadMitadEntrada;
    e->offsetCuartoFlotas = e->cantidadMitadEntrada;
    e->bytesCuartoFlotas = sizeof(float) * e->cantidadMitadEntrada;

    e->temporalAudio = (float *) malloc( e->bytesCuartoFlotas );
    e->temporalEcualizado = (float *) malloc( e->bytesCuartoFlotas * 3 ); 
    e->temporalFFT1 = (float *) malloc( e->bytesCuartoFlotas * 4 ); 
    e->temporalFFT2 = (float *) malloc( e->bytesCuartoFlotas * 4 );
    e->temporalSuma = (float *) malloc( e->bytesCuartoFlotas * 5 );

    inicializarFFT( &(e->transAudio), e->longitudfft, e->canales );


//// Para respuesta de frecuencia del filtro
    e->tipoFiltro = SPLINE_CUBICO;

    e->tamanoFiltroDiseno = e->longitudfft * 4;

    inicializarFFT( &(e->transDiseno), e->tamanoFiltroDiseno, COMPLEJO );
    inicializarFFT( &(e->transFiltro), e->longitudfft, COMPLEJO );
    inicializarFFT( &(e->transLPF), e->longitudfft, MONO );

    // respuesta real
    e->respFrecA = (COMPLEX *) malloc( sizeof(COMPLEX) * (e->longitudfft + 1) );
    // necesario para aplicar un LPF para sacar posibles apmplificaciones de
    // agudos mas alla de la frec de ny.
    e->respFrecB = (COMPLEX *) malloc( sizeof(COMPLEX) * (e->longitudfft + 1) );
    e->respFrec = e->respFrecA;

    e->respuestaFrecAuxFiltro = (COMPLEX *) malloc( sizeof(COMPLEX) * e->tamanoFiltroDiseno );
    e->ondaAuxFiltro = (COMPLEX *) malloc( sizeof(COMPLEX) * e->tamanoFiltroDiseno );
    e->ondaFiltro = (COMPLEX *) malloc( sizeof(COMPLEX) * e->tamanoFiltroDiseno );

    e->ventanaFiltro = (float *) malloc( sizeof(float) * e->longitudfft );
//// Fin filtro



//// Ventana para tomar pedazos de cancion y ventana para eqGrafico
//    crearBlackman( e->ventana, e->longitudVentana, CLASSIC_BLACKMAN );
//    crearHamming( e->ventana, e->longitudVentana, HANN );
    crearBartlett( e->ventana, e->longitudVentana );

    crearBlackman( e->ventanaFiltro, e->longitudfft, CLASSIC_BLACKMAN );
//// Fin ventana


    float parametros[5] = { 0, 0, 0, 0, 0 };
    actualizarFiltro( e, parametros );

    // inicializamos los valores del dither
    inicializarDither( 2 * e->cantidadMitadEntrada );

}

void actualizarFiltro( eq *const e, const float *const parametros )
{
    int cantidadSamples = e->longitudfft;
    int samplesDiseno = e->tamanoFiltroDiseno;
    int samplerate = e->sampleRate;
    
    fft *transformadorDiseno = &e->transDiseno;
    fft *transformadorFiltro = &e->transFiltro;
    
    COMPLEX *respuestaFrecA = e->respFrecA;

    COMPLEX *respuestaFrecAux = e->respuestaFrecAuxFiltro;
    COMPLEX *ondaAux = e->ondaAuxFiltro;
    COMPLEX *onda = e->ondaFiltro;
    float *ventana = e->ventanaFiltro;

    // declarar
    //    crearCustomResponse( parametros, 0, e->respFrec, e->longitudfft, e->tamanoFiltroDiseno, e->sampleRate, &(e->transDiseno), &(e->transFiltro), e->respuestaFrecAuxFiltro, e->ondaAuxFiltro, e->ondaFiltro, e->ventanaFiltro );


    splineCubico( parametros, respuestaFrecAux, samplerate, samplesDiseno );

    complexIFFT( transformadorDiseno, ondaAux, respuestaFrecAux );

//    ROTAMOS
    memcpy( onda+cantidadSamples/2, ondaAux, sizeof(COMPLEX)*(samplesDiseno-cantidadSamples/2) );
    memcpy( onda, ondaAux+samplesDiseno-cantidadSamples/2, sizeof(COMPLEX)*(cantidadSamples/2) );

//    VENTANEAMOS
//    esto se puede hacer con un multiplicar ventanas?
    int i;
    for ( i=0 ; i<cantidadSamples; ++i ) {
        onda[i].real = onda[i].real * ventana[i];
        onda[i].imag = onda[i].imag * ventana[i];
    }

    // escribimos la respuesta en frecuencia lograda a parir del spline
    complexFFT( transformadorFiltro, onda, respuestaFrecA );
    


    /*
    if ( respuestaFrec == respuestaFrecA ) {
    //    TRUNCAMOS, es decir solo usamos los primeros cantidadSamples samples
    //    aca estamos modificando el puntero y si no es suficientemente
    //    atomico esto podria explotar
        complexFFT( transformadorFiltro, onda, respuestaFrecB );
        e->respFrec = respuestaFrecB;
    } else {
        complexFFT( transformadorFiltro, onda, respuestaFrecA );
        e->respFrec = respuestaFrecA;
    }
    */

/*
    //testeando la respuesta en frecuencia:
	FILE *graphicAmp = fopen( "/mnt/sdcard/EQgraficoAmp.dat", "w" );
	FILE *graphicPhase = fopen( "/mnt/sdcard/EQgraficoPhase.dat", "w" );
	for ( i=0 ; i<cantidadSamples; i++ ) {
		float frecuencia = 44100 * i / cantidadSamples;
		fprintf( graphicAmp, "%f\t%f\n", frecuencia, DB(AMPLITUD((e->respFrec)[i])) );
		fprintf( graphicPhase, "%f\t%f\n", frecuencia, FASE((e->respFrec)[i]) );
	}
	fclose( graphicAmp );
	fclose( graphicPhase );
*/

}

void limpiarEQ( eq *const e )
{
    memset( e->temporalAudio, 0, e->bytesCuartoFlotas );
    memset( e->temporalEcualizado, 0, e->bytesCuartoFlotas * 3 );
}

void ecualizar( eq *const e, void *const sampleada )
{
    int offsetCuartoFlotas = e->offsetCuartoFlotas;
    size_t bytesCuartoFlotas = e->bytesCuartoFlotas;
    int cantidadMitadEntrada = e->cantidadMitadEntrada;

// construyo arreglo para SEGUNDO fft
    // copio toda la ventana para realizar el segundo fft
#if ASMOPT
    getFlotasInt16ASM( (int16_t *) sampleada, e->temporalFFT2, cantidadMitadEntrada*2 );
#else
    getFlotasInt16( (int16_t *) sampleada, e->temporalFFT2, cantidadMitadEntrada*2 );
#endif

    // padeo el resto con 0's para realizar una conv aciclica
    memset( e->temporalFFT2+2*offsetCuartoFlotas, 0, 2*bytesCuartoFlotas );


// construyo arrelgo para PRIMER fft
    // copio la segunda mitad que guarde en la ecualizacion anterior al primer FFT
    memcpy( e->temporalFFT1, e->temporalAudio, bytesCuartoFlotas );

    // copio la primera mitad en el arreglo para realizar el primer fft
    memcpy( e->temporalFFT1+offsetCuartoFlotas, e->temporalFFT2 , bytesCuartoFlotas );

    // padeo el resto con 0's para realizar una conv aciclica
    memset( e->temporalFFT1+2*offsetCuartoFlotas, 0, 2*bytesCuartoFlotas );

    // guardo la segunda mitad para el proximo fft
    memcpy( e->temporalAudio, e->temporalFFT2+offsetCuartoFlotas, bytesCuartoFlotas );

    
    if ( e->canales == MONO ) {    ///////////////////////////// MONO ////////////////////////////////

        // multiplico por ventana
#if ASMOPT
        multiplicarVentanasMonoASM( e->temporalFFT1, e->ventana, e->longitudVentana );
        multiplicarVentanasMonoASM( e->temporalFFT2, e->ventana, e->longitudVentana );
#else
        multiplicarVentanasMono( e->temporalFFT1, e->ventana, e->longitudVentana );
        multiplicarVentanasMono( e->temporalFFT2, e->ventana, e->longitudVentana );
#endif

        // realizo fft1
        realFFT( &(e->transAudio), e->temporalFFT1, e->espectroEQ_L, NULL );
        // ecualizo
#if ASMOPT
        multiplicarEspectrosASM( e->espectroEQ_L, e->respFrec, e->longitudfft+1 );
#else
        multiplicarEspectros( e->espectroEQ_L, e->respFrec, e->longitudfft+1 );
#endif
        // retorno al dominio de las ondas
        realIFFT( &(e->transAudio), e->temporalFFT1, e->espectroEQ_L, NULL );

        // realizo fft2
        realFFT( &(e->transAudio), e->temporalFFT2, e->espectroEQ_L, NULL );
        // ecualizo
#if ASMOPT
        multiplicarEspectrosASM( e->espectroEQ_L, e->respFrec, e->longitudfft+1 );
#else
        multiplicarEspectros( e->espectroEQ_L, e->respFrec, e->longitudfft+1 );
#endif
        // retorno al dominio de las ondas
        realIFFT( &(e->transAudio), e->temporalFFT2, e->espectroEQ_L, NULL );

// construyo resultado y temporalEcualizado:

        // tomo lo que ecualize anteriormente
        memcpy( e->temporalSuma, e->temporalEcualizado, bytesCuartoFlotas*3 );

        // el resto son 0's por ahora
        memset( e->temporalSuma+offsetCuartoFlotas*3, 0, 2*bytesCuartoFlotas );

        // sumo el FFT1
#if ASMOPT
        sumarSenalesASM( e->temporalSuma, e->temporalFFT1, e->longitudfft );
#else
        sumarSenales( e->temporalSuma, e->temporalFFT1, e->longitudfft );
#endif

        // sumo el FFT2
#if ASMOPT
        sumarSenalesASM( e->temporalSuma+offsetCuartoFlotas, e->temporalFFT2, e->longitudfft );
#else
        sumarSenales( e->temporalSuma+offsetCuartoFlotas, e->temporalFFT2, e->longitudfft );
#endif

    } else {    /////////////////////////////////////// STEREO ////////////////////////////////////
         // multiplico por ventana

#if ASMOPT
        multiplicarVentanasStereoASM( e->temporalFFT1, e->ventana, e->longitudVentana );
        multiplicarVentanasStereoASM( e->temporalFFT2, e->ventana, e->longitudVentana );
#else
        multiplicarVentanasStereo( e->temporalFFT1, e->ventana, e->longitudVentana );
        multiplicarVentanasStereo( e->temporalFFT2, e->ventana, e->longitudVentana );
#endif

        // realizo fft1
        realFFT( &(e->transAudio), e->temporalFFT1, e->espectroEQ_L, e->espectroEQ_R );
        // ecualizo
#if ASMOPT
        multiplicarEspectrosASM( e->espectroEQ_L, e->respFrec, e->longitudfft+1 );
        multiplicarEspectrosASM( e->espectroEQ_R, e->respFrec, e->longitudfft+1 );
#else
        multiplicarEspectros( e->espectroEQ_L, e->respFrec, e->longitudfft+1 );
        multiplicarEspectros( e->espectroEQ_R, e->respFrec, e->longitudfft+1 );
#endif

        // retorno al dominio de las ondas
        realIFFT( &(e->transAudio), e->temporalFFT1, e->espectroEQ_L, e->espectroEQ_R );

        // realizo fft2
        realFFT( &(e->transAudio), e->temporalFFT2, e->espectroEQ_L, e->espectroEQ_R );
        // ecualizo
#if ASMOPT
        multiplicarEspectrosASM( e->espectroEQ_L, e->respFrec, e->longitudfft+1 );
        multiplicarEspectrosASM( e->espectroEQ_R, e->respFrec, e->longitudfft+1 );
#else
        multiplicarEspectros( e->espectroEQ_L, e->respFrec, e->longitudfft+1 );
        multiplicarEspectros( e->espectroEQ_R, e->respFrec, e->longitudfft+1 );
#endif

        // retorno al dominio de las ondas
        realIFFT( &(e->transAudio), e->temporalFFT2, e->espectroEQ_L, e->espectroEQ_R );

// construyo resultado y temporalEcualizado:

        // tomo lo que ecualize anteriormente
        memcpy( e->temporalSuma, e->temporalEcualizado, bytesCuartoFlotas*3 );

        // el resto son 0's por ahora
        memset( e->temporalSuma+offsetCuartoFlotas*3, 0, 2*bytesCuartoFlotas );

        // sumo el FFT1
#if ASMOPT
        sumarSenalesASM( e->temporalSuma, e->temporalFFT1, e->longitudfft*2 );
#else
        sumarSenales( e->temporalSuma, e->temporalFFT1, e->longitudfft*2 );
#endif

        // sumo el FFT2
#if ASMOPT
        sumarSenalesASM( e->temporalSuma+offsetCuartoFlotas, e->temporalFFT2, e->longitudfft*2 );
#else
        sumarSenales( e->temporalSuma+offsetCuartoFlotas, e->temporalFFT2, e->longitudfft*2 );
#endif

    }

    // guardo lo ecualizado necesario para la proxima ecualizacion
    memcpy( e->temporalEcualizado, e->temporalSuma+offsetCuartoFlotas*2, 3*bytesCuartoFlotas );

    // devuelvo el resultado de la ventana actual
#if ASMOPT
    getIntsInt16_ditherASM( (int16_t *) sampleada, e->temporalSuma, 2*cantidadMitadEntrada );
#else
    getIntsInt16_dither( (int16_t *) sampleada, e->temporalSuma, 2*cantidadMitadEntrada );
#endif
}

void ultimoCachito( eq *const e, void *const sampleada )
{
    memset( sampleada, 0, e->bytesMitadEntrada*2 );
    ecualizar( e, sampleada );
}

void ecualizarYEscribirCancion( eq *const e, const char *const archivoDeEntrada )
{
    // limpiamos lo que quede
    limpiarEQ( e );

    //arreglo temporal para leer y escribir
    int16_t *temporalIO = (int16_t *) malloc( sizeof(int16_t) * 2 * e->cantidadMitadEntrada );

    // archivos de lectura y escritura
    FILE *archivoEntrada = fopen( archivoDeEntrada, "r");
    PRINT_LOG("ecualizarYEscribirCancion: ecualizare este tema:", archivoDeEntrada, NULL );
    FILE *archivoSalida = fopen( "/mnt/sdcard/testArchivo2.wav", "w");


    // copiamos el header del wav (44 bytes)
    void *header = malloc( 44 );
    fread( header, 1, 44, archivoEntrada );
    fwrite( header, 1, 44, archivoSalida );
    free( header );

    int cantidadMitadEntrada = e->cantidadMitadEntrada;

    int cuantoLei;

    //leemos del archivo de entrada
    while ( (cuantoLei = fread( temporalIO, sizeof(int16_t), cantidadMitadEntrada*2, archivoEntrada )) == 2*cantidadMitadEntrada ) {

        // ecualizamos el pedazo
        ecualizar( e, temporalIO );
        // escribimos el pedazo en el archivo
        fwrite( temporalIO, sizeof(int16_t), 2 * cantidadMitadEntrada, archivoSalida );
    }

    // ecualizamos el ultimo cachito
    // recordar que cuantoLei no son bytes, son 16 bits. por eso multiplico por dos
    memset( temporalIO+cuantoLei, 0, (e->bytesMitadEntrada)*2 - cuantoLei*2 );
    ecualizar( e, temporalIO );
    fwrite( temporalIO, sizeof(int16_t), 2 * cantidadMitadEntrada, archivoSalida );

    ultimoCachito( e, temporalIO );
    // escribimos (solo lo que faltaba)
    fwrite( temporalIO, sizeof(int16_t), cuantoLei, archivoSalida );


    // liberamos el arreglo temporal para leer y escribir
    free( temporalIO );

    fclose( archivoEntrada );
    fclose( archivoSalida );
}

void destruirEQ( eq *const e )
{
    free( e->ventana );
    free( e->espectroEQ_L );
    free( e->espectroEQ_R );
    free( e->respFrec );
    free( e->temporalAudio );
    free( e->temporalEcualizado );
    free( e->temporalFFT1 );
    free( e->temporalFFT2 );
    free( e->temporalSuma );

    free( e->respuestaFrecAuxFiltro );
    free( e->ondaAuxFiltro );
    free( e->ondaFiltro );
    free( e->ventanaFiltro );

    destruirFFT( &(e->transAudio) );
    destruirFFT( &(e->transFiltro) );
    destruirFFT( &(e->transDiseno) );
    destruirFFT( &(e->transLPF) );
}
