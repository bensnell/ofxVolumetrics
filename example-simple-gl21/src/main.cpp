#include "ofMain.h"
#include "ofApp.h"
#include "ofGLProgrammableRenderer.h" // GL3

//========================================================================
int main( ){
    
    ofGLWindowSettings settings;
    settings.setGLVersion(2,1); /// < select your GL Version here
    ofCreateWindow(settings); ///< create your window here
    ofRunApp(new ofApp());
    
}
