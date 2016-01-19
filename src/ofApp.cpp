#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // shader
    if(ofIsGLProgrammableRenderer()){
        shader.load("shadersGL3/shader");
    } else {
        shader.load("shadersGL2/shader");
    }
   
    // init camera
    int width = 320;
    int height = 240;
    cam.setDeviceID(1);
    cam.initGrabber(width, height);
  
    // init fbo
    maskFbo.allocate(width, height);
    maskFbo.begin();
    ofClear(0, 0, 0, 255);
    ofFile maskFile;
    if(maskFile.open(ofToDataPath("mask.png"))) {
        ofImage i;
        i.load(maskFile);
        i.draw(0, 0);
    }
    maskFbo.end();
    
    fbo.allocate(width, height);
    fbo.begin();
    ofClear(0, 0, 0, 255);
    fbo.end();
   
    // ui settings
    aratio = cam.getWidth()/cam.getHeight();
    w = 200;
    h = w/aratio;
    rate = w / cam.getWidth();
}

//--------------------------------------------------------------
void ofApp::update(){
    cam.update();

    // update camera
    ofImage camImg = cam.getPixels();
   
    // draw a mask
    if (bBrushDown) {
        maskFbo.begin();
        
        ofSetColor(255, 0, 0);
        ofDrawEllipse(mouseX/rate , mouseY/rate, rad, rad);
        
        maskFbo.end();
    }
   
    // fbo
    fbo.begin();
    ofClear(0, 0, 0, 0);
    
    shader.begin();
    shader.setUniformTexture("maskTex", maskFbo.getTexture(), 1);
    cam.draw(0, 0);
   
    shader.end();
    fbo.end();
    
    // copy fbo to memory
    ofSetColor(255);
    fbo.readToPixels(pixels);
    frameImg.setFromPixels(pixels);

    // Background Subtraction(MoG)
    bgSub(toCv(frameImg), result);

    // queuing results
    if (ofGetFrameNum() % 1 == 0) {
        cv::Mat mat;
        result.copyTo(mat);
        results.emplace_back(mat);
        if (results.size() > 31) {
            results.pop_front();
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    // live camera
    ofSetColor(255, 255, 255, 10);
    cam.draw(0, 0, w, h);
   
    // masked camera
    ofSetColor(255, 255, 255, 240);
    fbo.draw(0, 0, w, h);
 
    // mask
    ofSetColor(255);
    maskFbo.draw(0, h, w, h);
    
    // capture image that is passed to bgSub
    frameImg.draw(w, 0, w, h);
    
    // draw!
    drawMat(result, w, h*2, w*2, h*2);

    for (auto it = results.begin(); it != results.end(); it++) {
        ofSetColor(255, 255, 255, 20);
        drawMat(*it, w*2, 0, w*2, h*2);
        //drawMat(*it, w*2, (results.end() - it)*20, w*2, h*2);
    }
    
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
        maskFbo.begin();
        ofClear(0, 0, 0, 255);
        maskFbo.end();
    } else if (key == 'e') {
        rad += 5;
    } else if (key == 'w') {
        rad -= 5;
    } else if (key == 's') {
        ofPixels savePixels;
        ofImage saveImage;
        maskFbo.readToPixels(savePixels);
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