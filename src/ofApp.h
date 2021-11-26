#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void audioIn(ofSoundBuffer & input);

		void gateTrigger();

		float red;
		float green;
		float blue;

		vector <float> left;
		vector <float> right;

		vector <float> volHistoryLeft;
		vector <float> volHistoryRight;

		int bufferCounter;
		int drawCounter;

		float smoothedVolLeft;
		float smoothedVolRight;

		float scaledVolLeft;
		float scaledVolRight;

		ofSoundStream stream;
		ofSoundStreamSettings settings;

		bool aboveThreshold;
};
