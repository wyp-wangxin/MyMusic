package com.wyp.mymusic;

import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import com.wyp.myplayer.WlTimeInfoBean;
import com.wyp.myplayer.listener.WlOnErrorListener;
import com.wyp.myplayer.listener.WlOnLoadListener;
import com.wyp.myplayer.listener.WlOnParparedListener;
import com.wyp.myplayer.listener.WlOnPauseResumeListener;
import com.wyp.myplayer.listener.WlOnTimeInfoListener;
import com.wyp.myplayer.log.MyLog;
import com.wyp.myplayer.player.WlPlayer;
import com.wyp.myplayer.util.WlTimeUtil;


public class MainActivity extends AppCompatActivity {
    private WlPlayer wlPlayer;
    private TextView tvTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tvTime = findViewById(R.id.tv_time);
        wlPlayer = new WlPlayer();
        wlPlayer.setWlOnParparedListener(new WlOnParparedListener() {
            @Override
            public void onParpared() {
                MyLog.d("准备好了，可以开始播放声音了");
                wlPlayer.start();
            }
        });
        wlPlayer.setWlOnLoadListener(new WlOnLoadListener() {
            @Override
            public void onLoad(Boolean bool) {
                if(bool){
                    MyLog.d("123 get child thread jnienv worng 8");
                   // Toast.makeText(getApplicationContext(),"加载中...",Toast.LENGTH_SHORT).show();
                }else {
                    MyLog.d("123 get child thread jnienv worng 9");
                   // Toast.makeText(getApplicationContext(),"播放中...",Toast.LENGTH_SHORT).show();
                }
            }
        });

        wlPlayer.setWlOnPauseResumeListener(new WlOnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if(pause)
                {
                    MyLog.d("暂停中...");
                }
                else
                {
                    MyLog.d("播放中...");
                }
            }
        });

        wlPlayer.setWlOnTimeInfoListener(new WlOnTimeInfoListener() {
            @Override
            public void onTimeInfo(WlTimeInfoBean timeInfoBean) {
//                MyLog.d(timeInfoBean.toString());
                Message message = Message.obtain();
                message.what = 1;
                message.obj = timeInfoBean;
                handler.sendMessage(message);

            }
        });

        wlPlayer.setWlOnErrorListener(new WlOnErrorListener() {
            @Override
            public void onError(int code, String msg) {
                MyLog.d("code:" + code + ", msg:" + msg);
            }
        });
    }



    public void begin(View view) {

        wlPlayer.setSource("http://v-cdn.zjol.com.cn/276982.mp4");
        wlPlayer.parpared();

    }

    public void pause(View view) {

        wlPlayer.pause();

    }

    public void resume(View view) {
        wlPlayer.resume();
    }
    Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if(msg.what == 1)
            {
                WlTimeInfoBean wlTimeInfoBean = (WlTimeInfoBean) msg.obj;
                tvTime.setText(WlTimeUtil.secdsToDateFormat(wlTimeInfoBean.getTotalTime(), wlTimeInfoBean.getTotalTime())
                        + "/" + WlTimeUtil.secdsToDateFormat(wlTimeInfoBean.getCurrentTime(), wlTimeInfoBean.getTotalTime()));
            }
        }
    };

    public void stop(View view) {
        wlPlayer.stop();
    }
}
