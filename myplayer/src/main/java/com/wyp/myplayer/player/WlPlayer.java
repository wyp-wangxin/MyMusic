package com.wyp.myplayer.player;

import android.text.TextUtils;

import com.wyp.myplayer.WlTimeInfoBean;
import com.wyp.myplayer.listener.WlOnCompleteListener;
import com.wyp.myplayer.listener.WlOnErrorListener;
import com.wyp.myplayer.listener.WlOnLoadListener;
import com.wyp.myplayer.listener.WlOnParparedListener;
import com.wyp.myplayer.listener.WlOnPauseResumeListener;
import com.wyp.myplayer.listener.WlOnTimeInfoListener;
import com.wyp.myplayer.log.MyLog;

/**
 * Created by yangw on 2018-2-28.
 */

public class WlPlayer {

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
    }

    private  String source;//数据源
    private static boolean playNext = false;
    private static WlTimeInfoBean wlTimeInfoBean;

    private WlOnParparedListener wlOnParparedListener;
    private WlOnLoadListener wlOnLoadListener;
    private WlOnPauseResumeListener wlOnPauseResumeListener;
    private WlOnTimeInfoListener wlOnTimeInfoListener;
    private WlOnErrorListener wlOnErrorListener;
    private WlOnCompleteListener wlOnCompleteListener;

    public WlPlayer()
    {}

    /**
     * 设置数据源
     * @param source
     */
    public void setSource(String source)
    {
        this.source = source;
    }

    /**
     * 设置准备接口回调
     * @param wlOnParparedListener
     */
    public void setWlOnParparedListener(WlOnParparedListener wlOnParparedListener)
    {
        this.wlOnParparedListener = wlOnParparedListener;
    }

    public void setWlOnLoadListener(WlOnLoadListener wlOnLoadListener) {
        this.wlOnLoadListener = wlOnLoadListener;
    }

    public void setWlOnPauseResumeListener(WlOnPauseResumeListener wlOnPauseResumeListener) {
        this.wlOnPauseResumeListener = wlOnPauseResumeListener;
    }

    public void setWlOnTimeInfoListener(WlOnTimeInfoListener wlOnTimeInfoListener) {
        this.wlOnTimeInfoListener = wlOnTimeInfoListener;
    }

    public void setWlOnErrorListener(WlOnErrorListener wlOnErrorListener) {
        this.wlOnErrorListener = wlOnErrorListener;
    }

    public void setWlOnCompleteListener(WlOnCompleteListener wlOnCompleteListener) {
        this.wlOnCompleteListener = wlOnCompleteListener;
    }

    public void parpared()
    {
        if(TextUtils.isEmpty(source))
        {
            MyLog.d("source not be empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_parpared(source);
            }
        }).start();

    }

    public void start()
    {
        if(TextUtils.isEmpty(source))
        {
            MyLog.d("source is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
    }
    public void stop()
    {
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        }).start();
    }

    public void pause()
    {
        n_pause();
        if(wlOnPauseResumeListener != null)
        {
            wlOnPauseResumeListener.onPause(true);
        }
    }


    public void resume()
    {
        n_resume();
        if(wlOnPauseResumeListener != null)
        {
            wlOnPauseResumeListener.onPause(false);
        }
    }

    public void seek(int secds)
    {
        n_seek(secds);
    }

    public void playNext(String url)
    {
        source = url;
        playNext = true;
        stop();
    }


    /**
     * c++回调java的方法
     */
    public void onCallParpared()
    {
        if(wlOnParparedListener != null)
        {
            wlOnParparedListener.onParpared();
        }
    }

    public native void n_parpared(String source);
    public native void n_start();
    private native void n_pause();
    private native void n_resume();
    private native void n_stop();
    private native void n_seek(int secds);

    public WlOnLoadListener getWlOnLoadListener() {
        return wlOnLoadListener;
    }

    public void onCallLoad(boolean bool){
        if(wlOnLoadListener!=null){
            wlOnLoadListener.onLoad(bool);
        }
    }

    public void onCallTimeInfo(int currentTime, int totalTime)
    {
        if(wlOnTimeInfoListener != null)
        {
            if(wlTimeInfoBean == null)
            {
                wlTimeInfoBean = new WlTimeInfoBean();
            }
            wlTimeInfoBean.setCurrentTime(currentTime);
            wlTimeInfoBean.setTotalTime(totalTime);
            wlOnTimeInfoListener.onTimeInfo(wlTimeInfoBean);
        }
    }

    public void onCallError(int code, String msg)
    {
        if(wlOnErrorListener != null)
        {
            stop();
            wlOnErrorListener.onError(code, msg);
        }
    }


    public void onCallComplete()
    {
        if(wlOnCompleteListener != null)
        {
            stop();
            wlOnCompleteListener.onComplete();
        }
    }

    public void onCallNext()
    {
        if(playNext)
        {
            playNext = false;
            parpared();
        }
    }

}
