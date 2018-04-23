package cn.test.ffmpegdemo;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private Button protocol,format,codec,filter,btn_play,btn_getfirstframe;
    private TextView tv_info;
    private Button btn_surface;
    private Button btn_texture;
    private Button btn_media_codec;

    private Button btnGl;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        init();
//        Bitmap.createBitmap(22,33, Bitmap.Config.ARGB_8888)
    }
    private void init() {
        protocol = (Button) findViewById(R.id.btn_protocol);
        format = (Button) findViewById(R.id.btn_format);
        codec = (Button) findViewById(R.id.btn_codec);
        filter = (Button) findViewById(R.id.btn_filter);
        tv_info = (TextView) findViewById(R.id.tv_info);
        btn_play=findViewById(R.id.btn_play);
        btn_getfirstframe=findViewById(R.id.btn_getfirstframe);
        btnGl=findViewById(R.id.btn_gl);
        btn_media_codec=findViewById(R.id.btn_media_codec);

        btn_surface=findViewById(R.id.btn_surface);
        btn_texture=findViewById(R.id.btn_texture);
        protocol.setOnClickListener(this);
        format.setOnClickListener(this);
        codec.setOnClickListener(this);
        filter.setOnClickListener(this);
        btn_play.setOnClickListener(this);
        btn_getfirstframe.setOnClickListener(this);
        btn_surface.setOnClickListener(this);
        btn_texture.setOnClickListener(this);
        btnGl.setOnClickListener(this);
        btn_media_codec.setOnClickListener(this);
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();


    public native String urlprotocolinfo();
    public native String avformatinfo();
    public native String avcodecinfo();
    public native String avfilterinfo();

    public native void getfirstframe();

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.btn_protocol:
                tv_info.setText(urlprotocolinfo());
                break;
            case R.id.btn_format:
                tv_info.setText(avformatinfo());
                break;
            case R.id.btn_codec:
                tv_info.setText(avcodecinfo());
                break;
            case R.id.btn_filter:
                tv_info.setText(avfilterinfo());
                break;
            case R.id.btn_play:
                startActivity(new Intent(this,PlayActivity.class));
                break;
            case R.id.btn_getfirstframe:
                getfirstframe();
                break;
            case R.id.btn_texture:
                startActivity(new Intent(this,TextureActivity.class));
                break;
            case R.id.btn_surface:
                startActivity(new Intent(this, GLSurfaceActivity.class));
                break;
            case R.id.btn_gl:
                startActivity(new Intent(this,GlPlayActivity.class));
                break;
            case R.id.btn_media_codec:
                startActivity(new Intent(this,MediaCodecActivity.class));
                break;
            default:
                break;
        }
    }
}
