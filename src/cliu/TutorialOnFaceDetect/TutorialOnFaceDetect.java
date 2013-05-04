package cliu.TutorialOnFaceDetect;

/*
 * EyeLine Detect from input file
 * 
 * [AUTHOR]: YoYo
 * [SDK   ]: Android SDK
 * [NOTE  ]: 
 * [LAST MODIFIED]:20130502
 */
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PointF;
import android.media.FaceDetector;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.text.format.Time;
import android.util.Log;
import android.widget.ImageView;
import android.widget.Toast;
import android.widget.LinearLayout.LayoutParams;

public class TutorialOnFaceDetect extends Activity {
	private MyImageView mIV;
	private Bitmap mFaceBitmap;
	public Bitmap draw;
	private int mFaceWidth = 200;
	private int mFaceHeight = 200;
	private static final int MAX_FACES = 1;
	private static String TAG = "TutorialOnFaceDetect";
	private static boolean DEBUG = false;
	public int[] pixels;
	public int[] lResult;
	public int[] rResult;
	public int placelength=0;
	public int[] im3,iml3;
	public int width,height;
	public double startTime,endTime,totTime;
	public Canvas canvasTemp;
	public Paint p;
	SimpleDateFormat sDateFormat = new
	SimpleDateFormat("yyyyMMdd_hhmmss");
	String date = sDateFormat.format(new java.util.Date());
	//Image saving path
    //String strCapatureFilePath = getSDPath()+"/camera_snap.png"; 	//For other machine
	private String SDcardPath = "/mnt/sdcard/Eyeline/";
	private String strFilePath = SDcardPath+"Result/camera_result_"+date+".jpg";
	private String strcompressFilePath = SDcardPath+"CompressedInput/camera_compress_"+date+".jpg";
    private String strCapatureFilePath = SDcardPath+"Input/wtrite.png";		//For TFE03
    private boolean exist=true;
	FileOutputStream fos;
    static
    {
    	System.loadLibrary("main");//Load the C code
    }
	protected static final int GUIUPDATE_SETFACE = 999;

	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);     
		setContentView(R.layout.main);
		getSDPath();
		if(!checkSDCard())
		{
			Log.e(TAG,"error");
			mMakeTextToast(getResources().getText(R.string.str_err_nosd).toString(),true);
		}
		
		//	Check the SD card is available
		if (Environment.getExternalStorageState()
				.equals(Environment.MEDIA_MOUNTED))
		{
			File sdFile = android.os.Environment.getExternalStorageDirectory();
			
			String folder = "/mnt/sdcard/Eyeline/";
			File dirFileRoot = new File(folder);

			if(!dirFileRoot.exists()){//如果資料夾不存在
				dirFileRoot.mkdir();//建立資料夾
				mMakeTextToast("No Input image file",true);
				exist = false;
			}	
			String folderRes = SDcardPath + File.separator + "Result";
			File dirFile = new File(folderRes);

			if(!dirFile.exists()){//如果資料夾不存在
				dirFile.mkdir();//建立資料夾
			}
			String folderComIn = SDcardPath + File.separator + "CompressedInput";
			File dirFile2 = new File(folderComIn);

			if(!dirFile2.exists()){//如果資料夾不存在
				dirFile2.mkdir();//建立資料夾
			}
			String folderIn = SDcardPath + File.separator + "Input";
			File dirFile3 = new File(folderIn);

			if(!dirFile3.exists()){//如果資料夾不存在
				dirFile3.mkdir();//建立資料夾
			}
		
			
		}
		
		//	Determine the newest photo in the input folder.
        String maxName="";
        long maxDate = 0;		
        File[] files = new File("//mnt//sdcard//Eyeline//Input//").listFiles();
        if(files.length==0)
        {
        	exist = false;
        	mMakeTextToast("No Input image file",true);
        	Log.e(TAG, "NO image");
        }
        for(File f:files){
        	if(f.getName().endsWith(".png") || f.getName().endsWith(".jpg"))
        	{
        		if(f.lastModified()> maxDate)
        		{
        			maxName = f.getName();
        			maxDate = f.lastModified();   
        		}
        	}
        }
        
        strCapatureFilePath = SDcardPath+"Input/"+maxName;	//Put the newest photo name into String

        //Log.e(TAG,"detect path"+ strCapature1FilePath);
        
        
		//	Load the photo
        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inPreferredConfig = Bitmap.Config.ARGB_8888;	//	Preload to preventv it out of memory.
        opt.inJustDecodeBounds = true;
        
    	//Log.e(TAG,"sd path "+strCapatureFilePath);
    	if(exist)
    		BitmapFactory.decodeFile(strCapatureFilePath, opt);
    	else
    		BitmapFactory.decodeResource(getResources(), R.drawable.face8,opt);
    	opt.inSampleSize = computeSampleSize(opt, -1, 1600*1200);	//	If it out of memory.Resize it under 1600*1200.
        opt.inJustDecodeBounds = false;
        try {
        	Bitmap b;
        	if(exist)
        		b = BitmapFactory.decodeFile(strCapatureFilePath, opt);
        	else
        		b = BitmapFactory.decodeResource(getResources(), R.drawable.face8,opt);
    		Log.e(TAG,"still work!");
            //	set the picture size into a ratio of 4:3.
            int w = b.getWidth();
            int h = b.getHeight();
    		Log.e(TAG,"After compressed: the w = "+w+" h= "+h);
            int destWidth;
            int destHeigth;
            if(w>h)
            {
            	destWidth = w;
                destHeigth = destWidth*3/4;
            }else
            {
            	destWidth = h*3/4;
                destHeigth = h;       	
            }
            float scaleWidth = ((float) destWidth) / w;
            float scaleHeight = ((float) destHeigth) / h;
            Matrix mtx = new Matrix();
            mtx.postScale(scaleWidth, scaleHeight);
            Bitmap resizedBMP = Bitmap.createBitmap(b, 0, 0, w, h, mtx, true);
    		Log.e(TAG,"still work2");
            draw=resizedBMP.copy(Bitmap.Config.ARGB_8888, true);
    		try {
    			//	save the picture of Compressed picture
    			fos = new FileOutputStream(strcompressFilePath);
    	    	draw.compress( Bitmap.CompressFormat.JPEG, 100, fos );
    	    	fos.close();
    		} catch (FileNotFoundException e) {
    			// TODO Auto-generated catch block
    			e.printStackTrace();
    		} catch (IOException e) {
    			// TODO Auto-generated catch block
    			e.printStackTrace();
    		}     
            mFaceBitmap = resizedBMP.copy(Bitmap.Config.RGB_565, true);
    		b.recycle();             
            ImageView iview = (ImageView)findViewById(R.id.imageView1); 
            iview.setImageBitmap(draw);

    		mFaceWidth = mFaceBitmap.getWidth();
    		mFaceHeight = mFaceBitmap.getHeight();  
    		pixels = new int[mFaceWidth * mFaceHeight];
    		draw.getPixels(pixels, 0, mFaceWidth, 0, 0, mFaceWidth, mFaceHeight);
    		setFace();
            mFaceBitmap.recycle();
            iview.setImageBitmap(draw);
            Log.e(TAG,"doneiview?");
            } catch (OutOfMemoryError err) {
            	err.printStackTrace();
         }

    }
    //	Face detection and eyeline detection.
    public void setFace() {
		startTime = System.currentTimeMillis();
		canvasTemp =  new  Canvas(draw);
		p = new Paint();
		p.setColor(Color.YELLOW);
		p.setStrokeWidth(3);
    	FaceDetector fd;
		double findFaceStart = System.currentTimeMillis();  
    	FaceDetector.Face [] faces = new FaceDetector.Face[MAX_FACES];
		double findFaceEnd = System.currentTimeMillis();
		Log.e(TAG,"find dace time="+(findFaceEnd -findFaceStart )/1000+"sec");    	
    	PointF eyescenter = new PointF();
    	float eyesdist = 0.0f;
    	int [] fpx = null;
    	int [] fpy = null;
    	int count = 1;
    	//canvasTemp.drawBitmap( draw, 0, 0, null );
    	try {
    		findFaceStart = System.currentTimeMillis();  
    		fd = new FaceDetector(mFaceWidth, mFaceHeight, MAX_FACES);  
    		findFaceEnd = System.currentTimeMillis();
    		Log.e(TAG,"find dace2 time="+(findFaceEnd -findFaceStart )/1000+"sec");  
    		findFaceStart = System.currentTimeMillis();
    		count = fd.findFaces(mFaceBitmap, faces);
    		findFaceEnd = System.currentTimeMillis();
    		Log.e(TAG,"find dace3 time="+(findFaceEnd -findFaceStart )/1000+"sec");  
    	} catch (Exception e) {
    		Log.e(TAG, "setFace(): " + e.toString());
    		return;
    	}

    	// check if we detect any faces
    	if (count > 0) {
    		fpx = new int[count * 2];
    		fpy = new int[count * 2];
    		int i = 0;
    			try {                 
    				faces[i].getMidPoint(eyescenter);                  
    				eyesdist = faces[i].eyesDistance();  
    				p.setStyle(Paint.Style.STROKE);//set the rectangle in stroke  
    		    	canvasTemp.drawRect( // draw the face detected label.
                            //(int)(eyescenter.x - eyesdist*0.9),
    		    			(int)(eyescenter.x - eyesdist*1.1),
                            //(int)(eyescenter.y - eyesdist*0.6),
    		    			(int)(eyescenter.y - eyesdist*0.6),
    		    			(int)(eyescenter.x + eyesdist*1.1),
                            (int)(eyescenter.y + eyesdist*1.5),
                            p);
    				// set up left eye location
    				fpx[2 * i] = (int)(eyescenter.x - eyesdist / 2);
    				fpy[2 * i] = (int)eyescenter.y;
    				int rightxside = (int)(eyescenter.x - eyesdist / 2);
    				int rightyside = (int)eyescenter.y;
    				width = (int)(eyesdist*0.9);
    				height = (int)eyesdist*3/7;
    				int xinit = (int)(rightxside-eyesdist/2);
    				int yinit = (int)(rightyside-eyesdist/5);    				
    				im3 = new int[width*height];
    				//rResult = new int[width*height];
    				int in =0;
    				Log.e(TAG, "x=" +xinit);
    				Log.e(TAG, "y=" +yinit);
    				Log.e(TAG, "width=" +width);
    				Log.e(TAG, "height=" +height);		
    				canvasTemp.drawRect(xinit,yinit,xinit+width,yinit+height, p);
    				p.setColor(Color.GREEN);
    				double passpixelstart = System.currentTimeMillis();
    				
    				for(int y = yinit; y < (yinit+height); y++)
    				{	
    					for (int x = xinit; x < (xinit+width); x++)    				
    					{
    						
    						int index = y * mFaceWidth + x;
    						int R = (pixels[index] >> 16) & 0xff;
    		        		int G = (pixels[index] >> 8) & 0xff;
    		        		int B = pixels[index] & 0xff;  
    		        		im3[in++]=R+G+B;
    		        		//if(in<100)
    		        		//	Log.e(TAG,"im3 = "+"(x,y)="+x+" "+y+"im"+im3[in-1]+" R="+R+" G="+G+" B="+B);
    		        		//Log.e(TAG, "im3 " +"x,y= "+x+" "+y+"content"+R+" "+G+" "+B);
    		                  
    					}
    				}
    				double passpixelend = System.currentTimeMillis();
    				//totTime = passpixelstart - passpixelend; 
    				Log.e(TAG,"pixel transfer time="+(passpixelend -passpixelstart )/1000+"sec");     				
    				rResult = new int[width*height];
    				rResult = sendToC(height,width,im3);

    				int co =0;
    				/*for(int k=0;k<rResult.length;k++)
    					if(rResult[k]==1)
    					{
    						int index = (int)(rightxside-eyesdist/3)+
    						pixels[]
    					}*/
    				double drawEyelineStart = System.currentTimeMillis();
    				for(int y = yinit; y < (yinit+height); y++)
    				{	
    					for (int x = xinit; x < (xinit+width); x++)    				
    					{
    						if(rResult[co]==1)
    						{
        	    				canvasTemp.drawPoint(x, y, p);
    							//canvasTemp.drawPoint(y,x, p);
    							//int index = y * mFaceWidth + x;
    							//int R = 255;
    							//int G = 0;
    							//int B = 0;
    							//pixels[index] = 0xff000000 | (R << 16) | (G << 8) | B;
    						}
    						co++;
    					}
    				}
    				double drawEyelineEnd = System.currentTimeMillis();
    				Log.e(TAG,"draw eyeline time="+(drawEyelineEnd - drawEyelineStart)/1000+"sec");    				
    				// set up right eye location
    				fpx[2 * i + 1] = (int)(eyescenter.x + eyesdist / 2);
    				fpy[2 * i + 1] = (int)eyescenter.y;
    				int leftxside = (int)(eyescenter.x + eyesdist / 2);
    				int leftyside = (int)eyescenter.y;
    				width = (int)(eyesdist*0.9);//ori setting 4/5
    				height = (int)eyesdist*3/7;
    				iml3 = new int[width*height];
    				in =0;
    				xinit = (int)(leftxside-eyesdist/3);
    				yinit = (int)(leftyside-eyesdist/5);    				
    				//rResult = new int[width*height];
    				Log.e(TAG, "x=" +xinit);
    				Log.e(TAG, "y=" +yinit);
    				Log.e(TAG, "width=" +width);
    				Log.e(TAG, "height=" +height); 
    				p.setColor(Color.YELLOW);
    				p.setStyle(Paint.Style.STROKE);//set the rectangle in stroke       				
    				canvasTemp.drawRect(xinit,yinit,xinit+width,yinit+height, p);
    				p.setColor(Color.GREEN);
    				for (int y = yinit; y < (yinit+height); y++)
    				{	
    					for(int x = xinit; x < (xinit+width); x++)    				
    					{ 						
    						int index = y * mFaceWidth + x;
    						int R = (pixels[index] >> 16) & 0xff;
    		        		int G = (pixels[index] >> 8) & 0xff;
    		        		int B = pixels[index] & 0xff;
    		        		iml3[in++]=R+G+B;
    		        		//Log.e(TAG, "im3 " +"x,y= "+x+" "+y+"content"+R+" "+G+" "+B);
    		                //Log.e(TAG,"iml3 = "+"(x,y)="+x+" "+y+"im"+iml3[in-1]);    
    					}
    				}
    				lResult = new int[width*height];
    				lResult = sendToC(height,width,iml3);
    				co =0;
    				for (int y = yinit; y < (yinit+height); y++)
    				{	
    					for(int x = xinit; x < (xinit+width); x++)    				
    					{ 	
    						if(lResult[co]==1)
    						{	
        	    				canvasTemp.drawPoint(x, y, p);  
    							//int index = y * mFaceWidth + x;
    							//int R = 255;
    							//int G = 0;
    							//int B = 0;
    							//pixels[index] = 0xff000000 | (R << 16) | (G << 8) | B;
    						}
    						co++;
    					}
    				}    				
    				DEBUG=false;
    				if (DEBUG)
    					Log.e(TAG, "setFace(): face " + i + ": confidence = " + faces[i].confidence() 
    							+ ", eyes distance = " + faces[i].eyesDistance()                             
    							+ ", pose = ("+ faces[i].pose(FaceDetector.Face.EULER_X) + ","                            
    							+ faces[i].pose(FaceDetector.Face.EULER_Y) + ","                            
    							+ faces[i].pose(FaceDetector.Face.EULER_Z) + ")"
    							+ "left eye"+fpx[0]+" "+fpy[0]+"right eye"+fpx[1]+" "+fpy[1]
    							+ ", eyes midpoint = (" + eyescenter.x + "," + eyescenter.y +")"); 
    			} catch (Exception e) { 
    				Log.e(TAG, "setFace(): face " + i + ": " + e.toString());
    			}            
				endTime = System.currentTimeMillis();
				totTime = endTime - startTime; 
				Log.e(TAG,"total Usage= "+totTime/1000+"sec");  
    	}
    	//Getting time zone 0413
    	//For saving the result picture into SD card 0413
		try {
			fos = new FileOutputStream(strFilePath);
	    	draw.compress( Bitmap.CompressFormat.JPEG, 100, fos );
	    	fos.close();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	//mIV.setDisplayPoints(fpx, fpy, count * 2, 1);
    } 
    public native int[] sendToC(int a, int b,int[] picture);//sender value to c function 

	public void mMakeTextToast(String str,boolean isLong)
	{
		if(isLong==true)
		{
			Toast.makeText(TutorialOnFaceDetect.this, str, Toast.LENGTH_LONG).show();
		}else
		{
			Toast.makeText(TutorialOnFaceDetect.this, str, Toast.LENGTH_SHORT).show();
		}
	}
	private boolean checkSDCard()
	{
		if(android.os.Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED))
		{
			return true;
		}else
		{
			return false;
		}
	}	
	public String getSDPath()
	{
		File sdDir = null;
		boolean sdCardExist = Environment.getExternalStorageState()
		.equals(android.os.Environment.MEDIA_MOUNTED); //判斷sd卡是否存在
		if (sdCardExist)
		{
			sdDir = Environment.getExternalStorageDirectory();//獲取跟目錄
		}
		//Log.e(TAG,"the path of sd "+sdDir.toString());
		return sdDir.toString();
	}
	public static int computeSampleSize(BitmapFactory.Options options,
	        int minSideLength, int maxNumOfPixels) {
	    int initialSize = computeInitialSampleSize(options, minSideLength,
	            maxNumOfPixels);
	 
	    int roundedSize;
	    if (initialSize <= 8) {
	        roundedSize = 1;
	        while (roundedSize < initialSize) {
	            roundedSize <<= 1;
	        }
	    } else {
	        roundedSize = (initialSize + 7) / 8 * 8;
	    }
	 
	    return roundedSize;
	}
	 
	private static int computeInitialSampleSize(BitmapFactory.Options options,
	        int minSideLength, int maxNumOfPixels) {
	    double w = options.outWidth;
	    double h = options.outHeight;
	 
	    int lowerBound = (maxNumOfPixels == -1) ? 1 :
	            (int) Math.ceil(Math.sqrt(w * h / maxNumOfPixels));
	    int upperBound = (minSideLength == -1) ? 128 :
	            (int) Math.min(Math.floor(w / minSideLength),
	            Math.floor(h / minSideLength));
	 
	    if (upperBound < lowerBound) {
	        return lowerBound;
	    }
	 
	    if ((maxNumOfPixels == -1) &&
	            (minSideLength == -1)) {
	        return 1;
	    } else if (minSideLength == -1) {
	        return lowerBound;
	    } else {
	        return upperBound;
	    }
	}   
}