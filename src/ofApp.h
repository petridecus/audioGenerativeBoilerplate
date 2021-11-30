#pragma once

#include "ofMain.h"
#include "ofxFBX.h"

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

		ofSpherePrimitive sphere;
		ofLight light;
		ofTexture noiseTexture;
		ofTexture grungeTexture;
		ofEasyCam cam;

		ofColor sphereColor;

		ofxFBX fbx;

		bool bRenderNormals = false;
		bool bRenderMeshes = true;
		bool bDrawBones = false;
};
