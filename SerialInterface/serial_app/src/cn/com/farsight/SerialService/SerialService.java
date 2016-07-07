package cn.com.farsight.SerialService;
import java.io.BufferedReader;
import java.io.UnsupportedEncodingException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.util.Log;

public class SerialService {
	private static final String TAG = "SerialService";
	// 底层初始化状态
	private boolean isInitOk = false;

	// 加载本地库，read()和write()的实现通过JNI在本地代码实现
	static {
		Log.d(TAG, "System.loadLibrary()");
		System.loadLibrary("serial_runtime");
	}   
	
	// 构造器，用于初始化本地HAL及runtime
	public SerialService(){	  
		Log.d(TAG, "SerialService()");
		// 初始化本地HAL
		isInitOk =  _init();
		Log.d(TAG, "_init()");
	}

	// 读串口数据
	public String read() {
		Log.d(TAG, "read()");
		if(!isInitOk)
			return "串口初始化失败，请确认已经连接串口设备。";
		// 由于 Java String是双字节字符，而串口数据是原始字节流，所以要进行转化
		byte[] data = new byte[128];
		// 从驱动读取字节流
		_read(data, 128);
		 
		String ret;
		try{
			// 转化为Java String字符
			ret = new String(data, "GBK");
		}catch(UnsupportedEncodingException e1) {
			return null;
		}
		return ret;
	}

	// 写入字符串数据
	public int  write(String s) {
		Log.d(TAG, "write()");
		int len;
		try{
			// 将Java String字符串转码成字节流后，写入串口
			len = _write(s.getBytes("GBK"));
		}catch(UnsupportedEncodingException e1) {
			return -1;
		}
		return len;
	}

	private static native boolean _init();
	private static native int _read(byte[] data, int len);
	private static native int _write(byte[] data);

}
