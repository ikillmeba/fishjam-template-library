package com.fishjam.storehelper;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Matrix;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.ImageView;

//http://blog.csdn.net/pathuang68/article/details/6992085 -- 手势后通过Matrix缩放、移动图片(为何不用 ScaleGestureDetector ? 版本问题？ )


class TestGestureIconInfo extends StartIconInfo{

	TestGestureIconInfo(Activity activity) {
		super(activity);
		this.name = "TestGesture";
		this.icon = R.drawable.bus_service;
	}

	@Override
	void onClick() {
		Intent intent = new Intent(mActivity, TestGestureActivity.class);
		mActivity.startActivity(intent);
	}
}

public class TestGestureActivity extends Activity implements OnTouchListener{
	final static String TAG = TestGestureActivity.class.getSimpleName();
	
	ImageView mImageView;
	ScaleGestureDetector mDetector;
	Matrix mMatrix;
	
	float mFactor = 1.0f;
	
	class MyScaleGestureListener  extends ScaleGestureDetector.SimpleOnScaleGestureListener{

		@Override
		public boolean onScale(ScaleGestureDetector detector) {
			boolean bRet = super.onScale(detector);
			
			Log.i(TAG, "onScale return " + bRet + ", factor=" + detector.getScaleFactor());
			//mFactor = detector.getScaleFactor();
			//mMatrix.setScale(mFactor, mFactor);
			//mImageView.invalidate();
			return false;
		}

		@Override
		public boolean onScaleBegin(ScaleGestureDetector detector) {
			boolean bRet = super.onScaleBegin(detector);
			Log.i(TAG, "onScaleBegin return " + bRet + ", span=" + detector.getCurrentSpan());
			//TestGestureActivity.this.mMatrix.postScale(sx, sy)
			return true;
		}

		@Override
		public void onScaleEnd(ScaleGestureDetector detector) {
			Log.i(TAG, "onScaleEnd , factor=" + detector.getScaleFactor());
			super.onScaleEnd(detector);
		}
	}
	
	MyScaleGestureListener mGestureListener = new MyScaleGestureListener();
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.test_gesture);
		mDetector = new ScaleGestureDetector(this, mGestureListener);
		mImageView = (ImageView)findViewById(R.id.imageViewTest);
		mImageView.setOnTouchListener(this);
		mMatrix = new Matrix(); 
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		boolean bRetDetector = mDetector.onTouchEvent(event);
		boolean bDefaultToucheEvent = super.onTouchEvent(event);
		Log.i(TAG, "onTouchEvent " + event.getAction() +"Pos=" + event.getX() + "x" + event.getY() 
				+" ,Detector=" + bRetDetector + ", Defautl=" + bDefaultToucheEvent);
		return true;
	}

	//@Override
	public boolean onTouch(View v, MotionEvent event) {
		
		boolean bRetDetector = mDetector.onTouchEvent(event);

		//boolean bDefaultToucheEvent = super.onTouchEvent(event);
		if (event.getAction() != MotionEvent.ACTION_MOVE) {
			Log.i(TAG, "onTouch, view=" + v.toString() +",Action=" +event.getAction() +",Pos=" + event.getX() + "x" + event.getY() 
					+" ,Detector=" + bRetDetector
					//+ ", Defautl=" + bDefaultToucheEvent
					);
		}
		return bRetDetector;
	}
	
}
