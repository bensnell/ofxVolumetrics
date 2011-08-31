#pragma once

/*      ofxVolumetrics - render volumetric data on the GPU

 Written by Timothy Scaffidi (http://timothyscaffidi.com)
 Volumetric rendering algorithm adapted from Peter Trier (http://www.daimi.au.dk/~trier/?page_id=98)

 Usage:
    copy the shaders folder to your app's bin/data folder

    setup:
        void setup(int w, int h, int d, ofVec3f voxelSize);
        voxelSize is the size of each voxel, W:H:D, used for streching the volume, think pixel aspect ratio in video.
        If there is a data set that looks squashed with 1:1:1, try 1:1:2.

    load with data:
        create a voxel buffer unsigned char array. This must be RGBA! rendering voxels without an alpha channel defeats the point.

        void updateVolumeData(unsigned char * data, int w, int h, int d, int xOffset, int yOffset, int zOffset);

        you may upload the entire volume at once, or sub-portions of the volume using the x y and z offset amounts amd chaning the size of upload.
        Note that moving data from system memory to GFX memory is quite slow!
        Updating the entire volume every frame will be very slow, only update what has changed if you want to do animation.

    render:
        perform any tranformations as per usual
        void drawVolume(float x, float y, float z, float size, int zTexOffset);
        zTexOffset is used to slide the texture coordinates around on the z axis, which may be useful for animation if by uploading one slice at a time
*/

#include "ofFbo.h"
#include "ofShader.h"
#include "ofxTexture3d.h"
#include "ofxImageSequencePlayer.h"

class ofxVolumetrics
{
    public:
        ofxVolumetrics();
        virtual ~ofxVolumetrics();
        void setup(int w, int h, int d, ofVec3f voxelSize);
        void destroy();
        void updateVolumeData(unsigned char * data, int w, int h, int d, int xOffset, int yOffset, int zOffset);
        void drawVolume(float x, float y, float z, float size, int zTexOffset);
        void drawVolume(float x, float y, float z, float w, float h, float d, int zTexOffset);
        bool isInitialized();
        int getVolumeWidth();
        int getVolumeHeight();
        int getVolumeDepth();
        int getRenderWidth();
        int getRenderHeight();
        float getXyQuality();
        float getZQuality();
        float getThreshold();
        float getDensity();
        void setXyQuality(float q);
        void setZQuality(float q);
        void setThreshold(float t);
        void setDensity(float d);
        void setRenderSettings(float xyQuality, float zQuality, float dens, float thresh);
    protected:
    private:
        void drawRGBCube();
        void updateRenderDimentions();

        ofFbo fboBackground, fboRender;
        ofShader volumeShader;
        ofxTexture3d volumeTexture;
        //ofMesh volumeMesh; //unfortunately this only supports 2d texture coordinates at the moment.
        ofVec3f volVerts[24];
        ofVec3f volNormals[24];
        ofVec3f voxelRatio;
        bool bIsInitialized;
        int volWidth, volHeight, volDepth;
        ofVec3f quality;
        float threshold;
        float density;
        int renderWidth, renderHeight;
};
