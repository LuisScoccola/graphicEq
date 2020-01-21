package com.ecualizador;

import java.io.DataInputStream;
import java.io.FileInputStream;

import android.util.Log;

public class Wave {
    String      _nomArchivo;
    riffChunk   _riffChunk;
    fmtChunkH   _fmtChunkH;
    fmtChunkD   _fmtChunkD;
    dataChunkH  _dataChunkH;
    int         _dataOffset;
    double      _duration; //En seg.
    boolean     _valido;

    int RIFF  = 0x52494646;
    int WAVE  = 0x57415645;
    int FMT   = 0x666d7420;
    int DATA  = 0x64617461;

    public Wave(String nomArchivo) throws Exception {
        _nomArchivo = nomArchivo;
        _valido = true;

        Log.d("WAVE","Voy a ver si el archivo es PCM wav");

        DataInputStream buffBytesWAV = new DataInputStream(new FileInputStream(nomArchivo));

        // -- -- Leo el primer Chunk
        riffChunk.ctRiff = buffBytesWAV.readInt();
        riffChunk.csRiff = cambiarEndianness( buffBytesWAV.readInt() );
        riffChunk.format = buffBytesWAV.readInt();

        if( riffChunk.ctRiff != RIFF || riffChunk.format != WAVE ){
            Log.d("WAVE","no es valido1");
            _valido = false;
            return;
        }

        // -- -- Leo el segundo Chunk
        fmtChunkH.ctFMT = buffBytesWAV.readInt();
        fmtChunkH.csFMT = cambiarEndianness( buffBytesWAV.readInt() );

        if( fmtChunkH.ctFMT != FMT ){
            Log.d("WAVE","no es valido2");
            _valido = false;
            return;
        }

        // -- -- Leo el tercero


        fmtChunkD.formatTag         = cambiarEndianness( buffBytesWAV.readShort() ); 
        fmtChunkD.channels          = cambiarEndianness( buffBytesWAV.readShort() );
        fmtChunkD.samplesPerSec     = cambiarEndianness( buffBytesWAV.readInt() );
        fmtChunkD.avgBytesPerSec    = cambiarEndianness( buffBytesWAV.readInt() );
        fmtChunkD.blockAlign        = cambiarEndianness( buffBytesWAV.readShort() );
        fmtChunkD.bitsPerSample     = cambiarEndianness( buffBytesWAV.readShort() );

        if( fmtChunkD.bitsPerSample != 16 ) {
            Log.d("WAVE","no es valido3");
            _valido = false;
            return;
        }

        // -- -- Leo el cuarto
        dataChunkH.ctData = buffBytesWAV.readInt();
        dataChunkH.csData = cambiarEndianness( buffBytesWAV.readInt() );

        if( dataChunkH.ctData != DATA ){
            Log.d("WAVE","no es valido4");
            _valido = false;
            return;
        }

        // no hay que dividir por la cantidad de canales
        _duration = Math.floor( dataChunkH.csData / (double)( /*fmtChunkD.channels * */ fmtChunkD.avgBytesPerSec ) );  
        _dataOffset = 44;

        buffBytesWAV.close();

        Log.d("WAVE","El archivo es PCM: " + _valido);
    }

    int cambiarEndianness(int i) {
        return((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff);
    }

    short cambiarEndianness(short i) {
        return (short) (((i&0xff)<<8) + ((i&0xff00)>>8));
    }

}

class riffChunk{
    static int ctRiff;              //Constante "RIFF" - char[4]
    static int csRiff;              //Chunk SIZE - Archivo entero menos 8b
    static int format;              //Constante "WAVE" - char[4] - Si no, no es WAVE
}

class fmtChunkH{
    static int ctFMT;               //Constante "FMT " - char[4]
    static int csFMT;               //Chunk SIZE - PCM = 16
}

class fmtChunkD{
    static short    formatTag;      //Formato Audio - PCM = 1
    static short    channels;       //Cant. Canales
    static int      samplesPerSec;  //Sample Rate : 8000, 44100
    static int      avgBytesPerSec; //Data para reproductores
    static short    blockAlign;     //Numero de bytes de un sample
    static short    bitsPerSample;  //Bits por cada sample
}

class dataChunkH{
    static int    ctData;           //Constante "DATA" - char[4]
    static int    csData;           //Chunk SIZE - tam bytes DATA
}
