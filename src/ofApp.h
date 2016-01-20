#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxDatGui.h"
#include "key.h"

//#define USE_CAMERA

using namespace ofxCv;
using namespace cv;

class SystemThread: public ofThread{
    
public:
    string cmd;
    
    void init(string _cmd){
        this->cmd = _cmd;
    }
    
    void threadedFunction(){
        if (isThreadRunning()){
            ofSystem(cmd.c_str());
        }
    }
};

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
        void updateState();
        void onSliderEvent(ofxDatGuiSliderEvent e);
        void onButtonEvent(ofxDatGuiButtonEvent e);
        void updateVideoWidth(float width);
        void clearFbo(ofFbo &fbo);
   
#ifdef USE_CAMERA
    ofVideoGrabber cam;
#else
    ofVideoPlayer cam;
#endif
   
    BackgroundSubtractorMOG2 bgSub;
    ContourFinder contourFinder;
    
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
    Mat composedResults;
   
    // ui settings
    ofxDatGui *gui;
    
    float aratio, rate;
    int w, h;
    int bufferLength;
    bool forceUpdate, reverseMask;
    SystemThread sys;
};
