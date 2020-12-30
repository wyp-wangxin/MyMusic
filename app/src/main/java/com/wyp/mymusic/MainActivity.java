package com.wyp.mymusic;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import com.wyp.mymusic.R;
import com.wyp.myplayer.listener.WlOnParparedListener;
import com.wyp.myplayer.log.MyLog;
import com.wyp.myplayer.player.WlPlayer;


public class MainActivity extends AppCompatActivity {
    private WlPlayer wlPlayer;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        wlPlayer = new WlPlayer();
        wlPlayer.setWlOnParparedListener(new WlOnParparedListener() {
            @Override
            public void onParpared() {
                MyLog.d("准备好了，可以开始播放声音了");
                wlPlayer.start();
            }
        });
    }

    public void begin(View view) {

        wlPlayer.setSource("http://v-cdn.zjol.com.cn/280443.mp4");
        wlPlayer.parpared();

    }
}
