// CannyStill.cpp

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include<iostream>
#include<conio.h>           // may have to modify this line if not using Windows
#include<math.h>
#include <iomanip>


using namespace std;
using namespace cv;
using namespace cv::cuda;

bool iswhite(int B, int G, int R)
{
	bool B_ok = true;
	bool G_ok = true;
	bool R_ok = true;
	
	if (B < 130) { B_ok = false;}if (G < 130) { G_ok = false; }if (R < 130) { R_ok = false; } // knock out low outliers
	float colourdist = (pow(double(pow(double(255 - R), 2) + pow(double(255 - B), 2) + pow(double(255 - G), 2)),0.5));
	if (colourdist > 180) { B_ok = false; } // knock out numbers very far in distance from white.
	float avg = (G + B + R) / 3;
	float percent_thresh =43;
	/*
	if (avg > 200) {
		float percent_thresh = 43;
	}
	if ((avg <= 190) && (avg>180)) {
		float percent_thresh = 40.5;
	}
	if ((avg <= 190) && (avg>170)) {
		float percent_thresh = 38;
	}
	if ((avg <= 160) && (avg>150)) {
		float percent_thresh = 33;
	}
	if ((avg <= 150) && (avg>140)) {
		float percent_thresh = 28;
	}
	*/
	float low_B = B*(1-(percent_thresh/100)); 
	float high_B = B*(1+(percent_thresh/100));
	float low_R = R*(1 - (percent_thresh / 100));
	float high_R = R*(1 + (percent_thresh / 100));

	if ((low_B > float(G)) || (float(G) > high_B)) { G_ok = false; } //knock out greens far from blues 
	if ((low_B > float(R)) || (float(R) > high_B)) { R_ok = false; } //knock out reds far from blues 
	if ((low_R > float(G)) || (float(G) > high_R)) { G_ok = false; } //knock out greens far from reds 

	int culmulative_distance = abs(B - G) + abs(B - R) + abs(R - G);
	if (culmulative_distance > 115) { R_ok = false; }

	if (colourdist < 80) { return(true); } // override all if very small distance from white. 
	return (G_ok && B_ok && R_ok);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
int main() {


	cv::Mat imgOriginal;        // input image
	cv::Mat imgGrayscale;       // grayscale of input image
	cv::Mat imgforwhitedetection;         // intermediate blured image
	cv::Mat imgCanny;           // Canny edge image
	vector<Vec3f> output;

	imgOriginal = cv::imread("image3.jpg");          // open image

	if (imgOriginal.empty()) {                                  // if unable to open image
		std::cout << "error: image not read from file\n\n";     // show error message on command line
		_getch();                                               // may have to modify this line if not using Windows
		return(0);                                              // and exit program
	}

	cv::cvtColor(imgOriginal, imgGrayscale, CV_BGR2GRAY);       // convert to grayscale
	cv::cvtColor(imgOriginal, imgforwhitedetection, CV_BGR2Lab);
	//cv::GaussianBlur(imgGrayscale,          // input image
	//	imgBlurred,                         // output image
	//	cv::Size(5, 5),                     // smoothing window width and height in pixels
	//	1.5);                               // sigma value, determines how much the image will be blurred
	

	cv::Canny(imgGrayscale,           // input image
		imgCanny,                   // output image
		75,                        // low threshold
		100);                       // high threshold
	/*
	///
	line detection
	///
	*/

	vector<cv::Vec4i> lines;
	HoughLinesP(imgCanny, lines, 1, CV_PI / 180, 75, 500, 30);
	bool ignorel = false;
	int linedirection[200] = { 0 };// so far: 0 ignore line, 1 vertical line, 2 horizontal line. 
	int M = 0;
	// the final lines are stored as botthoriz. right vert. top hori, left verti
	Vec4i finallines[4] = { {100,100,4800,100},{4000,100,4000,4800},{4000,4800,100,4800},{100,4800,100,100} }; // begin by assuming square of 4800 pixels.
	cout <<  finallines[0][1] << finallines[1] << "SUPER ORIGINAL TOP/BOTTOM" << endl;
	for (size_t r = 0; r < lines.size(); r++)
	{
		if (r == 0) { cout << lines.size()<< " < SIZE "<<finallines[0][2] << finallines[2][2] << "ORIGINAL TOP/BOTTOM" << endl; }

		ignorel = false;
		Vec4i l = lines[r];

		Vec4i others = lines[r];

		if (abs((int(l[0]) - int(l[2]))) < 40) { linedirection[r] = 1; } // check x begin/x end within certain distance(vertical).
		if (abs((int(l[1]) - int(l[3]))) < 40) { linedirection[r] = 2; } // check y begin/y end within certain distance(horizontal). 

		if (linedirection[r] == 1) { l[1] = 10; l[3] = 100000; l[0] = l[2]; } // align, extend vertical lines. 
		if (linedirection[r] == 2) { l[0] = 10; l[2] = 100000; l[1] = l[3]; 
		cout << l << endl;
		} // align, extend horizontal lines.

		if (r == 0) { cout << finallines[0] << finallines[2] << "ORIGINAL TOP/BOTTOM" << endl; }

		for (int k = 1; k < r; k++) {
			others = lines[r - k];

			if ((int(l[0]) > int(others[0]) - 20) && (int(l[0]) < int(others[0]) + 20)) { linedirection[r] = 0; }
			if ((int(l[1]) > int(others[1]) - 20) && (int(l[1]) < int(others[1]) + 20)) { linedirection[r] = 0; }
			if ((int(l[2]) > int(others[2]) - 20) && (int(l[2]) < int(others[2]) + 20)) { linedirection[r] = 0; }
			if ((int(l[3]) > int(others[3]) - 20) && (int(l[3]) < int(others[3]) + 20)) { linedirection[r] = 0; }
			
			if ((linedirection[r] == 2)) { cout << l << "this is the line" << endl;
			cout << l[1] << "needs to be lower than 2000 and higher than  ->" << finallines[0][1] << endl;
			cout << "or" << l[1] << "needs to be higher than 2000 and lower than  ->" << finallines[2][1] << endl;
			}

			if ((linedirection[r] == 1) && (l[0] < 2000) && (l[0]) > finallines[3][0]) { finallines[3] = l; }
			if ((linedirection[r] == 1) && (l[0] > 2000) && (l[0]) < finallines[1][0]) { finallines[1] = l; }
			if ((linedirection[r] == 2) && (l[1] < 2000) && (l[1]) > finallines[0][1]) { finallines[0] = l; }
			if ((linedirection[r] == 2) && (l[1] > 2000) && (l[1]) < finallines[2][1]) { finallines[2] = l; }
			//ingore lines generated from points very close to eachother. 

		}

		if (linedirection[r] == 0) {
			M = M + 1;
			//cout << M << "line code" << l[0] << " < xstart " << l[1] << " <ystart " << l[2] << " <xend " << l[3] << "yend"<<endl ;
			//line(imgOriginal, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, CV_AA);
		}
	}

		for (int k = 0; k < 4; k++) {
	
			cout << finallines[k] << endl;
			line(imgOriginal, Point(finallines[k][0], finallines[k][1]), Point(finallines[k][2], finallines[k][3]), Scalar(0, 0, 255), 3, CV_AA);
		
		}












	cv::HoughCircles(imgGrayscale, output, CV_HOUGH_GRADIENT, 2, imgGrayscale.rows / 40, 70, 67, 65,80);
	int balls[16][6];//stored as x,y,B,G,R,#ofwhitepixels.
	int deciderarray[3];
	int tempwhitepix = 0;
	int tempnotwhite = 0;
	bool whitechecker = false;
	int ballfound = 0;
	
	//eginning of circle detection 

	for (size_t i = 0; i < output.size(); i++)
	{

		/* for testing if statement. 
		cout << output[i][0] << " < " << finallines[1][0] << " RIGHT VERTI " << endl;
		cout << output[i][0] << " > " << finallines[3][0] << " LEFT VERTI" << endl;
		cout << output[i][1] << " < " << finallines[2][1] << " BELOW TOP" << endl;
		cout << output[i][1] << " > " << finallines[0][1] << " ABOVE BOTT" << endl;
		*/


		if (		// only takes balls in the lines. 
			((output[i][0]) < (finallines[1][0])-70)		// to the left of right verti x 

			&& ((output[i][0]) > (finallines[3][0])+70) // to the right of left verti x

			&& ((output[i][1]) < (finallines[2][1])-70) // below the top hori y

			&& ((output[i][1]) > (finallines[0][1])+70) // above the bottom hori y

			) { //for testing
			ballfound = ballfound + 1;


			Point center(cvRound(output[i][0]), cvRound(output[i][1]));
			cv::Vec3b COLA(0, 0, 0);
			int radius = cvRound(output[i][2]);




			//Vec3b colour = imgOriginal.at<Vec3b>(cvRound(output[i][0]), cvRound(output[i][1]));


			deciderarray[0] = 0; deciderarray[1] = 0; deciderarray[2] = 0;
			tempwhitepix = 0; tempnotwhite = 0; whitechecker = 0;

			for (size_t j = 0; j < 10; j++) {
		
				Point current(cvRound(output[i][0] + ((radius / 10) * j*pow(-1, j))), cvRound(output[i][1]));
				COLA = imgOriginal.at<cv::Vec3b>(current);
				//imgOriginal.at<cv::Vec3b>(current) = (0, 0, 255);

				whitechecker = iswhite(int(COLA.val[0]), int(COLA.val[1]), int(COLA.val[2]));
				//cout << COLA << whitechecker << endl;

				//cout << whitechecker << "whitechecker" << endl;
				if (whitechecker == true) {
					tempwhitepix = tempwhitepix + 1;
					//cout << COLA << endl;

				}
				else {

					deciderarray[0] = deciderarray[0] + int(COLA.val[0]);
					deciderarray[1] = deciderarray[1] + int(COLA.val[1]);
					deciderarray[2] = deciderarray[2] + int(COLA.val[2]);
					//cout << deciderarray[0] << "< decider array" << int(COLA.val[0]) << "< the colour" << tempnotwhite << "< numcol" << endl;

					tempnotwhite = tempnotwhite + 1;
				}
			}

			for (size_t k = 0; k < 10; k++) {
				Point current(cvRound(output[i][0]), cvRound((output[i][1]) + ((radius / 10) * k*pow(-1, k))));
				COLA = imgOriginal.at<cv::Vec3b>(current);
				//imgOriginal.at<cv::Vec3b>(current) = (0, 0, 255);
				whitechecker = iswhite(int(COLA.val[0]), int(COLA.val[1]), int(COLA.val[2]));
				//cout << COLA << whitechecker << endl;
				if (whitechecker == true) {
					tempwhitepix = tempwhitepix + 1;
					//cout << COLA << "iswhite" << endl;
				}
				else {
					deciderarray[0] = deciderarray[0] + int(COLA.val[0]);
					deciderarray[1] = deciderarray[1] + int(COLA.val[1]);
					deciderarray[2] = deciderarray[2] + int(COLA.val[2]);
					//cout << deciderarray[0] << "< decider array" << int(COLA.val[0]) << "< the colour" << tempnotwhite << "< numcol" << endl;
					tempnotwhite = tempnotwhite + 1;
				}
			}
			if (tempnotwhite > 0) {
				//cout << "coloured numcolouredpixels >> " << tempnotwhite << endl;
				balls[ballfound][0] = cvRound(output[i][0]);
				balls[ballfound][1] = cvRound(output[i][1]);
				balls[ballfound][2] = cvRound(deciderarray[0] / tempnotwhite);
				balls[ballfound][3] = cvRound(deciderarray[1] / tempnotwhite);
				balls[ballfound][4] = cvRound(deciderarray[2] / tempnotwhite);
				balls[ballfound][5] = tempwhitepix;
			}
			else
			{
				//cout << "ball" << i << "iswhite" << endl;
				balls[ballfound][0] = cvRound(output[i][0]);
				balls[ballfound][1] = cvRound(output[i][1]);
				balls[ballfound][2] = 255;
				balls[ballfound][3] = 255;
				balls[ballfound][4] = 255;
				balls[ballfound][5] = tempwhitepix;
			}

			cout << balls[ballfound][0] << "|0|" << balls[ballfound][1] << "|1|" << balls[ballfound][2] << "|2|" << balls[ballfound][3] << "<|3|" << balls[ballfound][4] << "<|4|" << balls[ballfound][5] << "<|5|" << endl;


			int C = 0;

			if (i == 1) {
				C = 255;
			}


			// draw the circle center
			circle(imgOriginal, center, 3, Scalar(C, 255, 0), -1, 8, 0);
			// draw the circle outline
			circle(imgOriginal, center, radius, Scalar(C, 255, 0), 3, 8, 0);



		}
		//cout << i << "------------------";

	}
	

	//cv::namedWindow("circles", CV_WINDOW_NORMAL);
	//cv::imshow("circles", imgOriginal);
	




									// declare windows
	cv::namedWindow("imgOriginal", CV_WINDOW_NORMAL);     // note: you can use CV_WINDOW_NORMAL which allows resizing the window
	//cv::namedWindow("imgCanny", CV_WINDOW_NORMAL);        // or CV_WINDOW_AUTOSIZE for a fixed size window matching the resolution of the image
															// CV_WINDOW_AUTOSIZE is the default
	cv::imshow("imgOriginal", imgOriginal);     // show windows
	//cv::imshow("imgCanny", imgCanny);
	cv::waitKey(0);                 // hold windows open until user presses a key
	
	return(0);
}



