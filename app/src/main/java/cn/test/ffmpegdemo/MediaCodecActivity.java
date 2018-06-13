package cn.test.ffmpegdemo;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaExtractor;
import android.media.MediaMetadataRetriever;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.github.hiteshsondhi88.libffmpeg.ExecuteBinaryResponseHandler;
import com.github.hiteshsondhi88.libffmpeg.FFmpeg;
import com.github.hiteshsondhi88.libffmpeg.LoadBinaryResponseHandler;
import com.github.hiteshsondhi88.libffmpeg.exceptions.FFmpegCommandAlreadyRunningException;
import com.github.hiteshsondhi88.libffmpeg.exceptions.FFmpegNotSupportedException;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Arrays;

/**
 * @author lql E-mail: 595308107@qq.com
 * @version 0 创建时间：2018/4/23 15:22
 * 类说明
 */
public class MediaCodecActivity extends AppCompatActivity {
    SurfaceView sv;
    private MediaExtractor mExtractor;
    private int mVideoDecoderTrack;
    private long mVideoStopTimeStamp;
    private boolean isVideoExtractorEnd=false;
    private long nowTimeStamp;
    private FFmpeg ffmpeg;

    private final static String TAG=MediaCodecActivity.class.getSimpleName();
    private VideoSurfaceView videoSurfaceView;


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.mediacodec_activity);
//        ImageReader imageReader=ImageReader.newInstance(0,0,0,0);
//        imageReader.getSurface();
//        imageReader.setOnImageAvailableListener(new ImageReader.OnImageAvailableListener() {
//            @Override
//            public void onImageAvailable(ImageReader reader) {
//                Image image = reader.acquireNextImage();
//                image.getPlanes();
//            }
//        });
        ffmpeg = FFmpeg.getInstance(getApplicationContext());
        loadFFMpegBinary();
        sv=findViewById(R.id.sv);
        videoSurfaceView = new VideoSurfaceView(MediaCodecActivity.this);
        sv.getHolder().addCallback(new SurfaceHolder.Callback2() {
            @Override
            public void surfaceRedrawNeeded(SurfaceHolder holder) {

            }

            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                    System.out.println("surfaceCreated");
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                System.out.println("surfaceChanged");
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                System.out.println("surfaceDestroyed");
            }
        });

    }

    private void loadFFMpegBinary() {
        try {
            ffmpeg.loadBinary(new LoadBinaryResponseHandler() {
                @Override
                public void onFailure() {
                    showUnsupportedExceptionDialog();
                }
            });
        } catch (FFmpegNotSupportedException e) {
            showUnsupportedExceptionDialog();
        }
    }
    private void execFFmpegBinary(final String[] command, final String file) {
        try {
            ffmpeg.execute(command, new ExecuteBinaryResponseHandler() {
                @Override
                public void onFailure(String s) {
                   System.err.println("FAILED with output : "+s);
                }

                @Override
                public void onSuccess(String s) {
                    System.out.println("SUCCESS with output : "+s);
                }

                @Override
                public void onProgress(String s) {
                    Log.d(TAG, "Started command : ffmpeg "+command);
                    System.out.println("progress : "+s);
                }

                @Override
                public void onStart() {


                    Log.d(TAG, "Started command : ffmpeg " + command);
                    System.out.println("Processing...");
                }

                @Override
                public void onFinish() {
                    Log.d(TAG, "Finished command : ffmpeg "+command);
                    startPlay(file);

                }
            });
        } catch (FFmpegCommandAlreadyRunningException e) {
            // do nothing for now
        }
    }
    private void showUnsupportedExceptionDialog() {
        Log.e("TAG","不支持转码");
//        new AlertDialog.Builder(this)
//                .setIcon(android.R.drawable.ic_dialog_alert)
//                .setTitle(getString(R.string.device_not_supported))
//                .setMessage(getString(R.string.device_not_supported_message))
//                .setCancelable(false)
//                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
//                    @Override
//                    public void onClick(DialogInterface dialog, int which) {
//                        Home.this.finish();
//                    }
//                })
//                .create()
//                .show();

    }

   boolean isRenderToWindowSurface=false;
    private static final int TIME_OUT=1000;
    private MediaCodec.BufferInfo videoDecodeBufferInfo=new MediaCodec.BufferInfo();


    public boolean tryTrancodeIfNeed(String path){
        boolean ret=true;
        MediaMetadataRetriever mMetRet = new MediaMetadataRetriever();

        if (path == null) {
            throw new IllegalArgumentException();
        }

        FileInputStream is = null;
        try {
            is = new FileInputStream(path);
            FileDescriptor fd = is.getFD();
            mMetRet.setDataSource(fd, 0, 0x7ffffffffffffffL);
        } catch (FileNotFoundException fileEx) {
            throw new IllegalArgumentException();
        } catch (IOException ioEx) {
            throw new IllegalArgumentException();
        }catch (RuntimeException e){
            e.printStackTrace();
            int lastIndexOf = path.lastIndexOf('/');
            String outpath=path.substring(0,lastIndexOf+1)+path.substring(lastIndexOf+1)+".mp4";
            File file=new File(outpath);
            if(file.exists())
            {
                System.out.println("文件已存在删除:"+file.toString());
                boolean retdel = file.delete();
                System.out.println(retdel+"");
            }
//            execFFmpegBinary(new String[]{"-i",path,outpath},outpath);
            ret= false;
        }

        try {
            if (is != null) {
                is.close();
            }
        } catch (Exception e) {}


        return ret;
    }

    public void start(View v) throws IOException {
        String mInputPath = videoPath;
        startPlay(mInputPath);
//        MediaCodec mediaCodec = MediaCodec.createByCodecName("");
//        mediaCodec.configure(MediaFormat.createVideoFormat());

    }

    private void startPlay(final String mInputPath) {
        new Thread() {
            @Override
            public void run() {
                super.run();
                long startTime = System.currentTimeMillis();

                    try {
                        ffmplay(mInputPath);
//                    isVideoExtractorEnd=false;
//                    MediaMetadataRetriever mMetRet = new MediaMetadataRetriever();
//
//                    if(!tryTrancodeIfNeed(mInputPath)){
//                        System.err.println("无法播放....");
//
//                        ffmplay(mInputPath);
//                        return;
//                    }
//                    mMetRet.setDataSource(mInputPath);
//                    long mVideoTotalTime=Long.valueOf(mMetRet.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION));
//                    String bitrate = mMetRet.extractMetadata(MediaMetadataRetriever.METADATA_KEY_BITRATE);
//                    String framerate = mMetRet.extractMetadata(MediaMetadataRetriever.METADATA_KEY_CAPTURE_FRAMERATE);
//                    System.out.println("video total time:"+mVideoTotalTime+", bitrate:"+bitrate+","+"framerate:"+framerate);
//                    mExtractor = new MediaExtractor();
//                    mExtractor.setDataSource(mInputPath);
//                    int sampleTrackIndex = mExtractor.getSampleTrackIndex();
//                    System.out.println("sampleTrackIndex:" + sampleTrackIndex
//                    );
//                    int count = mExtractor.getTrackCount();
//                    mVideoDecoderTrack = -1;
//                    MediaCodec codec;
//                    MediaFormat format = null;
//                    String mime = null;
//                    for (int i = 0; i < count; i++) {
//                        format = mExtractor.getTrackFormat(i);
//                        mime = format.getString(MediaFormat.KEY_MIME);
//                        if (mime.startsWith("video")) {
//                            mVideoDecoderTrack = i;
//                            break;
//                        }
//                    }
//
//                    if (format != null) {
//                        System.out.println("Input MIME:" + format.getString(MediaFormat.KEY_MIME));
//                        System.out.printf("Height:%d,Width:%d\n",format.getInteger(MediaFormat.KEY_HEIGHT),
//                                format.getInteger(MediaFormat.KEY_WIDTH));
////                mInputVideoWidth = format.getInteger(MediaFormat.KEY_WIDTH);
////                mInputVideoHeight = format.getInteger(MediaFormat.KEY_HEIGHT);
//                        try {
//                            codec = MediaCodec.createDecoderByType(mime);
//                        } catch (IOException e) {
//                            e.printStackTrace();
//                            ffmplay(mInputPath);
//                            return;
//                        }
//
////                        MediaFormat mediaFormat = MediaFormat.createVideoFormat(mime, info.width,info.height);
///**指定视频帧格式*/
////                        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT,MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);//指定帧格式
/////***/
////                        codec.configure(mediaFormat, null, null, 0);
//
//                        int[] colorFormats = codec.getCodecInfo().getCapabilitiesForType(mime).colorFormats;
//                        System.out.println("support colorFormats:"+ Arrays.toString(colorFormats));
//                        //将SurfaceTexture作为参数创建一个Surface，用来接收解码视频流
//                        codec.configure(format, sv.getHolder().getSurface(), null, 0);
//                        MediaFormat outputFormat = null;// option B
//                        codec.start();
//                        System.out.println("开始硬件解码");
//                        while (!isVideoExtractorEnd) {
//                            int inputBufferId = codec.dequeueInputBuffer(TIME_OUT);
//                            if (inputBufferId >= 0) {
//                                ByteBuffer inputBuffer = codec.getInputBuffers()[inputBufferId];
//                                inputBuffer.clear();
//                                mExtractor.selectTrack(mVideoDecoderTrack);
//                                int ret = mExtractor.readSampleData(inputBuffer, 0);
//                                if (ret != -1) {
//                                    mVideoStopTimeStamp = mExtractor.getSampleTime();
//                                    codec.queueInputBuffer(inputBufferId, 0, ret, mVideoStopTimeStamp, mExtractor.getSampleFlags());
//                                    isVideoExtractorEnd = false;
//                                } else {
//                                    //可以用!mExtractor.advance，但是貌似会延迟一帧。readSampleData 返回 -1 也表示没有更多数据了
//                                    isVideoExtractorEnd = true;
//                                }
//                                mExtractor.advance();
//                            }
//                            int outputBufferId = codec.dequeueOutputBuffer(videoDecodeBufferInfo, TIME_OUT);
//                            if (outputBufferId >= 0) {
//                                ByteBuffer outputBuffer = codec.getOutputBuffers()[outputBufferId];
//                                MediaFormat bufferFormat = codec.getOutputFormat();// option A
//                                // bufferFormat is identical to outputFormat
//                                // outputBuffer is ready to be processed or rendered.
//                                nowTimeStamp = videoDecodeBufferInfo.presentationTimeUs;
//                                codec.releaseOutputBuffer(outputBufferId, true);
//                            } else if (outputBufferId == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
//                                // Subsequent data will conform to new format.
//                                // Can ignore if using getOutputFormat(outputBufferId)
//                                outputFormat = codec.getOutputFormat(); // option B
//                                System.out.println("out mime:" + outputFormat.getString(MediaFormat.KEY_MIME));
//                                System.out.println("OUT KEY_COLOR_FORMAT:" + outputFormat.getInteger(MediaFormat.KEY_COLOR_FORMAT));
//                            }
//                        }
//                        mExtractor.release();
//                        mMetRet.release();
//                        codec.stop();
//                        codec.release();
                        long end = System.currentTimeMillis();
                        System.out.printf("total cost %.2f", (end - startTime) / 1000.0);
                        System.out.println("============Finish===============");
//                    }


                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }.start();
    }

    String videoPath = Environment.getExternalStorageDirectory()
            +"/VisualArts/materials/f64e4f069e4dd171962da282f5de60ae.mp4";

    private void ffmplay(String mInputPath) {
        System.out.println("软解...");
        long startTime;
        startTime = System.currentTimeMillis();
        videoSurfaceView.videoPlay(mInputPath,sv.getHolder().getSurface());
//                        VideoCodec videoCodec=new VideoCodec(mInputPath);
//                        videoCodec.open();
//                        videoCodec.init(sv.getWidth(),sv.getHeight());
//                        startTime = System.currentTimeMillis();
//                        videoCodec.play(sv.getHolder().getSurface());
        System.out.printf("cost %.2fs\n",(System.currentTimeMillis()-startTime)/1000.0f);
//                        videoCodec.close();
        //33.50s
        //5.40s
        return;
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
