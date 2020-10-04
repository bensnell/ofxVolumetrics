#include "ofxFbo3d.h"
#include "ofFbo.cpp"

// --------------------------------------------------
ofxFboSettings3d::ofxFboSettings3d(std::shared_ptr<ofBaseGLRenderer> renderer) {
	width = 0;
	height = 0;
	depth = 0;
	numColorbuffers = 1;
	useDepth = false;
	useStencil = false;
	depthStencilAsTexture = false;
	textureTarget = GL_TEXTURE_3D;
	internalformat = GL_RGBA;
	depthStencilInternalFormat = GL_DEPTH_COMPONENT24;
	wrapModeHorizontal = GL_CLAMP_TO_EDGE;
	wrapModeVertical = GL_CLAMP_TO_EDGE;
	minFilter = GL_LINEAR;
	maxFilter = GL_LINEAR;
	numSamples = 0;
	this->renderer = renderer;
}

// --------------------------------------------------
ofxFbo3d::ofxFbo3d()
{
	fbo = 0;
	fboTextures = 0;
	bIsAllocated = false;
	defaultTextureIndex = 0;
	// Clear dirty?
}

// --------------------------------------------------
ofxFbo3d::~ofxFbo3d()
{

}

// --------------------------------------------------
void ofxFbo3d::allocate(int w, int h, int d, int internalGlDataType) {

	settings.width = w;
	settings.height = h;
	settings.depth = d;
	settings.internalformat = internalGlDataType;

	// ?
	settings.numSamples = 0;
	settings.numColorbuffers = 1;
	settings.useDepth = false;
	settings.useStencil = false;
	settings.depthStencilAsTexture = false;

	settings.textureTarget = GL_TEXTURE_3D;

	allocate(settings);
}

// --------------------------------------------------
void ofxFbo3d::allocate(ofxFboSettings3d _settings) {
	// TODO: Check GL support
	
	// TODO: Clear
	auto renderer = _settings.renderer.lock();
	if (renderer) {
		settings.renderer = renderer;
	}
	else {
		settings.renderer = ofGetGLRenderer();
	}

	// check that passed values are correct
	if (_settings.width <= 0 || _settings.height <= 0 || _settings.depth <= 0) {
		ofLogError("ofFbo") << "width and height and depth have to be more than 0";
	}
	// numSamples = 0

	// Don't enable depth or stencil
	GLenum depthAttachment = GL_DEPTH_ATTACHMENT; // ?

	// No depth nor stencil

	// set needed values for allocation on instance settings
	// the rest will be set by the corresponding methods during allocation
	settings.width = _settings.width;
	settings.height = _settings.height;
	settings.numSamples = _settings.numSamples;

	// create main fbo
	// this is the main one we bind for drawing into
	// all the renderbuffers are attached to this (whether MSAA is enabled or not)
	glGenFramebuffers(1, &fbo);
	retainFB(fbo);

	GLint previousFboId = 0;

	// note that we are using a glGetInteger method here, which may stall the pipeline.
	// in the allocate() method, this is not that tragic since this will not be called 
	// within the draw() loop. Here, we need not optimise for performance, but for 
	// simplicity and readability .

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// No depth stencil as texture, nor depth, nor stencil

	settings.useDepth = _settings.useDepth;
	settings.useStencil = _settings.useStencil;
	settings.depthStencilInternalFormat = _settings.depthStencilInternalFormat;
	settings.depthStencilAsTexture = _settings.depthStencilAsTexture;
	settings.textureTarget = _settings.textureTarget;
	settings.wrapModeHorizontal = _settings.wrapModeHorizontal;
	settings.wrapModeVertical = _settings.wrapModeVertical;
	settings.maxFilter = _settings.maxFilter;
	settings.minFilter = _settings.minFilter;

	// Num samples should be zero
	fboTextures = fbo;
	// ^ These will always be the same since MSAA is not supported here 

	// now create all textures and color buffers
	// numColorBuffer = 1
	for (int i = 0; i < _settings.numColorbuffers; i++) createAndAttachTexture(_settings.internalformat, i);
	_settings.colorFormats = settings.colorFormats;
	settings.internalformat = _settings.internalformat;

	dirty.resize(_settings.colorFormats.size(), true); // we start with all color buffers dirty.

	// This doesn't happen below:
	// if textures are attached to a different fbo (e.g. if using MSAA) check it's status
	//if (fbo != fboTextures) {
	//	glBindFramebuffer(GL_FRAMEBUFFER, fboTextures);
	//}

	// check everything is ok with this fbo
	bIsAllocated = checkStatus();

	// restore previous framebuffer id
	glBindFramebuffer(GL_FRAMEBUFFER, previousFboId);
}

// --------------------------------------------------
void ofxFbo3d::createAndAttachTexture(GLenum internalFormat, GLenum attachmentPoint) {

	ofxTextureData3d texData;

	texData.textureTarget = settings.textureTarget;
	texData.width = settings.width;
	texData.height = settings.height;
	texData.depth = settings.depth;
	texData.glInternalFormat = internalFormat;

	// Are these supported?
	texData.bFlipTexture = false;
	texData.wrapModeHorizontal = settings.wrapModeHorizontal;
	texData.wrapModeVertical = settings.wrapModeVertical;
	texData.magFilter = settings.maxFilter;
	texData.minFilter = settings.minFilter;

	ofxTexture3d tex;
	tex.allocate(texData);

	attachTexture(tex, internalFormat, attachmentPoint);
	dirty.push_back(true);
	activeDrawBuffers.push_back(GL_COLOR_ATTACHMENT0 + attachmentPoint);
}

// --------------------------------------------------
void ofxFbo3d::attachTexture(ofxTexture3d& tex, GLenum internalFormat, GLenum attachmentPoint) {
	// bind fbo for textures (if using MSAA this is the newly created fbo, otherwise its the same fbo as before)
	GLint temp;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &temp);
	glBindFramebuffer(GL_FRAMEBUFFER, fboTextures);

	glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, tex.texData.textureTarget, tex.texData.textureID, 0, 0);
	if (attachmentPoint >= textures.size()) {
		textures.resize(attachmentPoint + 1);
	}
	textures[attachmentPoint] = tex;

	settings.colorFormats.resize(attachmentPoint + 1);
	settings.colorFormats[attachmentPoint] = internalFormat;
	settings.numColorbuffers = settings.colorFormats.size();

	// if MSAA, bind main fbo and attach renderbuffer
	// numSamples is zero
	glBindFramebuffer(GL_FRAMEBUFFER, temp);
}

// --------------------------------------------------
bool ofxFbo3d::checkStatus() const {
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		ofLogVerbose("ofxFbo3d") << "FRAMEBUFFER_COMPLETE - OK";
		return true;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		ofLogError("ofxFbo3d") << "FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		ofLogError("ofxFbo3d") << "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
		ofLogError("ofxFbo3d") << "FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
		break;
#ifndef TARGET_PROGRAMMABLE_GL
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
		ofLogError("ofxFbo3d") << "FRAMEBUFFER_INCOMPLETE_FORMATS";
		break;
#endif
	case GL_FRAMEBUFFER_UNSUPPORTED:
		ofLogError("ofxFbo3d") << "FRAMEBUFFER_UNSUPPORTED";
		break;
#ifndef TARGET_OPENGLES
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		ofLogWarning("ofxFbo3d") << "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		ofLogError("ofxFbo3d") << "FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		ofLogError("ofxFbo3d") << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
		break;
#endif
	default:
		ofLogError("ofxFbo3d") << "UNKNOWN ERROR " << status;
		break;
	}
	return false;
}

//----------------------------------------------------------
float ofxFbo3d::getWidth() const {
	return settings.width;
}

//----------------------------------------------------------
float ofxFbo3d::getHeight() const {
	return settings.height;
}

// --------------------------------------------------
float ofxFbo3d::getDepth() const {
	return settings.depth;
}

// --------------------------------------------------
bool ofxFbo3d::isAllocated() const {
	return bIsAllocated;
}

// --------------------------------------------------
void ofxFbo3d::clear() {
	if (fbo) {
		releaseFB(fbo);
		fbo = 0;
	}
	fboTextures = 0; // ?
	// numSamples = 0
	textures.clear();
	activeDrawBuffers.clear();
	bIsAllocated = false;
}

//----------------------------------------------------------
ofxTexture3d& ofxFbo3d::getTexture() {
	return getTexture(defaultTextureIndex);
}

//----------------------------------------------------------
ofxTexture3d& ofxFbo3d::getTexture(int attachmentPoint) {
	//updateTexture(attachmentPoint); // no need to call since colorbuffer 's (MSAA) aren't supported

	return textures[attachmentPoint];
}

//----------------------------------------------------------
// TODO: Not OpenGLES
//void bindForBlitting(std::weak_ptr<ofBaseGLRenderer>& _renderer, const ofxFbo3d& fboSrc, 
//	ofxFbo3d& fboDst, int attachmentPoint) {
//	
//	auto renderer = _renderer.lock();
//	if (!renderer) return;
//
//	if (renderer->currentFramebufferId == fboSrc.getId()) {
//		ofLogWarning() << "Framebuffer with id: " << fboSrc.getId() << " cannot be bound onto itself. \n" <<
//			"Most probably you forgot to end() the current framebuffer before calling getTexture().";
//		return;
//	}
//	// this method could just as well have been placed in ofBaseGLRenderer
//	// and shared over both programmable and fixed function renderer.
//	// I'm keeping it here, so that if we want to do more fancyful
//	// named framebuffers with GL 4.5+, we can have
//	// different implementations.
//	framebufferIdStack.push_back(currentFramebufferId);
//	currentFramebufferId = fboSrc.getId();
//	glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFramebufferId);
//	glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentPoint);
//	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDst.getIdDrawBuffer());
//	glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachmentPoint);
//}

// --------------------------------------------------
//void ofxFbo3d::updateTexture(int attachmentPoint) {
//	if (!bIsAllocated) return;
//#ifndef TARGET_OPENGLES
//	if (fbo != fboTextures && dirty[attachmentPoint]) {
//
//		// if fbo != fboTextures, we are dealing with an MSAA enabled FBO.
//		// and we need to blit one fbo into another to see get the texture
//		// content
//
//		if (!ofIsGLProgrammableRenderer()) {
//			// save current drawbuffer
//			glPushAttrib(GL_COLOR_BUFFER_BIT);
//		}
//
//		// Is this necessary?
//		auto renderer = settings.renderer.lock();
//		if (renderer) {
//			renderer->bindForBlitting(*this, *this, attachmentPoint);
//			glBlitFramebuffer(0, 0, settings.width, settings.height, 0, 0, settings.width, settings.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
//			renderer->unbind(*this);
//
//			glReadBuffer(GL_BACK);
//		}
//
//		if (!ofIsGLProgrammableRenderer()) {
//			// restore current drawbuffer
//			glPopAttrib();
//		}
//		dirty[attachmentPoint] = false;
//	}
//#endif
//}

// --------------------------------------------------
//void ofxFbo3d::begin(ofFboMode mode) {
//	auto renderer = settings.renderer.lock();
//	if (renderer) {
//		renderer->begin(*this, mode);
//	}
//}
