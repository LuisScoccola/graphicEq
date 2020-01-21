package com.ecualizador;

public class LibC {

    public LibC( int longitudEQ, int longitudAn, int sampleRate, int canales, int cantBandasAnalizador )
    {
        inicializar( longitudEQ, longitudAn, sampleRate, canales, cantBandasAnalizador );
    }

    public void close()
    {
        destruir();
    }

    private native void inicializar( int longitudEQ, int longitudAn, int sampleRate, int canales, int cantBandasAnalizador );
    private native void destruir();

    public native void limpiarEQYAnalizador();
    public native void ecualizar( byte[] bufferSong );
    public native void ultimo( byte[] bufferSong );

    public native void actualizarFiltroEQ( float[] parametros );

    public native void ecualizarYEscribirCancion( String nombreArchivo );

    public native void analizar( byte[] bufferSong );
    public native void dameAnalisis( float[] bandas );

    static
    {
        System.loadLibrary("interfazParaJava"); 
    }


}
