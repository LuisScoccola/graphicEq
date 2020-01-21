/*    TODO:
 *
 *
 */

#include "realdft.h"

#define LOGPAR(N) (N&0x55555555)

////////////////////// FUNCION FFT //////////////////////////

static void fftFunction(
            const int N,
            COMPLEX *const x,
            COMPLEX *X,
            COMPLEX *temp,
            const float *const tablaSenos,
            const float *const tablaCosenos )
{

    COMPLEX *pares, *impares;
    COMPLEX *resAct1, *resAct2, *resActual;
    int iteracion;              // contador de ineracion
    int salto;                  // distancia entre par-impar
    float twiddleR, twiddleI;  // factor para mult los impares
    float iItemp, iRtemp;      // temporales para mult los impares
    int k, l, n;                /* n = longitud del dft actual, k itera
                                   sobre los sinusoides de la base, l sobre
                                   los resultados del dft anterior    */

    if ( N==1 ) {               // caso dft de tamano 1
        X[0] = x[0];
        return;
    }

    /*    Como vamos intercalando entre X (arreglo de salida) y
        temp (arreglo temporal) para escribir los resultados
        vemos con cual empezar para terminar con los resultados
        finales en X.
        Se chequea si N es potencia par o impar de 2.    */

    if ( LOGPAR(N) == 0 ) {
        iteracion = 1;
    } else {
        iteracion = 0;
    }
    
    pares = x;                  // comenzamos leyendo de la entrada

    for ( n=1 ; n<N ; n<<=1 ) { // tamano del dft ( arbol de altura log(N) )

        if ( iteracion%2 == 0 ) {   // intercalamos entre X y temp en cada it.
            resActual = temp;
        } else {
            resActual = X;
        }

        salto = N/(2*n);         /*    salteamos el fft de tamano 1,
                                       dado que "viene hecho"     */
        resAct1 = resActual;          // X_k
        resAct2 = resActual + N/2;    // X_(k+N/2)
    
        for ( k=0 ; k<n ; ++k ) {     // para cada sinusoide de la base

            twiddleI = tablaSenos[k*salto];
            twiddleR = tablaCosenos[k*salto];

#if ASMOPT
            bucleMedioFftASM(&pares,salto,&resAct1,&resAct2,&twiddleI,&twiddleR);
#else
            for ( l=0 ; l<salto ; ++l ) {    // para cada par par-impar
                impares = pares + salto;
                
                // impares * e^(-i*2*pi*k/N)
                iRtemp = impares->real * twiddleR - impares->imag * twiddleI;
                iItemp = impares->real * twiddleI + impares->imag * twiddleR;

                // X_k = pares + impares
                resAct1->real = pares->real + iRtemp;
                resAct1->imag = pares->imag + iItemp;
                
                // X_(k+N/2) = pares - impares
                resAct2->real = pares->real - iRtemp;
                resAct2->imag = pares->imag - iItemp;
                
                resAct1++;
                resAct2++;
                pares++;
            }
#endif

            pares += salto;
        }

        pares = resActual;        // ahora leemos de lo que acabamos de escribir
        iteracion++;
    }
}

//////////////////////// FIN FUNCION FFT ////////////////////////









////////////// FUNCION REALDFT y REALIDFT (mono stereo y int16) /////////////////

static void splitMono( COMPLEX *X,
                       COMPLEX *A,
                       COMPLEX *B,
                       COMPLEX *espectro,
                       int longitud )
{
    int k;
    for (k=0; k<longitud ; k++) {
        espectro[k].real = X[k].real * A[k].real - X[k].imag * A[k].imag + X[longitud-k].real * B[k].real + X[longitud-k].imag * B[k].imag;
        espectro[k].imag = X[k].imag * A[k].real + X[k].real * A[k].imag + X[longitud-k].real * B[k].imag - X[longitud-k].imag * B[k].real;
    }
}

static void splitStereo( COMPLEX *X,
                         COMPLEX *espectroL,
                         COMPLEX *espectroR,
                         int longitud )
{
    int k, N;
    N = longitud/2;

#if ASMOPT
    bucleMedioSplitStereoASM(espectroL, espectroR, X, longitud);
#else
     for ( k=1 ; k<N ; ++k ) {
        espectroL[k].real = (X[k].real + X[longitud-k].real)/2;
        espectroL[k].imag = (X[k].imag - X[longitud-k].imag)/2;
        espectroR[k].real = (X[k].imag + X[longitud-k].imag)/2;
        espectroR[k].imag = (X[longitud-k].real - X[k].real)/2;
        espectroL[longitud-k].real = espectroL[k].real;
        espectroL[longitud-k].imag = -espectroL[k].imag;
        espectroR[longitud-k].real = espectroR[k].real;
        espectroR[longitud-k].imag = -espectroR[k].imag;
    }
   
#endif

    espectroL[0].real = X[0].real;
    espectroL[0].imag = 0;
    espectroR[0].real = X[0].imag;
    espectroR[0].imag = 0;
    espectroL[N].real = X[N].real;
    espectroL[N].imag = 0;
    espectroR[N].real = X[N].imag;
    espectroR[N].imag = 0;

}

static void realFFTyaOrdenadosMono( int longitud,
                                    COMPLEX x[],
                                    COMPLEX espectro[],
                                    COMPLEX temporalComplex[],
                                    COMPLEX arregloTemp[],
                                    COMPLEX A[],
                                    COMPLEX B[],
                                    float tablaSenos[],
                                    float tablaCosenos[] )
{
    int k;

    fftFunction( longitud, x, temporalComplex, arregloTemp, tablaSenos, tablaCosenos );

    // ultimo igual al primero    
    temporalComplex[longitud].real = temporalComplex[0].real;
    temporalComplex[longitud].imag = temporalComplex[0].imag;
    
#if ASMOPT
    splitMonoASM( temporalComplex, A, B, espectro, longitud );
#else
    splitMono( temporalComplex, A, B, espectro, longitud );
#endif

    //copiamos espectro de las positivas a las negativas
#if ASMOPT
    bucleEspectroMonoASM(espectro, longitud);
#else
    for (k=1; k<longitud; ++k) {
        espectro[2*longitud-k].real = espectro[k].real;
        espectro[2*longitud-k].imag = -espectro[k].imag;
    }
#endif

    espectro[longitud].real = temporalComplex[0].real - temporalComplex[0].imag;
    espectro[longitud].imag = 0;


}

static void realIFFTyaOrdenadosMono( int longitud,
                                     COMPLEX x[],
                                     COMPLEX espectro[],
                                     COMPLEX temporalComplex[],
                                     COMPLEX arregloTemp[],
                                     COMPLEX IA[],
                                     COMPLEX IB[],
                                     float tablaSenos[],
                                     float tablaCosenos[] )
{
    int n, k;

#if ASMOPT
    splitMonoASM( espectro, IA, IB, temporalComplex, longitud );
   
    rIFFTyaomASM_1(temporalComplex,longitud);

#else
    splitMono( espectro, IA, IB, temporalComplex, longitud );

    //conjugado de X[k] para calcular la inversa    
    for ( k=0 ; k<longitud ; ++k ) {
        temporalComplex[k].imag = -temporalComplex[k].imag;
    }

#endif



    fftFunction(longitud, temporalComplex, x, arregloTemp, tablaSenos, tablaCosenos );

#if ASMOPT
    rIFFTyaomASM_2(x, longitud);
#else
    //dividimos por N y tomamos conjugado para terminar de calcular la inversa
    for ( n=0 ; n<longitud ; ++n ) {
        x[n].real = x[n].real/(float)longitud;
        x[n].imag = (-x[n].imag)/(float)longitud;
    }
#endif
}

static void realFFTyaOrdenadosStereo( int longitud,
                                      COMPLEX x[],
                                      COMPLEX espectroL[],
                                      COMPLEX espectroR[],
                                      COMPLEX temporalComplex[],
                                      COMPLEX arregloTemp[],
                                      float tablaSenos[],
                                      float tablaCosenos[] )
{
    fftFunction( longitud, x, temporalComplex, arregloTemp, tablaSenos, tablaCosenos );

    // ultimo igual al primero    
    temporalComplex[longitud].real = temporalComplex[0].real;
    temporalComplex[longitud].imag = temporalComplex[0].imag;
    
    splitStereo( temporalComplex, espectroL, espectroR, longitud );
}

static void realIFFTyaOrdenadosStereo( int longitud,
                                       COMPLEX x[],
                                       COMPLEX espectroL[],
                                       COMPLEX espectroR[],
                                       COMPLEX temporalComplex[],
                                       COMPLEX arregloTemp[],
                                       float tablaSenos[],
                                       float tablaCosenos[] )
{
    int n, k;

#if ASMOPT
    rIFFTyaosASM_1(x,espectroL,espectroR,longitud);
#else
    for ( k=0 ; k<longitud ; ++k ) {
        x[k].real = espectroL[k].real - espectroR[k].imag;
        x[k].imag = espectroL[k].imag + espectroR[k].real;
    }

    //conjugado 
    for (k=0; k<longitud; ++k) {
        x[k].imag = -x[k].imag;
    }
#endif

    fftFunction(longitud, x, temporalComplex, arregloTemp, tablaSenos, tablaCosenos );

    // dividimos por N y tomamos conjugado para terminar de calcular la inversa
    // y lo ponemos en el arreglo original
#if ASMOPT
    rIFFTyaosASM_2(x,temporalComplex,longitud);
#else
    for (n=0; n<longitud ; ++n) {
        x[n].real = temporalComplex[n].real/(float)longitud;
        x[n].imag = (-temporalComplex[n].imag)/(float)longitud;
    }
#endif
}

///////////////////////// FIN REALDFT y REALIDFT //////////////////////////




///////////////////////// FFT EXPORTADA ///////////////////////////////////


void realFFT( fft *const trans,
              float *input,
              COMPLEX *output1,
              COMPLEX *output2 )
{
    int longitud = trans->longitud;
    int canales = trans->canales;

    COMPLEX *temporalComplex = trans->temporalComplex;
    COMPLEX *arregloTemp = trans->arregloTemp;
    float *tablaSenos = trans->tablaSenos;
    float *tablaCosenos = trans->tablaCosenos;

    COMPLEX *A = trans->A;
    COMPLEX *B = trans->B;

    if ( canales == MONO ) {
        realFFTyaOrdenadosMono( longitud, (COMPLEX *) input, output1, temporalComplex, arregloTemp, A, B, tablaSenos, tablaCosenos );

    } else {
        realFFTyaOrdenadosStereo( longitud, (COMPLEX *) input, (COMPLEX *) output1, (COMPLEX *) output2, temporalComplex, arregloTemp, tablaSenos, tablaCosenos );
    }
}

void realIFFT( fft *const trans,
               float *output,
               COMPLEX *input1,
               COMPLEX *input2 )
{
    int longitud = trans->longitud;
    int canales = trans->canales;

    COMPLEX *temporalComplex = trans->temporalComplex;
    COMPLEX *arregloTemp = trans->arregloTemp;
    float *tablaSenos = trans->tablaSenos;
    float *tablaCosenos = trans->tablaCosenos;

    COMPLEX *IA = trans->IA;
    COMPLEX *IB = trans->IB;

    if ( canales == MONO ) {

        realIFFTyaOrdenadosMono( longitud, (COMPLEX *) output, input1, temporalComplex, arregloTemp, IA, IB, tablaSenos, tablaCosenos );

    } else {

        realIFFTyaOrdenadosStereo( longitud, (COMPLEX *) output, (COMPLEX *) input1, (COMPLEX *) input2, temporalComplex, arregloTemp, tablaSenos, tablaCosenos );

    }
}

void complexFFT( fft *const trans,
                 COMPLEX *input,
                 COMPLEX *output )
{
    fftFunction( trans->longitud, input, output, trans->arregloTemp, trans->tablaSenos, trans->tablaCosenos );
}

void complexIFFT( fft *const trans,
                  COMPLEX *output,
                  COMPLEX *input )
{
    int longitud = trans->longitud;
    int i;

    for ( i=0 ; i<longitud ; ++i ) {
        input[i].imag = - input[i].imag; 
    }

    fftFunction( trans->longitud, input, output, trans->arregloTemp, trans->tablaSenos, trans->tablaCosenos );

    for ( i=0 ; i<longitud ; ++i ) {
        input[i].imag = - input[i].imag;    // arreglo lo que aletere
        output[i].real = output[i].real / (float)longitud;
        output[i].imag = ( -output[i].imag ) / (float)longitud;
    }
}

//////////////////////// FIN FFT EXPORTADA ////////////////////////////////






/////////////////// FUNCION INICIALIZACION ////////////////////////////////

void inicializarFFT( fft *const trans,
                     const int longitudArray,
                     const int canales )
{

    // longitud = tamano del fft (complex)
    // longitudArray = tamano del arreglo (real) o arreglos (si estero)

    trans->canales = canales;
    trans->longitudArreglo = longitudArray;

    int longitud; 
    if ( canales == COMPLEJO ) {            // caso particular: lo voy a usar para hacer complexFFT
        longitud = longitudArray;
    } else if ( canales == MONO ) {
        longitud = longitudArray/2;
    } else {
        longitud = longitudArray;
    }

    trans->longitud = longitud;

    trans->temporalComplex = malloc( sizeof(COMPLEX) * (trans->longitudArreglo + 1) );    

    trans->arregloTemp = malloc( sizeof(COMPLEX) * (trans->longitud + 1) );        // de este mas uno no estoy seguro


    // inicializacion de twiddle factors

    trans->tablaSenos = malloc( sizeof(float) * (trans->longitud) );
    trans->tablaCosenos = malloc( sizeof(float) * (trans->longitud) );

    int k;
    float arg;

    for ( k=0 ; k<longitud ; k++ ) {
        arg = (-2.0*PI*(float)k) / (float)trans->longitud;
        trans->tablaSenos[k] = sin(arg);
        trans->tablaCosenos[k] = cos(arg); 
    }


    if ( canales == MONO ) {    // inicializacion de matrices para split (solo MONO)
        trans->A = malloc( sizeof(COMPLEX) * trans->longitud );
        trans->B = malloc( sizeof(COMPLEX) * trans->longitud );
        trans->IA = malloc( sizeof(COMPLEX) * trans->longitud );
        trans->IB = malloc( sizeof(COMPLEX) * trans->longitud );
        
        for ( k=0 ; k<longitud ; k++ ) {
            trans->A[k].imag = -cos(PI/(float)(trans->longitud)*(float)k) / 2.0;
            trans->A[k].real = (1.0 - sin(PI/(float)(trans->longitud)*(float)k)) / 2.0;
            trans->B[k].imag = ((cos(PI/(float)(trans->longitud)*(float)k)) / 2.0);
            trans->B[k].real = (1.0 + sin(PI/(float)(trans->longitud)*(float)k)) / 2.0;
            trans->IA[k].imag = -trans->A[k].imag;
            trans->IA[k].real = trans->A[k].real;
            trans->IB[k].imag = -trans->B[k].imag;
            trans->IB[k].real = trans->B[k].real;
        }
    }
}

void destruirFFT( fft *const trans )
{
    free(trans->temporalComplex);
    free(trans->arregloTemp);
    free(trans->tablaSenos);
    free(trans->tablaCosenos);

    if ( trans->canales == MONO ) {
        free(trans->A);
        free(trans->B);
        free(trans->IA);
        free(trans->IB);
    }
}

///////////////// FIN  INICIALIZACION ////////////////
