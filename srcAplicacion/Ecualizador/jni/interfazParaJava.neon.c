#include <jni.h>
#include "realdft.h"
#include "ecualizador.h"
#include "analizador.h"


/////////// GLOBALES ///////////

eq ecualizador;
analizador analyzer;

/////////// FIN GLOB ///////////

void Java_com_ecualizador_LibC_inicializar( JNIEnv* env, jobject objeto, jint logitudventanainframesEQ, jint longitudventanainframesAN, jint SR, jint canales, jint cantBandasAnalizador )
{
    //    longitudframe = longitud del cacho de audio en frames
    //    SR = sample rate

    // inicializamos el monitor para el profiler
    monstartup("libinterfazParaJava.so");


    inicializarEQ( &ecualizador, logitudventanainframesEQ, SR, canales );
    inicializarAnalizador( &analyzer, longitudventanainframesAN, SR, canales, cantBandasAnalizador );
}

void Java_com_ecualizador_LibC_limpiarEQYAnalizador( JNIEnv* env, jobject objeto )
{
    limpiarEQ( &ecualizador );
    limpiarAN( &analyzer );
}

void Java_com_ecualizador_LibC_destruir( JNIEnv* env, jobject objeto )
{
    destruirEQ( &ecualizador );
    destruirAnalizador( &analyzer );

    // cerramos el monitor para el profiler
    moncleanup();
}

void Java_com_ecualizador_LibC_ecualizar( JNIEnv* env, jobject objeto, jbyteArray entrada )
{
    void *sampleada = (void *) ((*env)->GetByteArrayElements(env, entrada, NULL ));
    ecualizar( &ecualizador, sampleada );
    (*env)->ReleaseByteArrayElements( env, entrada, sampleada, 0 );
}

void Java_com_ecualizador_LibC_ultimo( JNIEnv* env, jobject objeto, jbyteArray entrada )
{
    void *sampleada = (void *) ((*env)->GetByteArrayElements(env, entrada, NULL ));
    ultimoCachito( &ecualizador, sampleada );
    (*env)->ReleaseByteArrayElements( env, entrada, sampleada, 0 );
}

void Java_com_ecualizador_LibC_actualizarFiltroEQ( JNIEnv* env, jobject objeto, jfloatArray parametros )
{
    void *cincoParametros = (void *) ((*env)->GetByteArrayElements(env, parametros, NULL ));

    actualizarFiltro( &ecualizador, cincoParametros );
    // DEVOLVER SIN COPIAR
    (*env)->ReleaseByteArrayElements( env, parametros, cincoParametros, JNI_ABORT );
}

void Java_com_ecualizador_LibC_ecualizarYEscribirCancion( JNIEnv* env, jobject objeto, jstring archivo )
{

    const char *nombreArchivo = (*env)->GetStringUTFChars(env,archivo,0);
    ecualizarYEscribirCancion( &ecualizador, nombreArchivo );

}

void Java_com_ecualizador_LibC_analizar( JNIEnv* env, jobject objeto, jbyteArray entrada )
{
    void *sampleada = (void *) ((*env)->GetByteArrayElements(env, entrada, NULL ));

    analizar( &analyzer, sampleada );

    // DEVOLVER SIN COPIAR
    (*env)->ReleaseByteArrayElements( env, entrada, sampleada, JNI_ABORT );
}

void Java_com_ecualizador_LibC_dameAnalisis( JNIEnv* env, jobject objeto, jfloatArray salida )
{
    // VER SI SE PUEDE OBTENER SIN COPIAR!!    
    jfloat *espectro = (jfloat *) ((*env)->GetFloatArrayElements(env, salida, NULL ));

    dameAnalisis( &analyzer, espectro );
    
    (*env)->ReleaseFloatArrayElements( env, salida, espectro, 0 );
}
