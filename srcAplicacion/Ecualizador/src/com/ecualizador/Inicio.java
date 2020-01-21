package com.ecualizador;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.List;


import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

class SoloWav implements FilenameFilter
{

    @Override
    public boolean accept(File dir, String filename) {

        if ( filename.contains(".") ){
            return filename.endsWith(".wav");
        }
        return true;
    }
}


public class Inicio extends Activity
{ 

    String rutaActual = "/mnt/sdcard";
    SoloWav filtroWAV = new SoloWav();

    private void cargarListaInicio(String strDir)
    {

        File directorio = new File( strDir );                           //Agarro el directorio que me pasaron

        List<String> ficheros = new ArrayList<String>();        //Armo Lista de Strings
        ficheros.add("..");                                                                     //Pongo el ..

        String[] datos = directorio.list(filtroWAV);            //Le pido los archivos del directorio

        for( int l = 0 ; l < datos.length ; l++){
            ficheros.add( datos[l]);                                                //Los agrego a la Lista
        }

        ArrayAdapter<String> adaptadorLista = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, ficheros.toArray(datos) );
        //Armo una lista visual

        ListView lstInicio = (ListView) findViewById(R.id.txtChannels); 
        lstInicio.setAdapter(adaptadorLista);                                                   //La muestro

    }

    private OnItemClickListener itemListener = new OnItemClickListener()
    {

        //Este es el Listener de los Items, cuando les hacen click se ejecuta
        public void onItemClick(AdapterView<?> adaptador, View v, int position, long id)
        {

            String opcionSeleccionada =  (String) adaptador.getAdapter().getItem(position);

            TextView txtInicio = (TextView) findViewById(R.id.txtInicio);

            //Si la opcion seleccionada es un .wav, tengo que abrirlo, me voy a otra pantalla

            if( opcionSeleccionada.endsWith(".wav") ){
                //Quieren abrir un WAV!!!!!!

                Intent cambioPantalla = new Intent( Inicio.this, Informacion.class );
                Bundle miBundle = new Bundle();
                miBundle.putString("archivo", rutaActual + "/" + opcionSeleccionada);
                cambioPantalla.putExtras(miBundle);
                startActivity(cambioPantalla);
            }else{

                if( opcionSeleccionada.equals("..")){
                    rutaActual = rutaActual.substring(0, rutaActual.lastIndexOf("/") );
                }else{
                    rutaActual = rutaActual + "/" + opcionSeleccionada ;
                }                                                                                                                       //Actualizo el nombre, depende si estan entrando o saliendo

                txtInicio.setText(rutaActual );                                                         //Lo muestro
                cargarListaInicio( rutaActual );                                                        //Cargo la lista
            }
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.inicio);

        //ESTE ES EL MAIN PAPA

        //        LibC testLib = new LibC(0,0,0,0,0);

        TextView txtInicio = (TextView) findViewById(R.id.txtInicio);

        //        int a[] = {1,2,3,4};
        //        int b[] = {2,3,4,5};

        //        txtInicio.setText(rutaActual +' '+ testLib.nada(a,b));                                                                  //Aranco en '/mnt/sd'
        cargarListaInicio(rutaActual);                                                                  //Cargo la lista

        ListView lstInicio = (ListView) findViewById(R.id.txtChannels );
        lstInicio.setOnItemClickListener( itemListener );    
    }

}
