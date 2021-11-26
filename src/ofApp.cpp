#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    ofSetCircleResolution(80);
    ofBackground(54, 54, 54);

    int bufferSize = 256;

    left.assign(bufferSize, 0.0);
    right.assign(bufferSize, 0.0);

    volHistoryLeft.assign(400, 0.0);
    volHistoryRight.assign(400, 0.0);

    bufferCounter = 0;
    drawCounter = 0;

    smoothedVolLeft = 0.0;
    smoothedVolRight = 0.0;

    scaledVolLeft = 0.0;
    scaledVolRight = 0.0;

    // AUDIO
    auto devices = stream.getDeviceList(ofSoundDevice::ALSA);
    stream.printDeviceList();

    if (!devices.empty()) {
        settings.setInDevice(devices[5]);
        std::cout << "setting up alsa audio input" << std::endl;
    }

    settings.setInListener(this);
    settings.sampleRate = 48000;
    settings.bufferSize = 256;
    settings.numInputChannels = 2;
    settings.numOutputChannels = 0;
    settings.numBuffers = 4;

    stream.setup(settings);

    aboveThreshold = false;
}

//--------------------------------------------------------------
void ofApp::update() {
    // scale vols up to a 0-1 range
    scaledVolLeft = ofMap(smoothedVolLeft, 0.0, 0.17, 0.0, 1.0, true);
    scaledVolRight = ofMap(smoothedVolRight, 0.0, 0.17, 0.0, 1.0, true);

    //lets record the volume into an array
    volHistoryLeft.push_back( scaledVolLeft );
    volHistoryRight.push_back( scaledVolRight );

    // drop oldest values in each stereo channel's buffer
    if( volHistoryLeft.size() >= 400 ){
        volHistoryLeft.erase(volHistoryLeft.begin(), volHistoryLeft.begin()+1);
    }
    if( volHistoryRight.size() >= 400 ){
        volHistoryRight.erase(volHistoryRight.begin(), volHistoryRight.begin()+1);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofDrawBitmapString("Left: " + ofToString(scaledVolLeft * 100.0, 0), 4, 18);
    ofDrawBitmapString("Right: " + ofToString(scaledVolRight * 100.0, 0), 4, 48);

    if (scaledVolLeft >= 0.5 && !aboveThreshold) {
        gateTrigger();
        aboveThreshold = true;
    } else if (scaledVolLeft < 0.5 && aboveThreshold) {
        aboveThreshold = false;
    }

    float sizeLeft = scaledVolLeft * 190.0f;
    float sizeRight = scaledVolRight * 190.0f;

    // once per frame, second
    ofTranslate(ofGetWidth() / 3, ofGetHeight() / 2);
    ofDrawRectangle(-sizeLeft / 2, -sizeLeft / 2, sizeLeft, sizeLeft);

    ofTranslate(ofGetWidth() / 3, 0);
    ofDrawRectangle(-sizeRight / 2, -sizeRight / 2, sizeRight, sizeRight);
}

//--------------------------------------------------------------
void ofApp::gateTrigger() {
    std::cout << "cycling color" << std::endl;
    ofSetColor( ofColor(ofRandom(100, 255), ofRandom(100, 255), ofRandom(100, 255)) );
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer &input) {
    float curVolLeft = 0.0;
    float curVolRight = 0.0;

    // samples are "interleaved"
    int numCounted = 0;

    //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
    for (size_t ii = 0; ii < input.getNumFrames(); ++ii){
        left[ii] = input[ii*2]*0.5;
        right[ii] = input[ii*2+1]*0.5;

        curVolLeft += left[ii] * left[ii];
        curVolRight += right[ii] * right[ii];

        ++numCounted;
    }

    //this is how we get the mean of rms :)
    curVolLeft /= (float)numCounted;
    curVolRight /= (float)numCounted;

    // this is how we get the root of rms :)
    curVolLeft = sqrt( curVolLeft );
    curVolRight = sqrt( curVolRight );

    smoothedVolLeft *= 0.93;
    smoothedVolRight *= 0.93;

    smoothedVolLeft += 0.07 * curVolLeft;
    smoothedVolRight += 0.07 * curVolRight;

    bufferCounter++;
}
