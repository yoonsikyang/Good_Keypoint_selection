#define _STDINT
#include <windows.h>
#include <process.h>
#include <fstream>
#include <iostream>
#include <list>

#include "stdint.h"
#include "brisk/brisk.h"
//#include "projection.h"
#include "brisk/Matcher.h"
#include "brisk/MatchVerifier.hpp"
#include "brisk/GroundTruth.hpp"

#include "freak/freak.h"

//#include "HandyAR/HandyAR.h"
//
//#include <gl/glut.h>
//#include "SungwookUtility.hpp"
//#include "SungwookFeature.hpp"
//#include "SungwookAR.hpp"
//#include "calibration/Camera.h"
//#include "KatoPoseEstimation/KatoPoseEstimator.h"
//#include "wonjo.h"

#include <opencv2/legacy/legacy.hpp>

#include "opencv_files.h"
#include "parameters.h"
//#include "freak/hammingseg.h"
//#include "gcFilterMatches.cpp"



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

	char str_socre_txt[50];
	char str_genuine_txt[50];
	char str_imposed_txt[50];

	char imgA_filename[30];
	char imgB_filename[30];

	FILE* fp_match_scores;
	FILE* fp_genuine_scores;
	FILE* fp_imposed_scores;
	sprintf(str_socre_txt,"../scores/matching_scores.txt");			
	if((fp_match_scores=fopen(str_socre_txt,"w")) == NULL)
	{
		printf("파일을 생성하지 못했습니다.");
		return;
	}

	sprintf(str_genuine_txt,"../scores/genuine_scores.txt");	
	if((fp_genuine_scores=fopen(str_genuine_txt,"w")) == NULL)
	{
		printf("파일을 생성하지 못했습니다.");
		return;
	}

	sprintf(str_imposed_txt,"../scores/imposed_scores.txt");	
	if((fp_imposed_scores=fopen(str_imposed_txt,"w")) == NULL)
	{
		printf("파일을 생성하지 못했습니다.");
		return;
	}



	// Load images
	for(int imgA_idx = 0 ; imgA_idx < IMGA_NUMBER ; imgA_idx++)
	{
		sprintf(imgA_filename,"../trainingset/%d.jpg", imgA_idx+1);			
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
		FREAKFF descriptorExtractor_freak;
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


		/////////////////////////////////////////////////////////////////////////////////
		/*
		for(int imgB_idx = 0 ; imgB_idx < IMGB_NUMBER ; imgB_idx++)
		{
		sprintf(imgB_filename,"../testset/%d.jpg", imgB_idx+1);			
		IplImage *imgB_ipl = cvLoadImage(imgB_filename);
		CvMat* rot_mat_B = cvCreateMat(2,3, CV_32FC1);	
		imgB_scaled_rotated = cvCloneImage( imgB_ipl );
		imgB_scaled_rotated->origin = imgB_ipl->origin;
		cvZero( imgB_scaled_rotated );
		CvPoint2D32f center_B = cvPoint2D32f(imgB_ipl->width/2, imgB_ipl->height/2);	
		cv2DRotationMatrix(center_B, angle_B, scale_B, rot_mat_B);
		cvWarpAffine(imgB_ipl, imgB_scaled_rotated, rot_mat_B, INTER_NEAREST);		

		IplImage* gray_B = cvCreateImage( cvGetSize(imgB_scaled_rotated), IPL_DEPTH_8U, 1);	 //의심스러운 부분
		cvCvtColor(imgB_scaled_rotated, gray_B, CV_RGB2GRAY);
		Mat imgB = Mat(gray_B);

		Mat imgB_color = Mat(imgB_scaled_rotated);


		/////////////////// DETECTION ///////////////////		
		detector_B->detect(imgB, keypointsB);



		///////////////////// DESCRIPTOR ///////////////////
		//BRISK//
		keypointsB_brisk = keypointsB;
		descriptorExtractor_brisk->compute(imgB, keypointsB_brisk, descriptorsB_brisk);
		vector<KeyPoint> keypointsB_brisk_rise;


		//FREAK//
		keypointsB_freak = keypointsB;
		descriptorExtractor_freak.compute(imgB, keypointsB_freak, descriptorsB_freak);
		vector<KeyPoint> keypointsB_freak_rise;		



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

		//FREAK//
		matcher.match(descriptorsA_freak, descriptorsB_freak, matches_freak);
		matcher.match(descriptorsB_freak, descriptorsA_freak, matches_opp_freak);
		gcFilterMatches(matches_freak, matches_opp_freak, best_matches_freak, keypointsA_freak, keypointsB_freak);			

		//BRISK+FREAK//
		vector<DMatch> matches_common;
		for(int i=0 ; i<matches_brisk.size() ; i++)
		{
		for(int j=0 ; j<matches_freak.size() ; j++)
		{
		if(    (keypointsA_brisk[matches_brisk[i].queryIdx].pt.x == keypointsA_freak[ matches_freak[j].queryIdx ].pt.x) 
		&& (keypointsA_brisk[matches_brisk[i].queryIdx].pt.y == keypointsA_freak[ matches_freak[j].queryIdx ].pt.y) 

		&& (keypointsB_brisk[matches_brisk[i].trainIdx].pt.x == keypointsB_freak[ matches_freak[j].trainIdx ].pt.x)   
		&& (keypointsB_brisk[matches_brisk[i].trainIdx].pt.y == keypointsB_freak[ matches_freak[j].trainIdx ].pt.y)  )
		{
		matches_common.push_back(matches_brisk[i]);
		}
		}
		}

		vector<DMatch> matches_opp_common;
		for(int i=0 ; i<matches_opp_brisk.size() ; i++)
		{
		for(int j=0 ; j<matches_opp_freak.size() ; j++)
		{
		if(    (keypointsA_brisk[matches_opp_brisk[i].trainIdx].pt.x == keypointsA_freak[ matches_opp_freak[j].trainIdx ].pt.x) 
		&& (keypointsA_brisk[matches_opp_brisk[i].trainIdx].pt.y == keypointsA_freak[ matches_opp_freak[j].trainIdx ].pt.y) 

		&& (keypointsB_brisk[matches_opp_brisk[i].queryIdx].pt.x == keypointsB_freak[ matches_opp_freak[j].queryIdx ].pt.x)   
		&& (keypointsB_brisk[matches_opp_brisk[i].queryIdx].pt.y == keypointsB_freak[ matches_opp_freak[j].queryIdx ].pt.y)  )
		{
		matches_opp_common.push_back(matches_opp_brisk[i]);
		}
		}
		}



		gcFilterMatches(matches_common, matches_opp_common, best_matches_common, keypointsA_brisk, keypointsB_brisk);



		std::cout << "IMAGE INDEX " << imgA_idx+1 << "-" << imgB_idx+1 << " : " 
		<< "T : "<< keypointsA.size() 
		<< " Q : " << keypointsB.size() 
		<< " BRISK :" << best_matches_brisk.size() 
		<< " FREAK :" << best_matches_freak.size()
		<< " BOTH :" << best_matches_common.size() <<std::endl;// 매칭갯수 출력		



		fprintf(fp_match_scores,"%d-%d %d %d %d %d %d \n",
		imgA_idx+1, imgB_idx+1, keypointsA.size(), keypointsB.size(), best_matches_brisk.size(), best_matches_freak.size(), best_matches_common.size() );



		if(     (  (imgA_idx+1 == 1) && (imgB_idx+1 > 0) && (imgB_idx+1 < 11)      )
		||  (  (imgA_idx+1 == 2) && (imgB_idx+1 > 10) && (imgB_idx+1 < 21)     )
		||  (  (imgA_idx+1 == 3) && (imgB_idx+1 > 20) && ( imgB_idx+1 < 31)    )
		||  (  (imgA_idx+1 == 4) && (imgB_idx+1 > 30) && ( imgB_idx+1 < 41)    )
		||  (  (imgA_idx+1 == 5) && (imgB_idx+1 > 40) && ( imgB_idx+1 < 51)    )
		||  (  (imgA_idx+1 == 6) && (imgB_idx+1 > 50) && ( imgB_idx+1 < 61)    )
		||  (  (imgA_idx+1 == 7) && (imgB_idx+1 > 60) && ( imgB_idx+1 < 71)    )
		||  (  (imgA_idx+1 == 8) && (imgB_idx+1 > 70) && ( imgB_idx+1 < 81)    )
		||  (  (imgA_idx+1 == 9) && (imgB_idx+1 > 80) && ( imgB_idx+1 < 91)    )
		||  (  (imgA_idx+1 == 10) && (imgB_idx+1 > 90) && ( imgB_idx+1 < 101)  )            )
		fprintf(fp_genuine_scores,"%d %d %d \n", best_matches_brisk.size(), best_matches_freak.size(), best_matches_common.size() );												
		else
		fprintf(fp_imposed_scores,"%d %d %d \n", best_matches_brisk.size(), best_matches_freak.size(), best_matches_common.size() );												


		//matches_brisk.~vector<DMatch>();
		//matches_freak.~vector<DMatch>();
		//matches_opp_brisk.~vector<DMatch>();
		//matches_opp_freak.~vector<DMatch>();

		//keypointsB.~vector<KeyPoint>();
		//keypointsB_brisk.~vector<KeyPoint>();			
		//keypointsB_freak.~vector<KeyPoint>();			
		//descriptorsB_brisk.~Mat();
		//descriptorsB_freak.~Mat();		

		imgB.release();
		//cvReleaseImage(&imgB_scaled_rotated);
		cvReleaseImage(&gray_B);	

		//cvReleaseImage(&imgB_ipl); //이거 활성화 시키면 종료시 에러남			
		}
		*/
		

		CvCapture* cap = cvCaptureFromCAM(0);

		
		IplImage *imgB_ipl;
		CvMat* rot_mat_B;
		IplImage* gray_B;

		Mat imgB;
		Mat imgB_color;

		while(true)
		{
			//sprintf(imgB_filename,"../testset/%d.jpg", 1);			
			//IplImage *imgB_ipl = cvLoadImage(imgB_filename);

			cvWaitKey(100);
			
			cvGrabFrame(cap);
			imgB_ipl = cvRetrieveFrame(cap);

			cvShowImage("IplImage",imgB_ipl);
			//imshow("image",image);

			rot_mat_B = cvCreateMat(2,3, CV_32FC1);	
			imgB_scaled_rotated = cvCloneImage( imgB_ipl );
			imgB_scaled_rotated->origin = imgB_ipl->origin;
			cvZero( imgB_scaled_rotated );
			CvPoint2D32f center_B = cvPoint2D32f(imgB_ipl->width/2, imgB_ipl->height/2);	
			cv2DRotationMatrix(center_B, angle_B, scale_B, rot_mat_B);
			cvWarpAffine(imgB_ipl, imgB_scaled_rotated, rot_mat_B, INTER_NEAREST);		

			gray_B = cvCreateImage( cvGetSize(imgB_scaled_rotated), IPL_DEPTH_8U, 1);	 //의심스러운 부분
			cvCvtColor(imgB_scaled_rotated, gray_B, CV_RGB2GRAY);
			imgB = Mat(gray_B);

			imgB_color = Mat(imgB_scaled_rotated);


			/////////////////// DETECTION ///////////////////		
			detector_B->detect(imgB, keypointsB);



			///////////////////// DESCRIPTOR ///////////////////
			//BRISK//
			keypointsB_brisk = keypointsB;
			descriptorExtractor_brisk->compute(imgB, keypointsB_brisk, descriptorsB_brisk);
			vector<KeyPoint> keypointsB_brisk_rise;


			//FREAK//
			keypointsB_freak = keypointsB;
			descriptorExtractor_freak.compute(imgB, keypointsB_freak, descriptorsB_freak);
			vector<KeyPoint> keypointsB_freak_rise;		



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

			//FREAK//
			matcher.match(descriptorsA_freak, descriptorsB_freak, matches_freak);
			matcher.match(descriptorsB_freak, descriptorsA_freak, matches_opp_freak);
			gcFilterMatches(matches_freak, matches_opp_freak, best_matches_freak, keypointsA_freak, keypointsB_freak);			

			//BRISK+FREAK//
			vector<DMatch> matches_common;
			for(int i=0 ; i<matches_brisk.size() ; i++)
			{
				for(int j=0 ; j<matches_freak.size() ; j++)
				{
					if(    (keypointsA_brisk[matches_brisk[i].queryIdx].pt.x == keypointsA_freak[ matches_freak[j].queryIdx ].pt.x) 
						&& (keypointsA_brisk[matches_brisk[i].queryIdx].pt.y == keypointsA_freak[ matches_freak[j].queryIdx ].pt.y) 

						&& (keypointsB_brisk[matches_brisk[i].trainIdx].pt.x == keypointsB_freak[ matches_freak[j].trainIdx ].pt.x)   
						&& (keypointsB_brisk[matches_brisk[i].trainIdx].pt.y == keypointsB_freak[ matches_freak[j].trainIdx ].pt.y)  )
					{
						matches_common.push_back(matches_brisk[i]);
					}
				}
			}

			vector<DMatch> matches_opp_common;
			for(int i=0 ; i<matches_opp_brisk.size() ; i++)
			{
				for(int j=0 ; j<matches_opp_freak.size() ; j++)
				{
					if(    (keypointsA_brisk[matches_opp_brisk[i].trainIdx].pt.x == keypointsA_freak[ matches_opp_freak[j].trainIdx ].pt.x) 
						&& (keypointsA_brisk[matches_opp_brisk[i].trainIdx].pt.y == keypointsA_freak[ matches_opp_freak[j].trainIdx ].pt.y) 

						&& (keypointsB_brisk[matches_opp_brisk[i].queryIdx].pt.x == keypointsB_freak[ matches_opp_freak[j].queryIdx ].pt.x)   
						&& (keypointsB_brisk[matches_opp_brisk[i].queryIdx].pt.y == keypointsB_freak[ matches_opp_freak[j].queryIdx ].pt.y)  )
					{
						matches_opp_common.push_back(matches_opp_brisk[i]);
					}
				}
			}



			gcFilterMatches(matches_common, matches_opp_common, best_matches_common, keypointsA_brisk, keypointsB_brisk);



			std::cout << "IMAGE INDEX " << imgA_idx+1 << " : " 
				<< "T : "<< keypointsA.size() 
				<< " Q : " << keypointsB.size() 
				<< " BRISK :" << best_matches_brisk.size() 
				<< " FREAK :" << best_matches_freak.size()
				<< " BOTH :" << best_matches_common.size() <<std::endl;// 매칭갯수 출력		



			//fprintf(fp_match_scores,"%d-%d %d %d %d %d %d \n",
			//	imgA_idx+1, imgB_idx+1, keypointsA.size(), keypointsB.size(), best_matches_brisk.size(), best_matches_freak.size(), best_matches_common.size() );



			if(     (  (imgA_idx+1 == 1) )
				||  (  (imgA_idx+1 == 2)  )
				||  (  (imgA_idx+1 == 3)  )
				||  (  (imgA_idx+1 == 4)  )
				||  (  (imgA_idx+1 == 5) )
				||  (  (imgA_idx+1 == 6) )
				||  (  (imgA_idx+1 == 7)   )
				||  (  (imgA_idx+1 == 8)  )
				||  (  (imgA_idx+1 == 9)   )
				||  (  (imgA_idx+1 == 10)  )   )
				printf("%d %d %d \n", best_matches_brisk.size(), best_matches_freak.size(), best_matches_common.size() );												
			else
				printf("%d %d %d \n", best_matches_brisk.size(), best_matches_freak.size(), best_matches_common.size() );												


			//matches_brisk.~vector<DMatch>();
			//matches_freak.~vector<DMatch>();
			//matches_opp_brisk.~vector<DMatch>();
			//matches_opp_freak.~vector<DMatch>();

			//keypointsB.~vector<KeyPoint>();
			//keypointsB_brisk.~vector<KeyPoint>();			
			//keypointsB_freak.~vector<KeyPoint>();			
			//descriptorsB_brisk.~Mat();
			//descriptorsB_freak.~Mat();		

			

			imgB.release();
			//cvReleaseImage(&imgB_scaled_rotated);
			cvReleaseImage(&gray_B);	

			//cvReleaseImage(&imgB_ipl); //이거 활성화 시키면 종료시 에러남			
		}






		//keypointsA.~vector<KeyPoint>();
		//keypointsA_brisk.~vector<KeyPoint>();			
		//keypointsA_freak.~vector<KeyPoint>();			
		//descriptorsA_brisk.~Mat();
		//descriptorsA_freak.~Mat();		

		cvReleaseImage(&imgA_ipl);
	}


	//cvNamedWindow("Matches",1);	


	//		outimg = Mat::zeros(imgB.rows, imgB.cols, imgB.type());
	//
	//		vector<KeyPoint> keypointsA_dummy;
	//		vector<KeyPoint> keypointsB_dummy;
	//
	//		//Draw Match로 그리기		
	//		drawMatches(imgA_color, keypointsA_dummy, imgB_color, keypointsB_dummy, dummy, outimg, Scalar(0,0,255), Scalar(0,0,255));
	//		//drawMatches(imgA_color, keypointsA_brisk, imgB_color, keypointsB_brisk, best_matches_brisk, outimg, Scalar(0,0,255), Scalar(0,0,255));
	//		//drawMatches(imgA_color, keypointsA_freak, imgB_color, keypointsB_freak, best_matches_freak, outimg, Scalar(0,0,255), Scalar(0,0,255));
	//		//drawMatches(imgA_color, keypointsA_brisk, imgB_color, keypointsB_brisk, best_matches_common, outimg, Scalar(0,0,255), Scalar(0,0,255));		
	//
	//
	//
	//		//matches2points
	//		//gcMatches2Points(best_matches_brisk, keypointsA_brisk_filtered, keypointsB_brisk_filtered, keypointsA_brisk, keypointsB_brisk);
	//		//gcMatches2Points(best_matches_freak, keypointsA_freak_filtered, keypointsB_freak_filtered, keypointsA_freak, keypointsB_freak);
	//		gcMatches2Points(best_matches_common, keypointsA_common_filtered, keypointsB_common_filtered, keypointsA_brisk, keypointsB_brisk);		
	//
	//
	//		//Draw Homography
	//		//gcHomography(computeHomography, imgA, outimg,  keypointsA_brisk_filtered, keypointsB_brisk_filtered, Scalar(0, 0, 255)); // RED
	//		//gcHomography(computeHomography, imgA, outimg,  keypointsA_freak_filtered, keypointsB_freak_filtered, Scalar(0, 0, 255)); // BLUE
	//		gcHomography(computeHomography, imgA, outimg,  keypointsA_common_filtered, keypointsB_common_filtered, Scalar(0, 0, 255)); // GREEN
	//
	//
	//
	//		imshow("Matches", outimg);
	//		//cvShowImage("Matches", outimg);
	//

	//
	//		//ESC 누르면 스톱
	//		c = cvWaitKey(10);//이거 안쓰면 영상이 안뜸



	waitKey(0);
	fclose(fp_match_scores);
	fclose(fp_genuine_scores);
	fclose(fp_imposed_scores);

	return;
}