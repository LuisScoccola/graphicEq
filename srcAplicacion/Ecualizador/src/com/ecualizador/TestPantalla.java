package com.ecualizador;


import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class TestPantalla extends SurfaceView  implements SurfaceHolder.Callback{

    Thread _threadTEST; 


    private class GameThread extends Thread {

        /* Handle to the surface manager object we interact with */
        private SurfaceHolder _surfaceHolder;
        private Paint _paint;

        public GameThread(SurfaceHolder surfaceHolder, Context context, Handler handler){
            _surfaceHolder = surfaceHolder;
            _paint = new Paint();
        }

        @Override
        public void run() {
            int i = 0;

            while(i++ < 10){
                Canvas canvas = _surfaceHolder.lockCanvas();



                //set the colour
                _paint.setARGB(0xFF, i*0x10, i*0x10, 0);

                //draw the ball
                canvas.drawRect(new Rect(i*10,i*10,i*10 + 100,i*10 + 100),_paint);

                _surfaceHolder.unlockCanvasAndPost(canvas);
            }
        }

    }


    public TestPantalla(int a, Context context){
        super( context );

        //So we can listen for events...
        SurfaceHolder holder = getHolder();
        holder.addCallback(this);
        setFocusable(true); 

        //and instantiate the thread
        _threadTEST = new Thread( new GameThread(holder, context, null) );

        Log.d("LA ** QUE ENTRO", ""+ a);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width,
            int height) {
        // TODO Auto-generated method stub

    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // TODO Auto-generated method stub
        _threadTEST.start();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        // TODO Auto-generated method stub
        // cerrar thread o fijarse  BLA

    }




}

