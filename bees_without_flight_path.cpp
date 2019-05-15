
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include<iostream>
#include<array>
#include<unistd.h>
#ifdef WINDOWS
#include<conio.h>           // it may be necessary to change or remove this line if not using Windows
#endif

#include "Blob.h"

using namespace cv;
using namespace std;

// global variables 
const Scalar SCALAR_BLACK = Scalar(0.0, 0.0, 0.0);
const Scalar SCALAR_WHITE = Scalar(255.0, 255.0, 255.0);
const Scalar SCALAR_BLUE = Scalar(255.0, 0.0, 0.0);
const Scalar SCALAR_GREEN = Scalar(0.0, 200.0, 0.0);
const Scalar SCALAR_RED = Scalar(0.0, 0.0, 255.0);

// resolution of detected video input
const int WIDTH = 800;
const int HEIGHT = 450;

int main(void) {

	VideoCapture capVideo;
	Mat imgFrame1;
	Mat imgFrame2;

        //capVideo.open("http://169.254.131.230:8080/stream?action=stream&dummy=param.mjpeg&amp;type=.mjpeg");
        //capVideo.open("http://192.168.1.4:8080/stream?amp;type=.mjpg");
	//capVideo.open("D:/DP/7.10.2018/VID_20181007_140025.mp4");
        //capVideo.open("http://@192.168.1.11:80/webrtc");
        //capVideo.open("tcpclientsrc host=169.254.131.230 port=8080 ! gdpdepay ! rtph264depay ! video/x-h264, width=1280, height=720, format=YUY2, framerate=49/1 ! ffdec_h264 ! autoconvert ! appsink sync=false"
//,CAP_GSTREAMER);
        //capVideo.open("http://169.254.131.230:8080/stream?type=.mjpg?action=stream");
        
        
        //capVideo.open("http://169.254.131.230:8160/"); //funguje 
        //capVideo.open("http://169.254.131.230:8554/?dummy=param.mjpeg"); //funguje
        //capVideo.open("http://169.254.131.230:8001/html/cam_pic_new.php"); //funguje použitím https://elinux.org/RPi-Cam-Web-Interface
        //capVideo.open("D:/DP/finalne_videa/jeden_ul/z_predu/jeden_ul_z_predu_7_cropped_flipped.mp4");
        
        
        //capVideo.open("D:/DP/finalne_videa/jeden_ul/z_predu/jeden_ul_z_predu_7_cropped_flipped.mp4");
        //capVideo.open("http://192.168.137.47:8554/?dummy=param.mjpeg");
        capVideo.open("http://192.168.137.47:8160/"); 
        //capVideo.open("http://192.168.137.47:80/html/cam_pic_new.php");

	if (!capVideo.isOpened()) {
		cout << "\nerror reading video file" << endl << endl;
#ifdef WINDOWS
		_getch();
#endif
		return(0);
	}

	capVideo.read(imgFrame1);
	capVideo.read(imgFrame2);
	resize(imgFrame1, imgFrame1, Size(WIDTH, HEIGHT), 0, 0, INTER_CUBIC);
	resize(imgFrame2, imgFrame2, Size(WIDTH, HEIGHT), 0, 0, INTER_CUBIC);

	// create a black image with the size as the camera output
	Mat imgLines = Mat::zeros(imgFrame1.size(), CV_8UC3);

	char chCheckForEscKey = 0;

	while (capVideo.isOpened() && chCheckForEscKey != 27) {
            
		vector<Blob> blobs;

		capVideo.read(imgFrame2);
		resize(imgFrame2, imgFrame2, Size(WIDTH, HEIGHT), 0, 0, INTER_CUBIC);

		Mat imgFrame1Copy = imgFrame1.clone();
		Mat imgFrame2Copy = imgFrame2.clone();
                
                GaussianBlur(imgFrame1, imgFrame1, Size(5, 5), 0);
                Mat gray;
                Mat edged;
                
                cvtColor(imgFrame1, gray, COLOR_BGR2GRAY);
                
                Canny(gray, edged, 200, 250);
                
                // comment out to hide filter output//////
                imshow("imgEdged", edged);
                //////////////////////////////////////////

                Mat imgDifference;
                Mat imgThresh;
                
                cvtColor(imgFrame1Copy, imgFrame1Copy, COLOR_BGR2GRAY);
		cvtColor(imgFrame2Copy, imgFrame2Copy, COLOR_BGR2GRAY);

		GaussianBlur(imgFrame1Copy, imgFrame1Copy, Size(5, 5), 0);
		GaussianBlur(imgFrame2Copy, imgFrame2Copy, Size(5, 5), 0);

		absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);
                     
                // comment out to hide filter output//////
		imshow("imgDifference", imgDifference);
                //////////////////////////////////////////
                
		threshold(imgDifference, imgThresh, 30, 255.0, THRESH_BINARY);
                
                // comment out to hide filter output//////
		imshow("imgThresh", imgThresh);
                //////////////////////////////////////////

		Mat structuringElement3x3 = getStructuringElement(MORPH_RECT, Size(3, 3));
		Mat structuringElement5x5 = getStructuringElement(MORPH_RECT, Size(5, 5));
		Mat structuringElement7x7 = getStructuringElement(MORPH_RECT, Size(7, 7));
		Mat structuringElement9x9 = getStructuringElement(MORPH_RECT, Size(9, 9));

		dilate(imgThresh, imgThresh, structuringElement5x5);
		dilate(imgThresh, imgThresh, structuringElement5x5);
		erode(imgThresh, imgThresh, structuringElement5x5);

		Mat imgThreshCopy = imgThresh.clone();

		vector<vector<Point> > contours;

		findContours(imgThreshCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		Mat imgContours(imgThresh.size(), CV_8UC3, SCALAR_BLACK);

		drawContours(imgContours, contours, -1, SCALAR_WHITE, -1);
                
                // comment out to hide filter output//////
		imshow("imgContours", imgContours);
                //////////////////////////////////////////

		vector<vector<Point> > convexHulls(contours.size());

		for (unsigned int i = 0; i < contours.size(); i++) {
			convexHull(contours[i], convexHulls[i]);
		}

		for (auto &convexHull : convexHulls) {
			Blob possibleBlob(convexHull);

			if (    possibleBlob.boundingRect.area() > 30 &&
				possibleBlob.dblAspectRatio >= 0.2 &&
				possibleBlob.dblAspectRatio <= 1.2 &&
				possibleBlob.boundingRect.width > 15 &&
				possibleBlob.boundingRect.height > 15 &&
				possibleBlob.dblDiagonalSize > 22) {
				blobs.push_back(possibleBlob);
			}
		}

		Mat imgConvexHulls(imgThresh.size(), CV_8UC3, SCALAR_BLACK);

		convexHulls.clear();

		for (auto &blob : blobs) {
			convexHulls.push_back(blob.contour);
		}

		drawContours(imgConvexHulls, convexHulls, -1, SCALAR_WHITE, -1);

		// comment out to hide filter output//////
		imshow("imgConvexHulls", imgConvexHulls);
                //////////////////////////////////////////

		imgFrame2Copy = imgFrame2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above


		for (auto &blob : blobs) {                                                  // for each blob
			rectangle(imgFrame2Copy, blob.boundingRect, SCALAR_RED, 2);             // draw a red box around the blob
		}

		imgFrame2Copy = imgFrame2Copy + imgLines;
		imshow("imgFrame2Copy", imgFrame2Copy);

		///////////////////////////////////////////
		// now we prepare for the next iteration //
                ///////////////////////////////////////////

		imgFrame1 = imgFrame2.clone();           // move frame 1 up to where frame 2 is

                // uncomment if using video file 
                //capVideo.read(imgFrame2);    
		//if ((capVideo.get(CAP_PROP_POS_FRAMES) + 1) < capVideo.get(CAP_PROP_FRAME_COUNT)) {       // if there is at least one more frame
		//	capVideo.read(imgFrame2);                            // read it
		//} else {                                                 // else
		//	cout << "end of video\n";                            // show end of video message
		//	break;                                               // and jump out of while loop
		//}

                // to slow down video for developing purpose
                //usleep(200000);
                
		chCheckForEscKey = waitKey(1);      // get key press in case user pressed esc
	}

	if (chCheckForEscKey != 27) {               // if the user did not press esc (i.e. we reached the end of the video)
		waitKey(0);                         // hold the windows open to allow the "end of video" message to show
	}
	// note that if the user did press esc, we don't need to hold the windows open, we can simply let the program end which will close the windows

	return(0);
}