#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // FBX SETUP
    ofxFBXSource::Scene::Settings fbxSettings;
    fbxSettings.filePath = "astroBoy_walk.fbx";

    fbxSettings.printInfo = true;

    ofSetLogLevel(OF_LOG_VERBOSE);

    if(fbx.load(fbxSettings)) {
        cout << "loaded the scene" << endl;
    } else {
        cout << "error loading the scene" << endl;
    }

    fbx.setAnimation(0);
    fbx.setPosition(0, -7, 0);

    // VIDEO
    ofDisableAlphaBlending();
    ofEnableDepthTest();
    ofDisableArbTex();

    ofLoadImage(noiseTexture, "noise_texture.png");
    ofLoadImage(grungeTexture, "grunge_texture.png");

    cam.setDistance(50);
    sphereColor = ofColor(255, 255, 255);

    // AUDIO
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
    bRenderNormals  = false;
    bRenderMeshes   = true;
    bDrawBones      = false;
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

    light.setPosition( cos(ofGetElapsedTimef()*2.) * 7, 4 + sin( ofGetElapsedTimef() ) * 2.5, 10  );

    ofVec3f target( ofMap( ofGetMouseX(), 0, ofGetWidth(), -10, 10, true), fbx.getPosition().y, fbx.getPosition().z+10 );
    fbx.lookAt( target );
    fbx.panDeg( 180 );

    fbx.getCurrentAnimation().setSpeed( ofMap( scaledVolRight, 0, 1, 0.5, 2.5, true ));

    // moves the bones into place based on the animation //
    fbx.earlyUpdate();

    // perform any bone manipulation here //
    shared_ptr<ofxFBXBone> bone = fbx.getBone("head");
    if( bone ) {
        bone->pointTo( light.getPosition(), ofVec3f(-1,0,0) ) ;
    }

    // manipulates the mesh around the positioned bones //
    fbx.lateUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
    cam.begin();

    if (scaledVolLeft >= 0.5 && !aboveThreshold) {
        gateTrigger();
        aboveThreshold = true;
    } else if (scaledVolLeft < 0.5 && aboveThreshold) {
        aboveThreshold = false;
    }

    if( bRenderMeshes ) {
        ofSetColor( 255, 255, 255 );
        fbx.draw();
    }

    if(bDrawBones) {
        fbx.drawSkeletons( 0.5 );
    }

    if( bRenderNormals ) {
        ofSetColor( 255, 0, 255 );
        fbx.drawMeshNormals( 0.5, false );
    }

    ofSetColor( sphereColor );
    noiseTexture.bind();
    ofDrawSphere( light.getPosition(), 5 * scaledVolLeft );
    noiseTexture.unbind();

    cam.end();
}

//--------------------------------------------------------------
void ofApp::gateTrigger() {
    std::cout << "cycling color" << std::endl;
    sphereColor = ofColor(ofRandom(100, 255), ofRandom(100, 255), ofRandom(100, 255));
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
