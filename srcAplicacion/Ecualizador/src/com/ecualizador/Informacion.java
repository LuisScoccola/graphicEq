package com.ecualizador;

import com.ecualizador.Programa;
import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.RelativeLayout.LayoutParams;

public class Informacion extends Activity {

    Wave miWAV = null;
    Programa programa = null;

    SeekBar refreshAn = null;
    SeekBar thresholdAn = null;
    SeekBar ecu1 = null;
    SeekBar ecu2 = null;
    SeekBar ecu3 = null;
    SeekBar ecu4 = null;
    SeekBar ecu5 = null;

    TextView dbEcu1 = null;
    TextView dbEcu2 = null;
    TextView dbEcu3 = null;
    TextView dbEcu4 = null;
    TextView dbEcu5 = null;
    
    TextView dbRA   = null;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.informacion);                                   //Aca se seteo la vista

        Bundle bundleLlego = this.getIntent().getExtras();              //Recibo lo de la otra pantalla
        String nombreWAV = bundleLlego.getString("archivo");

        // solo inicializo el programa y los botones si el wav es valido
        try {
            miWAV = new Wave(nombreWAV);                                                //Creo una estructura WAV

            if ( !miWAV._valido ) {
                throw new Exception();
            }
        } catch (Exception e) {
            TextView txtChannels = (TextView) findViewById(R.id.txtChannels);
            txtChannels.setText("NO PCM");

            Log.d("INFORMACION","El archivo no es PCM");

            return;
        }      

        // imporimo los datos del wav
        imprimirData(miWAV);                                                                //Si es valida imprimo la data

        // seteo botones y slides
        Button btnPlay = (Button) findViewById(R.id.btnPlay);
        Button btnStop = (Button) findViewById(R.id.btnStop);
        Button btnEqOnOff = (Button) findViewById(R.id.btnEqOnOff);
        refreshAn = (SeekBar) findViewById(R.id.refreshAn);
        thresholdAn = (SeekBar) findViewById(R.id.thresholdAn);
        ecu1 = (SeekBar) findViewById(R.id.ecu_1);
        ecu2 = (SeekBar) findViewById(R.id.ecu_2);
        ecu3 = (SeekBar) findViewById(R.id.ecu_3);
        ecu4 = (SeekBar) findViewById(R.id.ecu_4);
        ecu5 = (SeekBar) findViewById(R.id.ecu_5);

        dbEcu1 = (TextView) findViewById(R.id.dbEcu1);
        dbEcu2 = (TextView) findViewById(R.id.dbEcu2);
        dbEcu3 = (TextView) findViewById(R.id.dbEcu3);
        dbEcu4 = (TextView) findViewById(R.id.dbEcu4);
        dbEcu5 = (TextView) findViewById(R.id.dbEcu5);

        dbRA  = (TextView) findViewById(R.id.dbRA);

        btnPlay.setOnClickListener( clickListener );        //Agrego los listeners para los botones
        btnStop.setOnClickListener( clickListener );
        btnEqOnOff.setOnClickListener( clickListener );
        refreshAn.setOnSeekBarChangeListener(seekbarListener);
        thresholdAn.setOnSeekBarChangeListener(seekbarListener);
        ecu1.setOnSeekBarChangeListener(seekbarListener);
        ecu2.setOnSeekBarChangeListener(seekbarListener);
        ecu3.setOnSeekBarChangeListener(seekbarListener);
        ecu4.setOnSeekBarChangeListener(seekbarListener);
        ecu5.setOnSeekBarChangeListener(seekbarListener);

        // refresh empieza en 1
        refreshAn.setProgress(1);
        // empieza en -15dB
        thresholdAn.setProgress(15);
        // empiezan en 1/6
        ecu1.setProgress(10);
        ecu2.setProgress(10);
        ecu3.setProgress(10);
        ecu4.setProgress(10);
        ecu5.setProgress(10);

        Log.d("Informacion","ejecuto el programa");                

        programa = new Programa( miWAV, this );

        // comunico lo sbotos y slides con el programa
        LayoutParams lparams = new LayoutParams(LayoutParams.WRAP_CONTENT,600);
        lparams.addRule( RelativeLayout.BELOW,R.id.ecu_5);
        programa.setLayoutParams(lparams);

        RelativeLayout relativeLayoutInformacion = (RelativeLayout) findViewById(R.id.relativeLayoutInformacion);

        relativeLayoutInformacion.addView(programa);
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();

        // si el programa fue inicializado lo cierro
        if ( ! (programa==null) ) {
            programa.close();
            programa = null;
        }
    }

    private SeekBar.OnSeekBarChangeListener seekbarListener = new SeekBar.OnSeekBarChangeListener()
    {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {}

        @Override
        public void onStartTrackingTouch(SeekBar seekBar){}

        @Override
        public void onStopTrackingTouch(SeekBar seekBar)
        {
            if ( seekBar.getId() == R.id.refreshAn ) {
                programa.setearRefreshRateAnalizador( seekBar.getProgress() );
            }else if( seekBar.getId() == R.id.thresholdAn  ){
                programa.modificarThresholdAnalizador( seekBar.getProgress() ); 
                dbRA.setText("-"+seekBar.getProgress()+"db" );
            }else{
                // estan entre 0 y 60 
                // se puede subir un dB y bajar hasta 5 
                float param1 = -ecu1.getProgress()/10 + (float) 1;
                float param2 = -ecu2.getProgress()/10 + (float) 1;
                float param3 = -ecu3.getProgress()/10 + (float) 1;
                float param4 = -ecu4.getProgress()/10 + (float) 1;
                float param5 = -ecu5.getProgress()/10 + (float) 1;
                // ahora entre +1 y -5

               dbEcu1.setText( ( (param1>0)? "+":"") + param1 + " db" );
               dbEcu2.setText( ( (param2>0)? "+":"") + param2 + " db" );
               dbEcu3.setText( ( (param3>0)? "+":"") + param3 + " db" );
               dbEcu4.setText( ( (param4>0)? "+":"") + param4 + " db" );
               dbEcu5.setText( ( (param5>0)? "+":"") + param5 + " db" );

                programa.modificarSlidesEQ( param1, param2, param3, param4, param5 );
            }
        }
    };

    private View.OnClickListener clickListener = new View.OnClickListener()
    {
        public void onClick( View clickeado )
        {
            switch(clickeado.getId()){

                case R.id.btnPlay:
                    programa.botonPlay();
                    break;

                case R.id.btnStop:
                    programa.botonStop();
                    break;
               
                case R.id.btnEqOnOff:
                    programa.cambiarUsarEQ();
                    break;
            }
        }
    };


    @Override
    public void onBackPressed(){
        if(!(programa==null)) {
            programa.close();
            programa=null;
        }
        finish();
    }

    private void imprimirData(Wave miWAV)
    {
        TextView nmbCancion  = (TextView) findViewById(R.id.nmbCancion);
        TextView txtChannels = (TextView) findViewById(R.id.txtChannels);
        TextView txtSamplesPS = (TextView) findViewById(R.id.txtSamplesPS);
        TextView txtAvgBytesPS = (TextView) findViewById(R.id.txtAvgBytesPS);
        TextView txtBitsPerSample = (TextView) findViewById(R.id.txtBitsPerSample);
        TextView txtDuration = (TextView) findViewById(R.id.txtDuration);

        nmbCancion.setText( miWAV._nomArchivo );

        txtChannels.setText(            "Channels: " + fmtChunkD.channels );
        txtSamplesPS.setText(           "Samples/sec.: " + fmtChunkD.samplesPerSec );
        txtAvgBytesPS.setText(          "Bytes/sec.: " + fmtChunkD.avgBytesPerSec );
        txtBitsPerSample.setText(       "Bits/Sample: " + fmtChunkD.bitsPerSample );
        txtDuration.setText(            "Duration: " + miWAV._duration + " s" );
    }
}
