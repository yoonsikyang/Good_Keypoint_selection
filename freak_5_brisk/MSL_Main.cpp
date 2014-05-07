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
}