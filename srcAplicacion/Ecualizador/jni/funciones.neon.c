#include "funciones.h"

//////// Matriz para spline hardcodeada ////////////
float abcd[4][4][5] = 
               
        { {
                {0.267857, -0.607143, 0.428571, -0.107143, 0.0178571},
                {-0.339286, 1.03571, -1.14286, 0.535714, -0.0892857},
                {0.0892857, -0.535714, 1.14286, -1.03571, 0.339286},
                {-0.0178571, 0.107143, -0.428571, 0.607143, -0.267857}
          }, {
                {0, 0, 0, 0, 0},
                {0.803571, -1.82143, 1.28571, -0.321429, 0.0535714},
                {-0.214286, 1.28571, -2.14286, 1.28571, -0.214286},
                {0.0535714, -0.321429, 1.28571, -1.82143, 0.803571}
          }, {
                {-1.26786, 1.60714, -0.428571, 0.107143, -0.0178571},
                {-0.464286, -0.214286, 0.857143, -0.214286, 0.0357143},
                {0.125, -0.75, 0., 0.75, -0.125},
                {-0.0357143, 0.214286, -0.857143, 0.214286, 0.464286}
          }, {
                {1, 0, 0, 0, 0},
                {0, 1, 0, 0, 0},
                {0, 0, 1, 0, 0},
                {0, 0, 0, 1, 0}
             }
        };
//////////////// fin matriz spline /////////////////


// NO va en ASM
void inicializarDither( int longitud_rand )
{
    s = 0.5;

    // le damos un pequeno margen
    // para escalar a la ida
    int16_max_float = (float) INT16_MAX + 1.;

    // a este no, porque es para escalar a la vuelta
    w = (float) INT16_MAX;
    // inversa
    wi= 1.0/w;
    // amplitud del dither
    d = wi/RAND_MAX;
    // para remover dc offset
    o = wi*0.5;

    // arreglo de randoms para restarlos para el dither
    int16_t r1[longitud_rand];

    // espacio para guardar los random
    randoms = (float *) malloc( sizeof(float) * longitud_rand );

   // abro el archivo de randoms
    FILE *urand = fopen( "/dev/urandom", "r" );
    // nos copiamos todos los random que necesitaremos para
    // esta pasada
    fread( (void *) r1, sizeof(int16_t), longitud_rand, urand );
    fclose( urand );

    for( int i=1 ; i<longitud_rand ; i++ ) {
        randoms[i] = (float) (r1[i] - r1[i-1]);
    }
    randoms[0] = (float) (r1[0] - r1[longitud_rand-1]);
}

void destruidDither()
{
    free( randoms );
}

// falta en ASM
void multiplicarEspectros( COMPLEX *const espectro1,
                           const COMPLEX *const espectro2,
                           const int tamano )
{        // retorna en espectro1 !!!
    int i;
    float rTemp, iTemp;

    for ( i=0 ; i<tamano ; i++ ) {
        // guardo coeficientes del primer complejo
        rTemp = espectro1[i].real;
        iTemp = espectro1[i].imag;
        
        // realizo multiplicacion compleja
        espectro1[i].real = rTemp * espectro2[i].real - iTemp * espectro2[i].imag;
        espectro1[i].imag = iTemp * espectro2[i].real + rTemp * espectro2[i].imag;
    }
}

// ya esta en ASM
void sumarSenales( float *const senal1,
                   const float *const senal2,
                   const int tamano )
{        // retorna en senal 1 !!!
    int i;
    
    for ( i=0 ; i<tamano ; ++i ) {
        senal1[i] += senal2[i];
    }
}

// falta en ASM
void getFlotasInt16( const int16_t *const ints,
                      float *const flotas,
                      const int tamano )
{
    int i;
    for ( i=0 ; i<tamano ; ++i ) {
        // escalamos para que el audio quede entre -1 y +1
        flotas[i] = (float) ints[i] / int16_max_float;

//        if ( flotas[i] > 1. || flotas[i] < -1. ) {
//            PRINT_LOG("yendoAInt", "overflow : %f", flotas[i] );
//        }
    }
}

// no va en ASM
// de hecho ni se usa, porque se hace con dither
void getIntsInt16( int16_t *const ints,
                   const float *const flotas,
                   const int tamano )
{
    int i; for ( i=0 ; i<tamano ; ++i ) {
        // reescalamos
        ints[i] = flotas[i]*int16_max_float;
    }
}

// falta en ASM
void getIntsInt16_dither( int16_t *const ints,
                          const float *const flotas,
                          const int tamano )
{

    int i;
    for ( i=0 ; i<tamano ; ++i ) {
        in = flotas[i];

        in += s * (s1 + s1 - s2);            //error feedback
        tmp = in + o + d * randoms[i];    //dc offset and dither

        out = (int16_t)(w * tmp);            //truncate downwards
        if(tmp<0.0) out--;                  //this is faster than floor()

        s2 = s1;
        s1 = in - wi * (float)out;           //error

        //devolvemos resultado
        ints[i] = out;
   }
}

// ya esta en ASM
void multiplicarVentanasMono( float *const v1,
                              const float *const v2,
                              const int tamano )
{
    int i;
    for ( i=0 ; i<tamano ; i++ ) {
        v1[i] *= v2[i];
    }
}

void multiplicarVentanasStereo( float *const v1,
                                const float *const v2,
                                const int tamano )
{
    int i;
    for (i=0 ; i<tamano ; i++ ) {
        v1[i*2] = v1[i*2] * v2[i];
        v1[i*2+1] = v1[i*2+1] * v2[i];
    }
}

// esta VA
void memCopia( void *a, const void *de, size_t cuanto )
{
    memcpy( a, de, cuanto );
}

// esta VA
void memSeteado( void *a, int que, size_t cuanto )
{
    memset( a, que, cuanto );
}

// no va en ASM
void crearBartlett( float *const ventana,
                    const int tamano )
{
    int i;
    for ( i=0 ; i<tamano/2 ; ++i ) {
        // recta y=x hasta la mitad
        ventana[i] = (float)i*2.0/(float)tamano;
        // recta y=1-x, segunda mitad
        ventana[i+tamano/2] = 1.0 - (float)i*2.0/(float)tamano;
    }

}

// no va en ASM
void crearHamming( float *const ventana,
                   const int tamano,
                   const float alpha,
                   const float beta )
{
//    alpha = 0.54
//    beta = 0.46
    int i;
    for ( i=0 ; i<tamano ; ++i ) {
        ventana[i] = alpha - beta * cos( 2.0*PI*(float)i / (float)(tamano-1) );
    }
}

// no va en ASM
void crearBlackman( float *const ventana,
                    const int tamano,
                    const float a0,
                    const float a1,
                    const float a2 )
{
//    a0 = 0.42659
//    a1 = 0.49656
//    a2 = 0.076849    
    int i;
    for ( i=0 ; i<tamano ; i++ ) {
        ventana[i] = a0 - a1*cos( 2*PI*(float)i/(float)(tamano-1)) + a2*cos(4*PI*(float)i/(float)(tamano-1));
    }
}

// no va en ASM
void crearSincFunction( float *const ventana,
                        const float corte,
                        const int tamano )
{        //tamano debe ser impar!
    int i;
    float x;
    int medio = (tamano-1)/2;

    for ( i=0 ; i<tamano; i++ ) {

        if ( i == medio ) {
            // caso particular, por l'hopital, tenemos:
            ventana[i] = 2.0*corte;
        } else {
            x =  i - 0.5 * (tamano-1);
            ventana[i] = sin( 2.0*PI*corte*x ) / (x*PI);
        }
    }
}

// no va en ASM
void crearLPF( const float frecCorte,
               COMPLEX *const respuestaFrec,
               const int tamanoFiltro,
               const int cantidadSamples,
               const float sampleRate,
               fft *const transformador )
{
    float corte = frecCorte / sampleRate;

    float ventanitaFiltro[cantidadSamples];
    float ventanaParaSinc[tamanoFiltro];

    crearBlackman( ventanaParaSinc, tamanoFiltro, CLASSIC_BLACKMAN );
//    crearHamming( ventanaParaSinc, tamanoFiltro, CLASSIC_HAMMING );
//    crearBartlett( ventanaParaSinc, tamanoFiltro );

    crearSincFunction( ventanitaFiltro, corte , tamanoFiltro );
    
    int i;
    // ventaneamos la sinc function
    for ( i=0 ; i<tamanoFiltro ; i++ ) {
        ventanitaFiltro[i] *= ventanaParaSinc[i];
    }

    // padeamos
    memset( ventanitaFiltro+tamanoFiltro, 0, sizeof(float)*(cantidadSamples-tamanoFiltro) );

    // tomamos el espectro
    realFFT( transformador, ventanitaFiltro, respuestaFrec, NULL ); 
}

// no va en ASM
void crearHPF( const float frecCorte,
               COMPLEX *const respuestaFrec,
               const int tamanoFiltro,
               const int cantidadSamples,
               const float sampleRate,
               fft *const transformador )
{        //tamanoFiltro debe ser impar!
    float corte = frecCorte / sampleRate;

    float ventanitaFiltro[cantidadSamples];
    int medio = (tamanoFiltro-1)/2;

    float ventanaParaSinc[tamanoFiltro];
    crearBlackman( ventanaParaSinc, tamanoFiltro, CLASSIC_BLACKMAN );
    crearSincFunction( ventanitaFiltro, corte, tamanoFiltro );

    // ventaneamo la funcion ( d - sincFunction ), para obtener un highpass
    // ( d es un allpass, la sincFunction un low pass )
    int i;
    for ( i=0 ; i<tamanoFiltro ; i++ ) {
        if ( i==medio ) {
            ventanitaFiltro[i] = ventanaParaSinc[i] * (1.0 - ventanitaFiltro[i]);
        } else {
            ventanitaFiltro[i] = (-ventanitaFiltro[i]) * ventanaParaSinc[i];
        }
    }

    // padeamos
    memset( ventanitaFiltro+tamanoFiltro, 0, sizeof(float)*(cantidadSamples-tamanoFiltro) );

    // tomamos el espectro
    realFFT( transformador, ventanitaFiltro, respuestaFrec, NULL ); 
}

// no va en ASM
void crearBPF( const float frecCentral,
               const float Q,
               COMPLEX *const respuestaFrec,
               const int tamanoFiltro,
               const int cantidadSamples,
               const float sampleRate,
               fft *const transformador )
{        //tamanoFiltro debe ser impar!

    COMPLEX hpf[cantidadSamples];

    float factor = sqrt(1.0/Q);

    crearLPF( frecCentral*factor, respuestaFrec, tamanoFiltro, cantidadSamples, sampleRate, transformador );
    crearHPF( frecCentral/factor, hpf, tamanoFiltro, cantidadSamples, sampleRate, transformador );

    // band pass = low pass + hi pass
    multiplicarEspectros( respuestaFrec, hpf, cantidadSamples );

}
 
// no va en ASM
static float polinomio( const float x,
                         const int j,
                         const float *const parametros )
{
    int i, k;
    
    float coeficiente;
    float res = 0;

    for ( i=0 ; i<4 ; ++i  ) {
        
        coeficiente = 0;

        for ( k=0 ; k<5 ; ++k ) {
            // cada y por cada coeficiente
            coeficiente += parametros[k]*abcd[i][j][k];
        }
        // cada coeficiente por x^(cada exponente)    
        res += coeficiente * pow( x-j, 3-i );
    }

    return res;
}

// no va en ASM
void escalera( const float *const parametros,
               COMPLEX *const respuestaFrec,
               const int samplerate,
               int cantidadSamples )
{
    float frecuencia;
    int j;
    int i = 0;

    // para cada intervalo el espectro es constante

    for ( j=0 ; j<4 ; j++ ) {

        frecuencia = (float)i*samplerate/(float)cantidadSamples;

        while ( frecuencia < 32*pow(4,j+1) ) {            // 32!!!
    
            respuestaFrec[i].real = IDB( parametros[j] );
            respuestaFrec[i].imag = 0;

            i++;

            frecuencia = (float)i*samplerate/(float)cantidadSamples;
        }
    }

    while ( frecuencia < samplerate ) {

        respuestaFrec[i].real = IDB( parametros[4] );
        respuestaFrec[i].imag = 0;

        i++;

        frecuencia = (float)i*samplerate/(float)cantidadSamples;
    }
}

// no va en ASM
void splineCubico( const float *const parametros,
                   COMPLEX *const respuestaFrec,
                   const int samplerate,
                   const int cantidadSamples )
{
    // las coordenadas X de los puntos a interpolar son:
    // 64
    // 256
    // 1024
    // 4096
    // 16384
    
    float frecuencia;
    int j;
    int indexEspectro = 0;

    // para cada intervalo el espectro vale lo que vale el polinomio 


    // para el primer intervalo escalamos proporcionalmente para evitar que se amplifiquen mucho los bajos
    // en caso de que el spline interpole una curva con mucha pendiente
    float primerCorte = 64.;
    float atenuacionPrimerIntervalo = 100.;
    for ( j=0 ; j<4 ; j++ ) {

        frecuencia = (float)indexEspectro*samplerate/(float)cantidadSamples;

        while ( frecuencia < 64*pow(4,j+1) && indexEspectro<cantidadSamples ) {
    
            if ( j==0 ) {
                float supuestaAmplitud = IDB( polinomio( GETX(frecuencia), j, parametros ) );
                if ( supuestaAmplitud-1.01 > 0. ) {
                    supuestaAmplitud = 1.+(supuestaAmplitud-1.)/(atenuacionPrimerIntervalo * (1.-(frecuencia/primerCorte)) );
                    respuestaFrec[indexEspectro].real = supuestaAmplitud;
//                    PRINT_LOG("spline cuabico", "frec: %f", frecuencia);
//                    PRINT_LOG("spline cuabico", "esto amplifica: %f", supuestaAmplitud );
                }
                respuestaFrec[indexEspectro].real = supuestaAmplitud;
            } else {
                respuestaFrec[indexEspectro].real = IDB( polinomio( GETX(frecuencia), j, parametros ) );
            }
            respuestaFrec[indexEspectro].imag = 0;

            indexEspectro++;

            frecuencia = (float)indexEspectro*samplerate/(float)cantidadSamples;
        }
    }

    float ultimo = respuestaFrec[indexEspectro-1].real;
    // cuanto falta para tener que tener ganancia unitaria
    int cuantoFalta = (cantidadSamples/2 - indexEspectro)*2;
    int porDondeVoy = 0;
    float x,y;

    // aqui volvemos a ganancia unitaria a partir de 22050 Hz + cierta cantidad para
    // mantener modificados los agudos hata 22050Hz
    // vamos a usar la logistic fucniton entre -6 y 6
    
    while ( cuantoFalta-porDondeVoy > 0 ) {
        // x entre -6 y 6
        x = 12*((float)porDondeVoy/cuantoFalta)-6;

        y = exp(-x);
        respuestaFrec[indexEspectro].real = (ultimo-1.)*(y/(1.+y))+1.;
        respuestaFrec[indexEspectro].imag = 0;

        porDondeVoy++;
        indexEspectro++;

        frecuencia = (float)indexEspectro*samplerate/(float)cantidadSamples;
    }

    while ( frecuencia < samplerate && indexEspectro<cantidadSamples ) {

        // ganancia unitaria
        respuestaFrec[indexEspectro].real = 1;
        respuestaFrec[indexEspectro].imag = 0;

        indexEspectro++;

        frecuencia = (float)indexEspectro*samplerate/(float)cantidadSamples;
    }
}
