#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    if(ofIsGLProgrammableRenderer()){
        shader.load("shadersGL3/shader");
    } else {
        shader.load("shadersGL2/shader");
    }
    
   
    int width = 320;
    int height = 240;
    cam.initGrabber(width, height);
   
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
}

//--------------------------------------------------------------
void ofApp::update(){
    cam.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    // update camera
    ofImage camImg = cam.getPixels();
   
    // draw a mask
    if (bBrushDown) {
        maskFbo.begin();
        
        ofSetColor(255, 0, 0);
        ofDrawEllipse(mouseX, mouseY, rad, rad);
        
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
    
    // live camera
    ofSetColor(255, 255, 255, 10);
    cam.draw(0, 0);
   
    // masked camera
    ofSetColor(255, 255, 255, 240);
    fbo.draw(0, 0);
 
    // mask
    ofSetColor(255);
    maskFbo.draw(0, 400);
 
    i.draw(700, 0);
    
    // copy fbo to memory
    ofSetColor(255);
    fbo.readToPixels(pixels);
    img.setFromPixels(pixels);
    img.draw(400, 0);
   
    // Background Subtraction(MoG)
    bgSub(toCv(img), result);
    
    // draw!
    drawMat(result, 400, 400);
    
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