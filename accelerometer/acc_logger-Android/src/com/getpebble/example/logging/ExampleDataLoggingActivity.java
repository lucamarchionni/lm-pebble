package com.getpebble.example.logging;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.widget.TextView;
import com.getpebble.android.kit.PebbleKit;
import com.google.common.base.Objects.ToStringHelper;
import com.google.common.primitives.UnsignedInteger;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;
import java.util.UUID;

import java.io.File;
//import java.io.FileOutputStream;
import java.io.FileWriter;

import android.os.Environment;
import android.util.Log;

/**
 * Sample code demonstrating how Android applications can receive data logs from Pebble.
 */
public class ExampleDataLoggingActivity extends Activity {
    private static final UUID ACC_LOG_APP_UUID = UUID.fromString("666fd1a5-0e9a-46a4-bf2f-255a99961f05");
    private static final DateFormat DATE_FORMAT = new SimpleDateFormat("HH:mm:ss");

    private final StringBuilder mDisplayTextX = new StringBuilder();
    private final StringBuilder mDisplayTextY = new StringBuilder();
    private final StringBuilder mDisplayTextZ = new StringBuilder();
    
    private static FileWriter mFile;
    private PebbleKit.PebbleDataLogReceiver mDataLogReceiver = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_counter);
        DATE_FORMAT.setTimeZone(TimeZone.getDefault());
        File f = new File(Environment.getExternalStorageDirectory(),"acceleration");
        try{
        mFile = new FileWriter(f);
        }catch(Exception e) {
            Log.d("Logger", e.getMessage());
        }
    }
    
    @Override
    protected void onDestroy()
    {
    	try{
    		mFile.close();
    	}catch(Exception e) {
    		Log.d("Logger", e.getMessage());
    	}
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mDataLogReceiver != null) {
            unregisterReceiver(mDataLogReceiver);
            mDataLogReceiver = null;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        final Handler handler = new Handler();

        // To receive data logs, Android applications must register a "DataLogReceiver" to receive data.
        //
        // In this example, we're implementing a handler to receive unsigned integer data that was logged by a
        // corresponding watch-app. In the watch-app, three separate logs were created, one per animal. Each log was
        // tagged with a key indicating the animal to which the data corresponds. So, the tag will be used here to
        // look up the animal name when data is received.
        //
        // The data being received contains the seconds since the epoch (a timestamp) of when an ocean faring animal
        // was sighted. The "timestamp" indicates when the log was first created, and will not be used in this example.
        mDataLogReceiver = new PebbleKit.PebbleDataLogReceiver(ACC_LOG_APP_UUID) {
            @Override
            public void receiveData(Context context, UUID logUuid, UnsignedInteger timestamp, UnsignedInteger tag, int data) {
                
                //mDisplayText.append(getUintAsTimestamp(timestamp));
            	try{
          
                if(tag.intValue() == 0x5 )
                {
                	mDisplayTextX.append("\n");
                	mDisplayTextX.append(timestamp.toString());
                	mDisplayTextX.append(": x = ");
                    mDisplayTextX.append( Integer.valueOf(data).toString());
                    //mFile.write(String.valueOf(data));

                }
                else if(tag.intValue() == 0xd )
                {
                	mDisplayTextY.append("\n");
                	mDisplayTextY.append(timestamp.toString());
                	mDisplayTextY.append(": y = ");
                    mDisplayTextY.append( Integer.valueOf(data).toString());
                }
                else if(tag.intValue() == 0xb )
                {
                	mDisplayTextZ.append("\n");
                	mDisplayTextZ.append(timestamp.toString());
                	mDisplayTextZ.append(": z = ");
                    mDisplayTextZ.append( Integer.valueOf(data).toString());
                }                

            	}catch(Exception e)
            	{
            		 Log.d("Logger", e.getMessage());
            	}
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        updateUi();
                    }
                });
            }
        };

        PebbleKit.registerDataLogReceiver(this, mDataLogReceiver);

        PebbleKit.requestDataLogsForApp(this, ACC_LOG_APP_UUID);
    }

    private void updateUi() {
        TextView textView = (TextView) findViewById(R.id.log_data_text_view);
        textView.setText(mDisplayTextX.toString() + mDisplayTextY.toString() + mDisplayTextZ.toString());
        
    }

    private String getUintAsTimestamp(UnsignedInteger uint) {
        return DATE_FORMAT.format(new Date(uint.longValue() * 1000L)).toString();
    }
}
