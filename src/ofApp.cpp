#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // shader
    if(ofIsGLProgrammableRenderer()){
        firstShader.load("shaderGL3/shader.vert", "shaderGL3/gaussianY.frag");
        secondShader.load("shaderGL3/shader.vert", "shaderGL3/gaussianXandMask.frag");
    } else {
        firstShader.load("shadersGL2/gaussianY");
        secondShader.load("shadersGL2/gaussianXandMask");
    }
   
#ifdef USE_CAMERA
    int width = 480;
    int height = 270;
    cam.setDeviceID(1);
    cam.initGrabber(width, height);
#else
    // init camera
    cam.load("movies/test_movie.mov");
    cam.play();
    
    int width = cam.getWidth();
    int height = cam.getHeight();
#endif
  
    // init fbo
    maskAreaFbo.allocate(width, height);
    maskAreaFbo.begin();
    ofClear(0, 0, 0, 255);
    ofFile maskFile;
    if(maskFile.open(ofToDataPath("mask.png"))) {
        ofImage i;
        i.load(maskFile);
        i.draw(0, 0);
    }
    maskAreaFbo.end();
   
    firstFbo.allocate(width, height);
    firstFbo.begin();
    ofClear(0, 0, 0, 255);
    firstFbo.end();
    
    secondFbo.allocate(width, height);
    secondFbo.begin();
    ofClear(0, 0, 0, 255);
    secondFbo.end();
   
    contourFinder.setMinAreaRadius(10);
    contourFinder.setMaxAreaRadius(400);
   
    results.clear();
    
    // ui settings
    aratio = cam.getWidth()/cam.getHeight();
    w = 450;
    h = w/aratio;
    rate = w / cam.getWidth();
}

//--------------------------------------------------------------
void ofApp::update(){
    cam.update();
    
    // draw a mask
    if (bBrushDown) {
        maskAreaFbo.begin();
       
        ofLogNotice() << "draw:" << mouseX/rate << ", " << mouseY/rate;
        ofSetColor(255, 0, 0, 255);
        ofDrawEllipse(mouseX/rate, mouseY/rate, rad, rad);
        
        maskAreaFbo.end();
    }
   
    // gaussian(Y)
    firstFbo.begin();
    firstShader.begin();
    cam.draw(0, 0);
    firstShader.end();
    firstFbo.end();
   
    // gaussian(X) and mask
    secondFbo.begin();
    ofClear(0, 0, 0, 0);
    
    secondShader.begin();
    secondShader.setUniformTexture("maskTex", maskAreaFbo.getTexture(), 1);
    firstFbo.draw(0, 0);
    secondShader.end();
    
    secondFbo.end();
    
    // copy fbo to memory
    ofSetColor(255);
    secondFbo.readToPixels(pixels);
    frameImg.setFromPixels(pixels);

    bgSub(toCv(frameImg), result);

    // queuing results
    if (ofGetFrameNum() % 2 == 0 && ofGetElapsedTimef() > 1) {
        cv::Mat mat;
        result.copyTo(mat);
        results.emplace_back(mat);
        if (results.size() > 300) { // 300frame: about 10sec
            results.pop_front();
        }
        
        // update composed histories
        composedResults = *results.begin();
        for (auto it = results.begin(); it != results.end(); it++) {
            bitwise_or(composedResults, *it, composedResults);
        }
    
        float threshold = ofMap(mouseX, 0, ofGetWidth(), 0, 255);
        contourFinder.setThreshold(threshold);
        contourFinder.findContours(composedResults);
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    // live camera
    ofSetColor(255, 255, 255, 10);
    cam.draw(0, 0, w, h);
   
    // masked camera
    ofSetColor(255, 255, 255, 240);
    secondFbo.draw(0, 0, w, h);
 
    // mask
    ofSetColor(255);
    maskAreaFbo.draw(0, h, w, h);
    
    // capture image that is passed to bgSub
    frameImg.draw(w, 0, w, h);
    
    // draw!
    drawMat(result, 0, h*2, w, h*1);
    drawMat(composedResults, w*2, 0, w*1, h*1);
        
    ofPushMatrix();
    ofTranslate(w*2, 0);
    ofScale(rate*1, rate*1);
    ofSetColor(255);
    contourFinder.draw();
    ofPopMatrix();
    
    // ui
    ofPushMatrix(); ofPushStyle();
    string str;
    str += "w: rad-=5; e: rad+=5; s: save buffer SPACE: clear\n";
    str += "rad: " + ofToString(rad);
    ofDrawBitmapString(ofToString(rad), 20, 20);
    ofPopMatrix(); ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        maskAreaFbo.begin();
        ofClear(0, 0, 0, 255);
        maskAreaFbo.end();
    } else if (key == 'e') {
        rad += 5;
    } else if (key == 'w') {
        rad -= 5;
    } else if (key == 's') {
        ofPixels savePixels;
        ofImage saveImage;
        maskAreaFbo.readToPixels(savePixels);
        saveImage.setFromPixels(savePixels);
        saveImage.save("mask.png");
    } else if (key == 'u') {
        updateData(ofGetTimestampString());
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    bBrushDown = true;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    bBrushDown = false;
}

//--------------------------------------------------------------
// blocking!!
void ofApp::updateData(string state) {
    string node = "/Users/kenta/.ndenv/shims/node";
    string script = ofToDataPath("../../scripts/update.js");
    ofLogNotice() << ofSystem(node + " " + script + " --key " + MACTKG_API_KEY + " --state " + state);
}