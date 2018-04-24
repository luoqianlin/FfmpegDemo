package cn.test.ffmpegdemo;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

import java.io.IOException;
import java.util.Arrays;

/**
 * @author lql E-mail: 595308107@qq.com
 * @version 0 创建时间：2018/4/23 15:22
 * 类说明
 */
public class MediaCodecActivity extends AppCompatActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.mediacodec_activity);

    }

    public void start(View v) throws IOException {
        MediaCodec mediaCodec = MediaCodec.createByCodecName("");
//        mediaCodec.configure(MediaFormat.createVideoFormat());
    }

    public void codecInfo(View v){
        int codecCount = MediaCodecList.getCodecCount();
        for(int i=0;i<codecCount;i++){
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
            System.out.println(codecInfo.getName()+"  "
                    + Arrays.toString(codecInfo.getSupportedTypes())
                    + " isEncoder:"+codecInfo.isEncoder());
        }

    }
}
