#pragma once

#include "ofMain.h"
#include "ofxCv.h"

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

    ofVideoGrabber cam;
    cv::BackgroundSubtractorMOG2 bgSub;
    bool bBrushDown;
    int rad = 20;
    
    // mask
    ofShader shader;
    ofFbo maskFbo;
    ofFbo fbo;
    ofImage img;
    ofPixels pixels;
    cv::Mat result;
};
