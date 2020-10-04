#pragma once

#include "ofMain.h"
#include "ofFbo.h"
#include "ofxTexture3d.h"

// Is this class even needed?
class ofxFboSettings3d : public ofFboSettings {
public:
    int depth;
    ofxFboSettings3d(std::shared_ptr<ofBaseGLRenderer> renderer = nullptr);
private:
    std::weak_ptr<ofBaseGLRenderer> renderer;
    friend class ofxFbo3d;
};

// Class for three dimensional fbos
class ofxFbo3d
{
    public:
        ofxFbo3d();
        virtual ~ofxFbo3d();
        
        void allocate(int w, int h, int d, int internalGlDataType);
        void allocate(ofxFboSettings3d settings = ofxFboSettings3d(nullptr));

        bool isAllocated() const;

        void clear();

        // No draw calls

        ofxTexture3d& getTexture();
        ofxTexture3d& getTexture(int attachmentPoint);

        // TODO:
        //void begin(ofFboMode mode = OF_FBOMODE_PERSPECTIVE | OF_FBOMODE_MATRIXFLIP);

        // TODO:
        //void end() const;

        float getWidth() const;
        float getHeight() const;
        float getDepth() const;

        bool checkStatus() const;

        // No need to call this because we aren't using MSAA
        // (muliple colorbuffer 's)
        //void updateTexture(int attachmentPoint);

private:

    GLuint fbo;
    GLuint fboTextures;

    std::vector<ofxTexture3d>	textures;

    mutable std::vector<bool> dirty;

    std::vector<GLenum>		activeDrawBuffers;

    int defaultTextureIndex; //used for getTextureReference

    bool bIsAllocated = false;

    ofxFboSettings3d settings;

    void createAndAttachTexture(GLenum internalFormat, GLenum attachmentPoint);
    void attachTexture(ofxTexture3d& tex, GLenum internalFormat, GLenum attachmentPoint);



};
