#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    ofSetCircleResolution(80);
    ofBackground(54, 54, 54);

    int bufferSize = 256;

    left.assign(bufferSize, 0.0);
    right.assign(bufferSize, 0.0);
    volHistory.assign(400, 0.0);

    bufferCounter = 0;
    drawCounter = 0;
    smoothedVol = 0.0;
    scaledVol = 0.0;

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
    // once per frame, first
	//lets scale the vol up to a 0-1 range
	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);

	//lets record the volume into an array
	volHistory.push_back( scaledVol );

	//if we are bigger the the size we want to record - lets drop the oldest value
	if( volHistory.size() >= 400 ){
		volHistory.erase(volHistory.begin(), volHistory.begin()+1);
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofDrawBitmapString("Scaled average vol (0-100): " + ofToString(scaledVol * 100.0, 0), 4, 18);

    if (scaledVol >= 0.5 && !aboveThreshold) {
        gateTrigger();
        aboveThreshold = true;
    } else if (scaledVol < 0.5 && aboveThreshold) {
        aboveThreshold = false;
    }

    float size = scaledVol * 190.0f;

    // once per frame, second
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
    ofDrawRectangle(-size / 2, -size / 2, size, size);
}

//--------------------------------------------------------------
void ofApp::gateTrigger() {
    std::cout << "cycling color" << std::endl;
    ofSetColor( ofColor(ofRandom(100, 255), ofRandom(100, 255), ofRandom(100, 255)) );
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer &input) {
    // std::cout << "in audioIn callback function!" << std::endl;

    float curVol = 0.0;

    // samples are "interleaved"
    int numCounted = 0;

    //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
    for (size_t i = 0; i < input.getNumFrames(); i++){
        // std::cout << input[i] << std::endl;

        left[i] = input[i*2]*0.5;
        right[i] = input[i*2+1]*0.5;

        curVol += left[i] * left[i];
        curVol += right[i] * right[i];
        numCounted+=2;
    }

    //this is how we get the mean of rms :)
    curVol /= (float)numCounted;

    // this is how we get the root of rms :)
    curVol = sqrt( curVol );

    smoothedVol *= 0.93;
    smoothedVol += 0.07 * curVol;

    bufferCounter++;
}
