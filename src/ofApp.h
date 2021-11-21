#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void audioIn(ofSoundBuffer & input);

		vector <float> left;
		vector <float> right;
		vector <float> volHistory;

		int 	bufferCounter;
		int 	drawCounter;

		float smoothedVol;
		float scaledVol;

		ofSoundStream stream;
		ofSoundStreamSettings settings;
};