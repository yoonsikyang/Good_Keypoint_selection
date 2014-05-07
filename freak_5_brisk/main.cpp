#define _STDINT
#include <windows.h>
#include <process.h>
#include <fstream>
#include <iostream>
#include <list>

#include "stdint.h"
#include "brisk/brisk.h"
#include "brisk/Matcher.h"
#include "brisk/MatchVerifier.hpp"
#include "brisk/GroundTruth.hpp"

#include "freak/freak.h"


#include "opencv_files.h"
#include "parameters.h"
#include <opencv2/legacy/legacy.hpp>
#include "value.h"


vector<int> histogram;

#define PI			3.14159265358979323846
#define DEG_TO_RAD	(PI/180.0)

Mat crop( Mat src,  CvRect roi)
{
	cv::Mat roiImg;
	cv::Mat ret;
	roiImg = src(roi);

	roiImg = src(roi).clone();

	resize(roiImg,ret,Size(roi.height*2,roi.width*2));

	imshow("ss",ret);
	cvWaitKey(0);

	return ret;
}

IplImage *transform(const IplImage *src, float angle, float centerX, float centerY,
	float scaleX, float scaleY,
	float moveX, float moveY ){
		IplImage *imageRotated = cvCreateImage(cvSize(src->width,src->height),src->depth,3);

		float sina = sin(-angle * DEG_TO_RAD);
		float cosa = cos(-angle * DEG_TO_RAD);
		CvMat*  transmat = cvCreateMat( 2,3, CV_32F );
		cvmSet( transmat, 0,0, scaleX*cosa );
		cvmSet( transmat, 0,1, scaleY*sina );
		cvmSet( transmat, 0,2, -centerX*scaleX*cosa - centerY*scaleY*sina + moveX + centerX );
		cvmSet( transmat, 1,0, -1.0*scaleX*sina );
		cvmSet( transmat, 1,1, scaleY*cosa );
		cvmSet( transmat, 1,2, -centerY*scaleY*cosa + centerX*scaleX*sina + moveY + centerY);

		cvWarpAffine( src, imageRotated, transmat);

		cvReleaseMat( &transmat );
		return imageRotated;
}

IplImage *rotateImage(const IplImage *src, int angleDegrees, double scale)
{
	//resize
	int w = src->width * scale;
	int h = src->height * scale;
	IplImage* temp= cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, 3 );
	cvResize( src, temp );

	float width = sqrt((double)temp->width * (double)temp->width + temp->height * temp->height);

	IplImage* rotateImage = cvCreateImage( cvSize(width,width), IPL_DEPTH_8U, 3 );
	cvSet(rotateImage,cvScalar(0));

	CvPoint center;
	center.x = rotateImage->width / 2.0;
	center.y = rotateImage->height / 2.0;


	//복사하고 회전
	cvSetImageROI(rotateImage, cvRect(center.x - temp->width / 2.0, center.y - temp->height / 2.0, temp->width, temp->height));
	cvCopy(temp,rotateImage);
	cvResetImageROI(rotateImage);


	IplImage *imageRotated;
	imageRotated = transform(rotateImage, angleDegrees, rotateImage->width/2, rotateImage->height/2, 1,1, 0,0 );

	/*
	*	점 이동
	*/

	/*
	CvPoint point = cvPoint(center.x - temp->width / 2 , center.y - temp->height / 2);

	point.x = point.x - center.x;
	point.y = point.y - center.y;

	CvPoint rotatePoint;

	rotatePoint.x = center.x + point.x * cos(angleDegrees * DEG_TO_RAD) - point.y * sin(angleDegrees * DEG_TO_RAD);
	rotatePoint.y = center.y + (point.x * sin(angleDegrees * DEG_TO_RAD)) + (point.y * cos(angleDegrees * DEG_TO_RAD));

	point.x = point.x + center.x;
	point.y = point.y + center.y;

	cvCircle(imageRotated,point,3,CV_RGB(255,0,0));
	cvCircle(imageRotated,rotatePoint,3,CV_RGB(0,255,0));

	cout<<rotatePoint.x <<" "<<rotatePoint.y<<endl;
	*/

	cvReleaseImage(&temp);
	cvReleaseImage(&rotateImage);

	return imageRotated;
}


int countCorrectMatch(IplImage *src,IplImage *imageRotated, int angleDegrees, double scale, vector<DMatch> match, vector<KeyPoint> DBKeypoint, vector<KeyPoint> liveKeypoint){
	int correctMatches = 0;
	//resize
	int w = src->width * scale;
	int h = src->height * scale;
	IplImage* temp= cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, 3 );
	cvResize( src, temp );

	float width = sqrt((double)temp->width * (double)temp->width + temp->height * temp->height);

	IplImage* rotateImage = cvCreateImage( cvSize(width,width), IPL_DEPTH_8U, 3 );
	cvSet(rotateImage,cvScalar(0));

	CvPoint center;
	center.x = rotateImage->width / 2.0;
	center.y = rotateImage->height / 2.0;

	for(int i = 0; i < match.size(); i++){
		int i1 = match[i].queryIdx; 
		int i2 = match[i].trainIdx; 

		CvPoint point = cvPoint(DBKeypoint[i1].pt.x* scale + (center.x - (src->width * scale / 2)) , DBKeypoint[i1].pt.y*scale + (center.y - (src->height * scale / 2)));
		point.x = point.x - center.x;
		point.y = point.y - center.y;

		CvPoint rotatePoint;

		rotatePoint.x = center.x + point.x * cos(angleDegrees * DEG_TO_RAD) - point.y * sin(angleDegrees * DEG_TO_RAD);
		rotatePoint.y = center.y + (point.x * sin(angleDegrees * DEG_TO_RAD)) + (point.y * cos(angleDegrees * DEG_TO_RAD));

		//cvLine(imageRotated,rotatePoint,liveKeypoint[i2].pt,cvScalar(255,255,0),2);

		if(abs((int)liveKeypoint[i2].pt.x - rotatePoint.x)<=3  && abs((int)liveKeypoint[i2].pt.y - rotatePoint.y)<=3){
			cvCircle(imageRotated, rotatePoint, 3,CV_RGB(255,0,0),2);
			cvCircle(imageRotated,liveKeypoint[i2].pt,3,CV_RGB(0,255,0),2);
			histogram.at(i1)++;
			correctMatches++;
		}
	}
	/*cvCircle(imageRotated,point,3,CV_RGB(255,0,0));
	cvCircle(imageRotated,rotatePoint,3,CV_RGB(0,255,0));
	*/

	//cvShowImage("imageRotated", imageRotated);
	//cvWaitKey(0);

	cvReleaseImage(&temp);
	cvReleaseImage(&rotateImage);
	return correctMatches;
}


void main(void) {

	//Variables
	std::vector<KeyPoint> keypointsA, keypointsB;	
	std::vector<KeyPoint> keypointsA_initial, keypointsB_initial;	
	std::vector<KeyPoint> keypointsA_brisk, keypointsB_brisk;
	std::vector<KeyPoint> keypointsA_freak, keypointsB_freak;	

	Mat descriptorsA_brisk, descriptorsB_brisk;
	Mat descriptorsA_freak, descriptorsB_freak;
	Mat descriptorsA_train, descriptorsB_train;	
	vector<DMatch> matches_brisk, matches_opp_brisk;
	vector<DMatch> matches_freak, matches_opp_freak;	

	Ptr<FeatureDetector> detector_A, detector_B;	
	Ptr<FeatureDetector> detector_A_raw, detector_B_raw;	
	Ptr<DescriptorExtractor> descriptorExtractor_brisk;	
	Ptr<DescriptorMatcher> descriptorMatcher;		

	vector<KeyPoint>			kp_database;
	Mat							desc_database;
	Mat							img_database;
	vector<Point2f>				obj_matching_corners(4);

	Mat outimg;

	bool computeHomography = true;
	bool hamming = true;

	char imgA_filename[30];
	char imgB_filename[30];

	FILE* histogram_f;


	FileStorage fsG("GoodFeature.xml",FileStorage::WRITE);
	FileStorage fsB("BadFeature.xml",FileStorage::WRITE);

	std::vector<KeyPoint> goodFeatureKeypoint,badFeatureKeypoint;
	Mat goodFeatureDescriptor, badFeatureDescriptor;


	histogram.resize(500);
	goodFeatureKeypoint.resize(0);
	badFeatureKeypoint.resize(0);

	// Load images
	for(int imgA_idx = 0 ; imgA_idx < IMGA_NUMBER ; imgA_idx++)
	{
		sprintf(imgA_filename,"../testset/%d.jpg", imgA_idx+1);			
		IplImage *imgA_ipl = cvLoadImage(imgA_filename);
		//IplImage *imgA_ipl_fancy = cvLoadImage("C:\\love\\Picture_11.jpg");		

		//Mat imgA = imread("C:\\love\\Reference_full.bmp", CV_LOAD_IMAGE_GRAYSCALE );
		IplImage* gray_A = cvCreateImage( cvGetSize(imgA_ipl), IPL_DEPTH_8U, 1);
		cvCvtColor(imgA_ipl, gray_A, CV_RGB2GRAY);
		Mat imgA = Mat(gray_A);	
		//Mat imgA_color = Mat(imgA_ipl_fancy);
		char c;//delay 주기 위한 변수

		/////////////////// DETECTION ///////////////////
		detector_A_raw = new BriskFeatureDetector(briskdetector_threshold_A, brisk_octave); // default : (60. 2)
		detector_B_raw = new BriskFeatureDetector(briskdetector_threshold_B, brisk_octave);	
		detector_A = new GridAdaptedFeatureDetector(detector_A_raw, 500, 1, 1);
		detector_B = new GridAdaptedFeatureDetector(detector_A_raw, 500, 1, 1);

		detector_A->detect(imgA, keypointsA);

		///////////////////// DESCRIPTOR ///////////////////
		//BRISK//
		descriptorExtractor_brisk = new BriskDescriptorExtractor(); // writer extractor
		keypointsA_brisk = keypointsA;	// brisk 위한 키포인트
		descriptorExtractor_brisk->compute(imgA, keypointsA_brisk, descriptorsA_brisk);		

		//FREAK//
		FREAK descriptorExtractor_freak;
		keypointsA_freak = keypointsA;	// freak 위한 키포인트
		descriptorExtractor_freak.compute(imgA, keypointsA_freak, descriptorsA_freak);		

		/////////////////// MATCHER ///////////////////
		//descriptorMatcher = new BruteForceMatcher<HammingSse>(); 	
#if CV_SSSE3
		BruteForceMatcher< HammingSeg<30,4> > matcher;
		//DescriptorMatcher<Hamming> matcher;
		//cv::FlannBasedMatcher matcher(new cv::flann::LshIndexParams(20,10,2));
#else	
		//DescriptorMatcher matcher;
		//Ptr<DescriptorMatcher> matcher;
		BruteForceMatcher<Hamming> matcher; // recent matcher
		//matcher = new BruteForceMatcher<Hamming>(); // writer matcher
		//BFMatcher matcher( NORM_HAMMING, true );
		//BFMatcher matcher( NORM_L2, false);
		//BruteForceMatcher<L2<float>> matcher;
#endif

		IplImage* imgB_scaled_rotated=NULL;
		double maxScale = MAX_SCALE;
		double scale = SCALE;
		for(scale = SCALE; scale < maxScale; scale += SCALE_INCREASE)
			for(int imgB_idx = 0 ; imgB_idx < IMGB_NUMBER ; imgB_idx++)
			{
				sprintf(imgB_filename,"../testset/%d.jpg", imgB_idx+1);			
				IplImage *imgB_ipl = cvLoadImage(imgB_filename);

				int angle = 0;

				for(int i = 0 ; angle < 360 ; i++){
					imgB_scaled_rotated = rotateImage(imgB_ipl,angle, scale);

					IplImage* gray_B = cvCreateImage( cvGetSize(imgB_scaled_rotated), IPL_DEPTH_8U, 1);
					cvCvtColor(imgB_scaled_rotated, gray_B, CV_RGB2GRAY);
					Mat imgB = Mat(gray_B);

					Mat imgB_color = Mat(imgB_scaled_rotated);


					sprintf(imgB_filename,"%d-%d-%d",imgA_idx+1, imgB_idx+1, angle);		

					//cvShowImage(imgB_filename, imgB_scaled_rotated);
					//cvWaitKey(0);
					/////////////////// DETECTION ///////////////////		
					detector_B->detect(imgB, keypointsB);

					///////////////////// DESCRIPTOR ///////////////////
					//BRISK//
					keypointsB_brisk = keypointsB;
					descriptorExtractor_brisk->compute(imgB, keypointsB_brisk, descriptorsB_brisk);
					vector<KeyPoint> keypointsB_brisk_rise;


					std::vector<DMatch> dummy, best_matches_brisk, best_matches_freak, best_matches_common;
					std::vector<cv::Point2f> keypointsA_all, keypointsB_all; //while문 안에 없으면 tracking이 굉장히 느리게 반응함
					std::vector<cv::Point2f> keypointsA_brisk_filtered, keypointsB_brisk_filtered; //while문 안에 없으면 tracking이 굉장히 느리게 반응함
					std::vector<cv::Point2f> keypointsA_freak_filtered, keypointsB_freak_filtered;
					std::vector<cv::Point2f> keypointsA_common_filtered, keypointsB_common_filtered;
					CvPoint dst_corners[4];

					/////////////////// MATCHER ///////////////////
					//std::vector<std::vector<DMatch> > matches, matches_opp;		
					//descriptorMatcher->radiusMatch(desc_camera_matching_thread, desc_database1, matches, 100.0);						
					//BRISK//
					matcher.match(descriptorsA_brisk, descriptorsB_brisk, matches_brisk);
					matcher.match(descriptorsB_brisk, descriptorsA_brisk, matches_opp_brisk);
					gcFilterMatches(matches_brisk, matches_opp_brisk, best_matches_brisk, keypointsA_brisk, keypointsB_brisk);			

					
					int correct_matches = countCorrectMatch(imgB_ipl,imgB_scaled_rotated, angle, scale, best_matches_brisk, keypointsA_brisk, keypointsB_brisk);

					std::cout << "IMAGE INDEX " << imgA_idx+1 << "-" << imgB_idx+1 << " " << angle <<" : " 
						<< "T : "<< keypointsA.size() 
						<< " Q : " << keypointsB.size() 
						<< " BRISK :" << best_matches_brisk.size() 
						<< " FREAK :" << best_matches_freak.size()
						<< " BOTH :" << best_matches_common.size() 
						<< " Correct Match" << correct_matches
						<< std::endl;// 매칭갯수 출력


					cvReleaseImage(&gray_B);
					cvReleaseImage(&imgB_scaled_rotated);
					imgB.release();


					angle += ANGLE;
				}		
			}
			if((histogram_f=fopen("histogram.txt","w")) == NULL)
			{
				printf("파일을 생성하지 못했습니다.");
				return;
			}

			int t = T;
			char path[100];
			
			for(int i = 0; i < keypointsA_brisk.size();i++){
				fprintf(histogram_f,"%d\n",histogram.at(i));
				
				if(histogram.at(i)>t){
					goodFeatureKeypoint.push_back(keypointsA_brisk[i]);
				}else{
					badFeatureKeypoint.push_back(keypointsA_brisk[i]);
				}
				/*Rect roi = Rect(keypointsA_brisk[i].pt.x - 5,keypointsA_brisk[i].pt.y - 5,10,10);
				crop(imgA,roi);*/

			}


			descriptorExtractor_brisk->compute(imgA, goodFeatureKeypoint, goodFeatureDescriptor);
			descriptorExtractor_brisk->compute(imgA, badFeatureKeypoint, badFeatureDescriptor);

			fclose(histogram_f);

			sprintf(path, "Keypoint");
			write(fsG, path, goodFeatureKeypoint);
			sprintf(path, "Descriptor");
			write(fsG, path, goodFeatureDescriptor);
			sprintf(path, "Keypoint");
			write(fsB, path, badFeatureKeypoint);
			sprintf(path, "Descriptor");
			write(fsB, path, badFeatureDescriptor);

			//keypointsA.~vector<KeyPoint>();
			//keypointsA_brisk.~vector<KeyPoint>();			
			//keypointsA_freak.~vector<KeyPoint>();			
			//descriptorsA_brisk.~Mat();
			//descriptorsA_freak.~Mat();		

			cvReleaseImage(&imgA_ipl);
	}

	waitKey(0);


	return;
}