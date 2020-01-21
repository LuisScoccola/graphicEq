package com.ecualizador;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Array;
import java.util.Arrays;
import java.util.Random;
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Handler;
import android.os.Looper;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/*    TODO
 * 
 */


public class Programa extends SurfaceView  implements SurfaceHolder.Callback
{
    Wave miWAV;

    int canales;
    int sampleRateInHz;
    int audioFormat;
    int channelConfig;

    byte[][] buffersAudio;
    float[][] buffersEspectro;

    Semaphore[] mutexLecturaAudioReproductor;
    Semaphore[] mutexLecturaAudioAnalizador;
    Semaphore[] mutexEscrituraAudioReproductor;
    Semaphore[] mutexEscrituraAudioAnalizador;
    Semaphore[] mutexLecturaEspectro;
    Semaphore[] mutexEscrituraEspectro;
    Semaphore   semaphoreHayQueGraficar;

    ReentrantReadWriteLock mutexBandasEQ;
    ReentrantReadWriteLock mutexCambiarEQ;
    ReentrantReadWriteLock mutexThresholdAN;
    ReentrantReadWriteLock mutexUsarEQ;
    boolean usarEQ = true;

    // cuidado que de este numero depende el define AMPLITUDREAL que se encuentra en tipos.h !
    int ventanaEQsize = (int) Math.pow( 2, 12 );            // cantidad de frames que quiero tomar de la cancion cuando ecualizo
    int ventanaAnalizadorSize = ventanaEQsize;              // cantidad de franes que quiero tomar de la cancion cuando analizo espectro

    int bufferSizeInFramesEQ;                                // cantidad de frames en el buffer (si es estereo sera el doble que si es mono)
    int bufferSizeInBytesEQ;                                // tamano de buffer de lectura y escritura en bytes
    int bufferSizeInFramesAn;                                // cantidad de frames en el buffer (si es estereo sera el doble que si es mono)
    int bufferSizeInBytesAn;                                // tamano de buffer de lectura y escritura en bytes
    int cantidadBuffersAudio = 8;
    int cantidadBuffersEspectro = 8;
    // debe ser multiplo de 5
    int cantidadBandasAnalizador = 30;
    // dado el ramano de ventana 2^12, 10 es un segundo:
    int refreshRateAnalizadorSeleccionada = 1;
    int refreshRateAnalizadorActual; 
    // dB de threshold del analizador
    int thresholdAnalizador = 15;
    int ceilBandas = 6;

    LibC lib = null;

    AudioTrack track = null;

    Thread procesadorAudioT = null;
    Thread reproductorT = null;
    Thread analizadorT = null;
    Thread graficadorT = null;
    SurfaceHolder holderGraficador;

    // bandas del ecualizador
    float[] estadoBandasEq = {(float)0.,(float)0.,(float)0.,(float)0.,(float)0.};
    float[] noEcualizar = {(float)0.,(float)0.,(float)0.,(float)0.,(float)0.};


    // constructor
    public Programa( Wave _miWAV, Context context )
    {
        //Llamo al constructor de SurfaceView
        super(context);

        // tomo el wav que me llego
        miWAV = _miWAV;

        // inicializacion data archivo
        canales = fmtChunkD.channels;
        channelConfig = ( fmtChunkD.channels == 1)? AudioFormat.CHANNEL_OUT_MONO : AudioFormat.CHANNEL_OUT_STEREO;
        audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        sampleRateInHz = fmtChunkD.samplesPerSec;
        // fin data archivo

        // inicializacion de buffersAudio, buffersEspectro y semaforos
        bufferSizeInFramesEQ = (channelConfig == AudioFormat.CHANNEL_OUT_MONO)? ventanaEQsize : ventanaEQsize*2;
        bufferSizeInBytesEQ = bufferSizeInFramesEQ*2;

        bufferSizeInFramesAn = (channelConfig == AudioFormat.CHANNEL_OUT_MONO)? ventanaAnalizadorSize : ventanaAnalizadorSize*2;
        bufferSizeInBytesAn = bufferSizeInFramesAn*2;

        buffersAudio = new byte[cantidadBuffersAudio][bufferSizeInBytesEQ];

        mutexLecturaAudioReproductor = new Semaphore[cantidadBuffersAudio];
        mutexLecturaAudioAnalizador = new Semaphore[cantidadBuffersAudio];
        mutexEscrituraAudioReproductor = new Semaphore[cantidadBuffersAudio];
        mutexEscrituraAudioAnalizador = new Semaphore[cantidadBuffersAudio];

        buffersEspectro = new float[cantidadBuffersEspectro][cantidadBandasAnalizador];
        mutexLecturaEspectro = new Semaphore[cantidadBuffersEspectro];
        mutexEscrituraEspectro = new Semaphore[cantidadBuffersEspectro];

        resetearMutex();
        // fin inicializacion de buffersAudio, buffersEspectro y semaforos
       
        // inicializacion de audio track
        track = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRateInHz, channelConfig, audioFormat, bufferSizeInBytesEQ , AudioTrack.MODE_STREAM );
        // fin inicializacion audio track

        // inicializacion de libreria para filtro y analizador
        lib = new LibC( ventanaEQsize, ventanaAnalizadorSize, fmtChunkD.samplesPerSec, fmtChunkD.channels, cantidadBandasAnalizador );
        // fin inicializacion de libreria
        
        // Inicializo parametros graficador
        holderGraficador = getHolder();
        holderGraficador.addCallback(this);
        // Fin graficador
    }

    // destructor (debe llamarse explicitamente)
    public void close()
    {
        botonStop();

        track.release();

        lib.close();
    }

    public void botonPlay()
    {
        //todos los threads deberian morirse juntos, si alguno esta vivo, entonces volve a tocar el boton mas tarde
        if ( !(procesadorAudioT==null || reproductorT==null || analizadorT==null || graficadorT==null) ) {
            if ( procesadorAudioT.isAlive() || reproductorT.isAlive() || analizadorT.isAlive() || graficadorT.isAlive() ) {
                return;
            }
        }

        // limpiamos mutexes y EQ y analizador
        resetearMutex();
        lib.limpiarEQYAnalizador();
        // fin limpiar
        

        // inicializacion de periodo para despertar al graficador
        refreshRateAnalizadorActual = refreshRateAnalizadorSeleccionada;
        track.setPositionNotificationPeriod(bufferSizeInFramesEQ * refreshRateAnalizadorActual / canales);
        track.setPlaybackPositionUpdateListener(frameListener);
        // fin inicializacion del periodo del graficador

        // le damos play para que empiece a reproducir apenas le demos audio
        track.play();
        //

        // inicializacion de threads
        procesadorAudioT = new Thread( new procesarAudio() );
        reproductorT = new Thread( new reproducir() );
        analizadorT = new Thread( new procesarEspectro() );
        graficadorT = new Thread( new graficador(holderGraficador) );
        // fin inicializacion de threads

        // arrancamos los threads
        Log.d("PLAY","inicio1");
        procesadorAudioT.start();
            // esto puede que no srva para nada
            //procesadorAudioT.setPriority(Thread.MAX_PRIORITY);
        Log.d("PLAY","inicio2");
        reproductorT.start();
        Log.d("PLAY","inicio3");
        analizadorT.start();
        Log.d("PLAY","inicio4");
        graficadorT.start();
        // fin arrancar threas
    }

    public void botonStop()
    {
        Log.d("BotonStop","apretaste Stop, vamo a ver que pasa");

        // tengo que matar a todos los threads
        try {
            if ( procesadorAudioT.isAlive() ) {
                procesadorAudioT.interrupt();
                procesadorAudioT.join();
                Log.d("BotonStop","join1");
            }
            if ( reproductorT.isAlive() ) {
                reproductorT.interrupt();
                reproductorT.join();
                Log.d("BotonStop","join2");
            }
            if ( analizadorT.isAlive() ) {
                analizadorT.interrupt();
                analizadorT.join();
                Log.d("BotonStop","join3");
            }
            if ( graficadorT.isAlive() ) {
                graficadorT.interrupt();
                graficadorT.join();
                Log.d("BotonStop","join4");
            }
        } catch (Exception e) {
            Log.d("BotonStop","no me joinie, va a haber quilombo");
        }

        track.flush();
        track.stop();

        procesadorAudioT = null;
        reproductorT = null;
        analizadorT = null;
        graficadorT = null;

        Log.d("BotonStop","Me joinie con todos, no deberia haber quilombo");
    }

    public class procesarAudio implements Runnable
    {
        public void run()
        {
            BufferedInputStream buffBytesWAV = null;

            try{
                Log.d("procesador","voy a inicializar el WAV");
                //              // automaticamente convertimos el tema (esto es para test, luego sacarlo y poner un boton para convertir
                //              lib.ecualizarYEscribirCancion( miWAV._nomArchivo );

                // inicializacion archivo de lectura
                buffBytesWAV = new BufferedInputStream( new FileInputStream(miWAV._nomArchivo) );
                buffBytesWAV.skip(44);                        //Salteate 44 maestro
                // fin inicializacion archivo de lectura

                int bufferActual = 0;
                byte bufferSong[] = new byte[bufferSizeInBytesEQ];

                int lei;

                Log.d("procesador","voy empezar a procesar");

                while( ( lei = buffBytesWAV.read( bufferSong, 0, bufferSizeInBytesEQ ) ) == bufferSizeInBytesEQ ) {

                    mutexUsarEQ.readLock().lock();
                    mutexUsarEQ.readLock().unlock();
                    mutexCambiarEQ.readLock().lock();
                    lib.ecualizar( bufferSong );                
                    mutexCambiarEQ.readLock().unlock();

                    mutexEscrituraAudioReproductor[ bufferActual%cantidadBuffersAudio ].acquire();
                    mutexEscrituraAudioAnalizador[ bufferActual%cantidadBuffersAudio ].acquire();
                    System.arraycopy( bufferSong, 0, buffersAudio[bufferActual%cantidadBuffersAudio], 0, bufferSizeInBytesEQ );
                    mutexLecturaAudioReproductor[ bufferActual%cantidadBuffersAudio ].release();        // una pa cada uno
                    mutexLecturaAudioAnalizador[ bufferActual%cantidadBuffersAudio ].release();

                    bufferActual++;

                }

                if ( lei == -1 ) {
                    Arrays.fill( bufferSong, (byte) 0 );
                } else {
                    Arrays.fill( bufferSong, lei, bufferSizeInBytesEQ-1, (byte) 0 );
                }
                
                mutexUsarEQ.readLock().lock();
                mutexUsarEQ.readLock().unlock();
                mutexCambiarEQ.readLock().lock();
                lib.ultimo( bufferSong );
                mutexCambiarEQ.readLock().unlock();
                
                mutexEscrituraAudioReproductor[ bufferActual%cantidadBuffersAudio ].acquire();
                mutexEscrituraAudioAnalizador[ bufferActual%cantidadBuffersAudio ].acquire();
                System.arraycopy( bufferSong, 0, buffersAudio[bufferActual%cantidadBuffersAudio], 0, bufferSizeInBytesEQ );
                mutexLecturaAudioReproductor[ bufferActual%cantidadBuffersAudio ].release();
                mutexLecturaAudioAnalizador[ bufferActual%cantidadBuffersAudio ].release();

                // cierro el archivo
                buffBytesWAV.close();

            }catch( Exception e ){
                try{
                    // cierro el archivo
                    if ( !(buffBytesWAV==null) ) {
                        buffBytesWAV.close();
                    }
                }catch( IOException ioe ) {}

                Log.d("Procesador","me voy a cerrar");
                return;
            }
        }
    }

    public class reproducir implements Runnable
    {
        public void run()
        {    
            try {
                Log.d("BufferSizeInBytes",""+bufferSizeInBytesEQ);

                int bufferActual = 0;

                while ( true ) {                
                    mutexLecturaAudioReproductor[ bufferActual%cantidadBuffersAudio ].acquire();
                    track.write( buffersAudio[ bufferActual%cantidadBuffersAudio ], 0, bufferSizeInBytesEQ );
                    mutexEscrituraAudioReproductor[ bufferActual%cantidadBuffersAudio ].release();

                    bufferActual++;
                }
            } catch ( Exception ex ) {
                Log.d("Reproductor","me voy a cerrar");
                return;
            }
        }
    }

    public class procesarEspectro implements Runnable
    {
        public void run()
        {
            try {
                int bufferActualEscritura = 0;
                int bufferActualLectura = 0;
                int refresh = 0;
                byte bufferSong[] = new byte[bufferSizeInBytesAn];
                float[] bufferEspectro = new float[cantidadBandasAnalizador];

                Log.d("procesarEspectro","Efectivamente, entro");

                while( true ) {

                    mutexLecturaAudioAnalizador[ bufferActualLectura%cantidadBuffersAudio ].acquire();
                    System.arraycopy( buffersAudio[bufferActualLectura%cantidadBuffersAudio], 0, bufferSong, 0, bufferSizeInBytesEQ );
                    mutexEscrituraAudioAnalizador[ bufferActualLectura%cantidadBuffersAudio ].release();

                    lib.analizar( bufferSong );

                    bufferActualLectura++;

                    if ( refresh == refreshRateAnalizadorActual ) {
                        lib.dameAnalisis( bufferEspectro );

                        mutexEscrituraEspectro[ bufferActualEscritura%cantidadBuffersEspectro ].acquire();
                        System.arraycopy( bufferEspectro, 0, buffersEspectro[bufferActualEscritura%cantidadBuffersEspectro], 0, cantidadBandasAnalizador );
                        mutexLecturaEspectro[ bufferActualEscritura%cantidadBuffersEspectro ].release();
                        bufferActualEscritura++;
                        refresh = 0;
                    }

                    refresh++;    

                }
            } catch (Exception e) {
                Log.d("espectro","me voy a cerrar");
                return;
            }
        }
    }

    public class graficador extends Thread implements Runnable
    {
        private SurfaceHolder _surfaceHolder;
        private Paint _pinturas[] = new Paint[5];

        // constructor
        public graficador(SurfaceHolder surfaceHolder){
            _surfaceHolder = surfaceHolder;

            for(int i=0;i<5;i++){
                _pinturas[i] = new Paint();
            }

            _pinturas[0].setARGB(0xFF, 152, 252,102);
            _pinturas[1].setARGB(0xFF,  51, 153,  0);
            _pinturas[2].setARGB(0xFF, 253, 202,  1);
            _pinturas[3].setARGB(0xFF, 244,  72,  0);
            _pinturas[4].setARGB(0xFF, 212,   0,  0);

        }

        public void dibujar_barra( Canvas canvasBarritasEspectro, float valor, int corrimiento )
        {
            // usamos 5 colores
            int indexColor = corrimiento / (cantidadBandasAnalizador/5);
            float tamBase = 8;
            float tamAltura = 400;

            float izq = 2;
            float arr = 2 + (tamBase+2)*corrimiento;
            canvasBarritasEspectro.drawRect( izq+2, arr, izq+2 + tamAltura*valor, arr + tamBase, _pinturas[indexColor] );
        }

        @Override
        public void run()
        {
            try {
                int bufferActualLectura = 0;
                byte bufferSong[] = new byte[bufferSizeInBytesAn];
                float[] bufferEspectro = new float[cantidadBandasAnalizador];

                while(true){

                    mutexLecturaEspectro[ bufferActualLectura %cantidadBuffersEspectro ].acquire();
                    System.arraycopy(buffersEspectro[bufferActualLectura%cantidadBuffersEspectro], 0,  bufferEspectro, 0, cantidadBandasAnalizador );
                    mutexEscrituraEspectro[ bufferActualLectura%cantidadBuffersEspectro ].release();

                    semaphoreHayQueGraficar.acquire();
                    //Me dijeron que grafique

                    // para graficar
                    Canvas canvasBarritasEspectro = _surfaceHolder.lockCanvas();
                    canvasBarritasEspectro.drawARGB(0xFF,0xee,0xee,0xee);

                    for ( int i=0 ; i<cantidadBandasAnalizador ; i++) {
                        // tmp esta en rango [-inf,0]
                        float tmp = bufferEspectro[i];
                        // mostrar esta en rango [0,1]
                        // voy a usar el threshold
                        mutexThresholdAN.readLock().lock();
                        float mostrar = (float) ((tmp < -thresholdAnalizador)? 0. : (tmp+thresholdAnalizador+ceilBandas)/thresholdAnalizador) ;
                        mutexThresholdAN.readLock().unlock();
//                        if ( i<19 && i>15 ) {
//                            Log.d("valor " + i, ""+mostrar);
//                        }
                        dibujar_barra(canvasBarritasEspectro,mostrar,i);

                    }
                    _surfaceHolder.unlockCanvasAndPost(canvasBarritasEspectro);

                    bufferActualLectura++;
                }
            } catch (Exception e) {
                // reseteo el dibujito
                Canvas canvasBarritasEspectro = _surfaceHolder.lockCanvas();
                canvasBarritasEspectro.drawARGB(0xFF,0xee,0xee,0xee);
                _surfaceHolder.unlockCanvasAndPost(canvasBarritasEspectro);

                Log.d("Graficador","me voy a cerrar");
                return;
            }
        }
    }

    private void resetearMutex()
    {
        for ( int i=0 ; i<cantidadBuffersAudio ; i++ ) {
            mutexLecturaAudioReproductor[i] = new Semaphore(0);
            mutexLecturaAudioAnalizador[i] = new Semaphore(0);
            mutexEscrituraAudioReproductor[i] = new Semaphore(1);
            mutexEscrituraAudioAnalizador[i] = new Semaphore(1);
        }

        for ( int i=0 ; i<cantidadBuffersEspectro ; i++ ) {
            mutexLecturaEspectro[i] = new Semaphore(0);
            mutexEscrituraEspectro[i] = new Semaphore(1);
        }    

        semaphoreHayQueGraficar = new Semaphore(0);

        mutexBandasEQ = new ReentrantReadWriteLock();
        mutexCambiarEQ = new ReentrantReadWriteLock();
        mutexThresholdAN = new ReentrantReadWriteLock();
        mutexUsarEQ = new ReentrantReadWriteLock();
    }

    // cuando se quiera tener la ecualizacion guardad en estadoBandasEq se llama
    // esta funcion
    public void modificarEQ()
    {
        // voy a leer los parametros
        mutexBandasEQ.readLock().lock();
        // y voy a modificar la respeusta del EQ
        mutexCambiarEQ.writeLock().lock();
        lib.actualizarFiltroEQ( estadoBandasEq );
        mutexCambiarEQ.writeLock().unlock();
        mutexBandasEQ.readLock().unlock();
    }

    public void apagarEQ()
    {
        // y voy a modificar la respeusta del EQ
        mutexCambiarEQ.writeLock().lock();
        lib.actualizarFiltroEQ( noEcualizar );
        mutexCambiarEQ.writeLock().unlock();
    }


    public void modificarSlidesEQ( float p1, float p2, float p3, float p4, float p5 )
    {
        mutexBandasEQ.writeLock().lock();

        // modificar el arreglo estadoBandasEq
        estadoBandasEq[0] = p1;
        estadoBandasEq[1] = p2;
        estadoBandasEq[2] = p3;
        estadoBandasEq[3] = p4;
        estadoBandasEq[4] = p5;

        Log.d("SLIDES",""+p1+" "+p2+" "+p3+" "+p4+" "+p5);
        
        mutexBandasEQ.writeLock().unlock();

        modificarEQ();
    }

    public void modificarThresholdAnalizador( int dB )
    {
        // dB esta entre 0 y 50
        mutexThresholdAN.writeLock().lock();
        thresholdAnalizador = dB;
        mutexThresholdAN.writeLock().unlock();
    }

    public void cambiarUsarEQ()
    {
        if ( usarEQ ) {
            // si estabamos ecaulizando
            apagarEQ();
        } else {
            modificarEQ();
        }
        // ahora hacemos lo opuesto de lo que veniamos haciendo
        usarEQ = !usarEQ;  
    }

    public void setearRefreshRateAnalizador( int ref )
    {
        // debe ser un entero mayor o igual a 1
        if (ref <1) ref=1;
        refreshRateAnalizadorSeleccionada = ref;
    }

    private AudioTrack.OnPlaybackPositionUpdateListener frameListener = new AudioTrack.OnPlaybackPositionUpdateListener()
    {
        int refreshTimes = 0;

         // A fines practicos esto no sirve aca
        @Override
        public void onMarkerReached(AudioTrack track) {}

        @Override
        public void onPeriodicNotification(AudioTrack track)
        {
            // si ya me llamaron la duficiente cantidad de veces, graficar!, si no chau
//            if (refreshTimes >= refreshRateAnalizadorActual ) {
                semaphoreHayQueGraficar.release();
//                refreshTimes = 0;
//            } else {
//                refreshTimes++;
//            }
        }
    };

    @Override
    public void surfaceCreated(SurfaceHolder holder)
    {
        Canvas canvasBarritasEspectro = holder.lockCanvas();
        canvasBarritasEspectro.drawARGB(0xFF,0xee,0xee,0xee);
        holder.unlockCanvasAndPost(canvasBarritasEspectro);
    }

    @Override
    public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) {}

    @Override
    public void surfaceDestroyed(SurfaceHolder arg0){}
}
