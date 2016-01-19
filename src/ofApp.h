#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "key.h"

//#define USE_CAMERA

using namespace ofxCv;
using namespace cv;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
        void updateData(string state);
   
#ifdef USE_CAMERA
    ofVideoGrabber cam;
#else
    ofVideoPlayer cam;
#endif
    
    cv::BackgroundSubtractorMOG2 bgSub;
    bool bBrushDown;
    int rad = 20;
    
    // mask
    ofShader firstShader; // gaussY
    ofShader secondShader; // gaussX, mask;
    ofFbo maskAreaFbo;
    ofFbo firstFbo;
    ofFbo secondFbo;
    
    ofImage frameImg;
    ofPixels pixels;
    cv::Mat result;
    
    std::deque<cv::Mat> results;
    
    // ui settings
    float aratio, rate;
    int w, h;
};