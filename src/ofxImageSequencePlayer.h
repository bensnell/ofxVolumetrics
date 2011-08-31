#pragma once

#include "ofMain.h"

class ofxImageSequencePlayer: public ofImage {
    public:
        ofxImageSequencePlayer() {
            setUseTexture(false);
        };
        void init(string prefix, int digits, string extension, int start) {
            curFrame = start;
            startFrame = start;
            fPrefix = prefix;
            fExt = extension;
            numDigits = digits;
            initialized = loadImage(generateFullFilename());

            if(!initialized){
                ofLogWarning() << "ofxImageSequencePlayer: unable to load inital image in the sequence: " << generateFullFilename();
                return;
            }

            totalFrames = 0;
            ofFile testFile;
            while(testFile.open(generateFullFilename()))
            {
                totalFrames++;
                curFrame++;
            }
            testFile.close();
            curFrame = startFrame;
        };

        bool loadNextFrame() {
            bool worked = loadImage(generateFullFilename());
            if(worked)
                curFrame++;
            return worked;
        };

        bool loadPreviousFrame() {
            bool worked = loadImage(generateFullFilename());
            if(worked)
                curFrame = curFrame > 0 ? curFrame - 1 : 0;
            return worked;
        };

        bool loadFrame(int n) {
            curFrame = startFrame + n;
            bool worked = loadImage(generateFullFilename());
            if(worked)
                curFrame++;
            return worked;
        };

        int getCurrentFrameNumber() { return curFrame - startFrame; }
        void setCurrentFrameNumber(int i) { curFrame = startFrame + i; }
        int getSequenceLength() { return totalFrames; }
        bool isInitialized() { return initialized; }

        string generateFullFilename() {
            std::ostringstream ss;
            ss << std::setw(numDigits) << std::setfill('0') << curFrame;
            std::string result = ss.str();
            if (result.length() > numDigits)
            {
                result.erase(0, result.length() - numDigits);
            }
            return fPrefix + ss.str() + fExt;
        };

    private:
        bool initialized;
        int curFrame, startFrame;
        string fPrefix, fExt;
        int numDigits;
        int totalFrames;
};
