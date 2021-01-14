package com.wyp.mymusic;

import android.media.MediaCodec;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import com.wyp.myplayer.WlTimeInfoBean;
import com.wyp.myplayer.listener.WlOnCompleteListener;
import com.wyp.myplayer.listener.WlOnErrorListener;
import com.wyp.myplayer.listener.WlOnLoadListener;
import com.wyp.myplayer.listener.WlOnParparedListener;
import com.wyp.myplayer.listener.WlOnPauseResumeListener;
import com.wyp.myplayer.listener.WlOnTimeInfoListener;
import com.wyp.myplayer.listener.WlOnValumeDBListener;
import com.wyp.myplayer.log.MyLog;
import com.wyp.myplayer.muteenum.MuteEnum;
import com.wyp.myplayer.player.WlPlayer;
import com.wyp.myplayer.util.WlTimeUtil;

import java.io.File;


public class MainActivity extends AppCompatActivity {
    private WlPlayer wlPlayer;
    private TextView tvTime;

    private SeekBar seekBarSeek;
    private int position = 0;
    private boolean isSeekBar = false;

    private SeekBar seekBarVolume;
    private TextView tvVolume;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tvTime = findViewById(R.id.tv_time);
        seekBarSeek = findViewById(R.id.seekbar_seek);
        seekBarVolume= findViewById(R.id.seekbar_volume);
        tvVolume = findViewById(R.id.tv_volume);

        wlPlayer = new WlPlayer();
        tvVolume.setText("音量：" + wlPlayer.getVolumePercent() + "%");
        seekBarVolume.setProgress(wlPlayer.getVolumePercent());
        wlPlayer.setMute(MuteEnum.MUTE_CENTER);//设置声道立体声

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

        wlPlayer.setWlOnCompleteListener(new WlOnCompleteListener() {
            @Override
            public void onComplete() {
                MyLog.d("播放完成了");
            }
        });

        wlPlayer.setWlOnValumeDBListener(new WlOnValumeDBListener() {
            @Override
            public void onDbValue(int db) {
                MyLog.d("当前DB : "+db);
            }
        });
        seekBarSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if(wlPlayer.getDuration() > 0 && isSeekBar)
                {
                    position = wlPlayer.getDuration() * progress / 100;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isSeekBar = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                wlPlayer.seek(position);
                isSeekBar = false;
            }
        });

        seekBarVolume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                wlPlayer.setVolume(progress);
                tvVolume.setText("音量：" + wlPlayer.getVolumePercent() + "%");
                MyLog.d("progress is " + progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

    }



    public void begin(View view) {

        wlPlayer.setSource("http://v-cdn.zjol.com.cn/276982.mp4");
        wlPlayer.parpared();
         MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();


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
                if(!isSeekBar)
                {
                    WlTimeInfoBean wlTimeInfoBean = (WlTimeInfoBean) msg.obj;
                    tvTime.setText(WlTimeUtil.secdsToDateFormat(wlTimeInfoBean.getTotalTime(), wlTimeInfoBean.getTotalTime())
                            + "/" + WlTimeUtil.secdsToDateFormat(wlTimeInfoBean.getCurrentTime(), wlTimeInfoBean.getTotalTime()));

                    seekBarSeek.setProgress(wlTimeInfoBean.getCurrentTime() * 100 / wlTimeInfoBean.getTotalTime());

                }
            }
        }
    };

    public void stop(View view) {
        wlPlayer.stop();
    }

    public void seek(View view) {
        wlPlayer.seek(15);
    }

    public void next(View view) {
        wlPlayer.playNext("http://v-cdn.zjol.com.cn/276984.mp4");
    }

    public void speed(View view) {
        wlPlayer.setSpeed(1.5f);
        wlPlayer.setPitch(1.0f);
    }

    public void pitch(View view) {
        wlPlayer.setPitch(1.5f);
        wlPlayer.setSpeed(1.0f);
    }

    public void speedpitch(View view) {
        wlPlayer.setSpeed(1.5f);
        wlPlayer.setPitch(1.5f);
    }

    public void normalspeedpitch(View view) {
        wlPlayer.setSpeed(1.0f);
        wlPlayer.setPitch(1.0f);
    }

    public void left(View view) {
        wlPlayer.setMute(MuteEnum.MUTE_LEFT);
    }

    public void right(View view) {
        wlPlayer.setMute(MuteEnum.MUTE_RIGHT);
    }

    public void center(View view) {
        wlPlayer.setMute(MuteEnum.MUTE_CENTER);
    }

    public void start_record(View view) {

        //wlPlayer.startRecord(new File("/mnt/shared/Other/textplayer-1.aac"));
        wlPlayer.startRecord(new File("/storage/emulated/0/Pictures/textplayer-1.aac"));

    }

    public void pause_record(View view) {
        wlPlayer.pauseRecord();
    }

    public void goon_record(View view) {
        wlPlayer.resumeRcord();
    }

    public void stop_record(View view) {
        wlPlayer.stopRecord();
    }
}
