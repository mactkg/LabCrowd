#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(30);
    
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
    auto dev = cam.listDevices();
    
    for (auto it = dev.begin(); it != dev.end(); it++) {
        ofLogNotice() << it->id << ": " << it->deviceName;
    }
    
    cam.setDeviceID(0);
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
   
    contourFinder.setThreshold(128);
    contourFinder.setMaxAreaRadius(400);
    contourFinder.setMinAreaRadius(5);
   
    results.clear();
    
    // ui settings
    aratio = cam.getWidth()/cam.getHeight();
    updateVideoWidth(450);
    
    gui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
   
    gui->setWidth(600);
    gui->addFRM();
    gui->addSlider("video.width", 10, 600, 450);
    
    gui->addBreak();
    
    gui->addSlider("cf.threshold", 0, 255, 128);
    gui->addSlider("cf.rad.max", 0, 800, 400);
    gui->addSlider("cf.rad.min", 0, 100, 5);
    
    gui->addBreak();
  
    gui->addButton("Clear buffer");
    gui->addButton("Force update");
    
    gui->onSliderEvent(this, &ofApp::onSliderEvent);
    gui->onButtonEvent(this, &ofApp::onButtonEvent);
    
    gui->addHeader("LabCrowd");
    gui->setTheme(new ofxDatGuiThemeCharcoal());
    gui->addFooter();
}

//--------------------------------------------------------------
void ofApp::update(){
    cam.update();
    
    // draw a mask
    if (bBrushDown && mouseX > w*2 && mouseX < w*3 && mouseY < h) {
        int x = (mouseX - w*2)/rate;
        int y = mouseY/rate;
        maskAreaFbo.begin();
       
        ofLogNotice() << "draw:" << x << ", " << y;
        ofSetColor(255, 0, 0, 255);
        ofDrawEllipse(x, y, rad, rad);
        
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
    // *: shoot
    // o: through
    // []: cv::Mat
    // oF: [o*o*o*o*][o*o*o*o*][o*o*o*o*]... in 30fps
    if (ofGetFrameNum() % 2 == 0 && ofGetFrameNum() >= 1) {
        cv::Mat mat;
        result.copyTo(mat);
        
        if(ofGetFrameNum() % 8 == 0 || results.empty()) {
            results.emplace_back(mat);
            //if (results.size() > 30) { // 30 * 8frame(actually 4 frames): about 8sec
            if (results.size() > 300) { // 300 * 8frame(actually 4 frames): about 80sec
                results.pop_front();
            }
            
            // update composed histories
            composedResults = *results.begin();
            for (auto it = results.begin(); it != results.end(); it++) {
                bitwise_or(composedResults, *it, composedResults);
            }
        
            contourFinder.findContours(composedResults);
        } else {
            bitwise_and(*(results.end()-1), mat, *(results.end()-1));
        }
    }
    
    if (((ofGetFrameNum() % (30 * 8)) == 0 && ofGetFrameNum() > 0) || forceUpdate) {
        updateState();
        forceUpdate = false;
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    cam.draw(0, 0, w, h);

    // live camera
    ofSetColor(255, 255, 255, 80);
    cam.draw(w*2, 0, w, h);
   
    // masked camera
    ofSetColor(255, 255, 255, 240);
    secondFbo.draw(w*2, 0, w, h);
 
    // mask
    ofSetColor(255);
    maskAreaFbo.draw(w, 0, w, h);
    
    // capture image that is passed to bgSub
    frameImg.draw(0, h, w, h);
    
    // composed history
    drawMat(result, w, h, w, h*1);
    drawMat(composedResults, w*2, h, w*1, h*1);
    
    // contourFinder
    ofPushMatrix();
    ofTranslate(w*2, h);
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
void ofApp::onSliderEvent(ofxDatGuiSliderEvent e){
    cout << "onSliderEvent: " << e.target->getLabel() << " " << e.target->getValue() << endl;
    if (e.target->is("video.width")) updateVideoWidth(e.value);
    if (e.target->is("cf.threshold")) contourFinder.setThreshold(e.value);
    if (e.target->is("cf.rad.max")) contourFinder.setMaxAreaRadius(e.value);
    if (e.target->is("cf.rad.min")) contourFinder.setMinAreaRadius(e.value);
}
//--------------------------------------------------------------
void ofApp::onButtonEvent(ofxDatGuiButtonEvent e){
    cout << "onButtonEvent: " << e.target->getLabel() << "Pressed" << endl;
    if (e.target->is("clear buffer")) {
        results.clear();
        composedResults.zeros(composedResults.rows, composedResults.cols, composedResults.type());
    } else if(e.target->is("force update")) {
        forceUpdate = true;
    }
}

//--------------------------------------------------------------
// blocking!!
void ofApp::updateState() {
    string state;
    bool yes = false;
    auto rects = contourFinder.getBoundingRects();
    ofLogNotice() << rects.size();
    for (auto it = rects.begin(); it != rects.end(); it++) {
        ofLogNotice("rect size") << it->area();
        if(it->area() > 10000) {
            yes = true;
        }
    }
    
    if (rects.size() > 2 || yes) {
        state = "人居る";
    } else {
        state = "人居ない";
    }
    string node = "/Users/kenta/.ndenv/shims/node";
    string script = ofToDataPath("../../scripts/update.js", true);
    string command = node + " " + script + " --key " + MACTKG_API_KEY + " --state " + state;
    ofLogNotice() << state;
    
    sys.init(command);
    sys.startThread();
}



//helper--------------------------------------------------------
void ofApp::updateVideoWidth(float width) {
    w = width;
    h = w/aratio;
    rate = w / cam.getWidth();
}