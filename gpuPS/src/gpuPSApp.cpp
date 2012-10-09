#include "cinder/app/AppBasic.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/ArcBall.h"
#include "cinder/Quaternion.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "cinder/Utilities.h"

#define SIDE 1024

using namespace ci;
using namespace ci::app;
using namespace std;

class gpuPSApp : public AppBasic {
public:
	void	prepareSettings( Settings *settings );
	void	setupTextures();
	void	resetFBOs();
	void	setupVBO();
	void	setup();
	void	resize( ResizeEvent event );
	void	update();
	void	draw();
	void    mouseDown( MouseEvent event );
	void    mouseDrag( MouseEvent event );
	void	keyDown( KeyEvent event );
	CameraPersp		mCam;
	Arcball			mArcball;
	Surface32f		mInitPos, mInitVel;
	int				mCurrentFBO;
	int				mOtherFBO;
	gl::Fbo			mFBO[2];
	gl::Texture		mPositions, mVelocities;
	gl::VboMesh		mVboMesh;
	gl::GlslProg	mPosShader, mDisplShader;
};

void gpuPSApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1000, 800 );
	//settings->setFullScreen(true);
}

void gpuPSApp::setupTextures(){
	// Position 2D texture array
	mInitPos = Surface32f( SIDE, SIDE, true);
	Surface32f::Iter pixelIter = mInitPos.getIter();
	while( pixelIter.line() ) {
		while( pixelIter.pixel() ) {
			/* Initial particle positions are passed in as R,G,B 
			 float values. Alpha is used as particle mass. */
			mInitPos.setPixel( pixelIter.getPos(), ColorAf( Rand::randFloat()-0.5f, Rand::randFloat()-0.5f, Rand::randFloat()-0.5f, Rand::randFloat(0.2f, 1.0f) ) );
		}
	}
	gl::Texture::Format tFormat;
	tFormat.setInternalFormat(GL_RGBA32F_ARB);
	mPositions = gl::Texture( mInitPos, tFormat);
	mPositions.setWrap( GL_REPEAT, GL_REPEAT );
	mPositions.setMinFilter( GL_NEAREST );
	mPositions.setMagFilter( GL_NEAREST );
	
	//Velocity 2D texture array
	mInitVel = Surface32f( SIDE, SIDE, true);
	pixelIter = mInitVel.getIter();
	while( pixelIter.line() ) {
		while( pixelIter.pixel() ) {
			/* Initial particle velocities are
			 passed in as R,G,B float values. */
			mInitVel.setPixel( pixelIter.getPos(), ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
		}
	}
	mVelocities = gl::Texture( mInitVel, tFormat);
	mVelocities.setWrap( GL_REPEAT, GL_REPEAT );
	mVelocities.setMinFilter( GL_NEAREST );
	mVelocities.setMagFilter( GL_NEAREST );
}

void gpuPSApp::resetFBOs(){
	mCurrentFBO = 0;
	mOtherFBO = 1;
	mFBO[0].bindFramebuffer();
	mFBO[1].bindFramebuffer();
	
	// Attachment 0 - Positions
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	gl::setMatricesWindow( mFBO[0].getSize(), false );
	gl::setViewport( mFBO[0].getBounds() );
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mPositions.enableAndBind();
	gl::draw( mPositions, mFBO[0].getBounds() );
	mPositions.unbind();
	
	// Attachment 1 - Velocities
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mVelocities.enableAndBind();
	gl::draw( mVelocities, mFBO[0].getBounds() );
	mVelocities.unbind();
	
	mFBO[1].unbindFramebuffer();
	mFBO[0].unbindFramebuffer();
	mPositions.disable();
	mVelocities.disable();
}

void gpuPSApp::setupVBO(){
	/*	A dummy VboMesh the same size as the
		texture to keep the vertices on the GPU */
	int totalVertices = SIDE * SIDE;
	vector<Vec2f> texCoords;
	vector<uint32_t> indices;
	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticNormals();
	//layout.setDynamicColorsRGBA();
	mVboMesh = gl::VboMesh( totalVertices, totalVertices, layout, GL_POINTS);
	for( int x = 0; x < SIDE; ++x ) {
		for( int y = 0; y < SIDE; ++y ) {	
			indices.push_back( x * SIDE + y );
			texCoords.push_back( Vec2f( x/(float)SIDE, y/(float)SIDE ) );
		}
	}
	mVboMesh.bufferIndices( indices );
	mVboMesh.bufferTexCoords2d( 0, texCoords );
}

void gpuPSApp::setup()
{	
	gl::clear();
	try {
		// Multiple render targets shader updates the positions/velocities
		mPosShader = gl::GlslProg( loadResource("pos.vert"), loadResource("pos.frag"));
		// Vertex displacement shader
		mDisplShader = gl::GlslProg( loadResource( "vDispl.vert" ), loadResource( "vDispl.frag" ));
	}
	catch( ci::gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << endl;
	}
	setupTextures();
	gl::Fbo::Format format;
	format.enableDepthBuffer(false);
	format.enableColorBuffer(true, 2);
	format.setMinFilter( GL_NEAREST );
	format.setMagFilter( GL_NEAREST );
	format.setColorInternalFormat( GL_RGBA32F_ARB );
	mFBO[0] = gl::Fbo( SIDE, SIDE, format );
	mFBO[1] = gl::Fbo( SIDE, SIDE, format );
	resetFBOs();
	// THE VBO HAS TO BE DRAWN AFTER FBO!
	setupVBO();
	
//	gl::enableDepthRead();
//	gl::enableAlphaBlending();
}

void gpuPSApp::resize( ResizeEvent event )
{
	mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( Vec2f( getWindowWidth() / 2.0f, getWindowHeight() / 2.0f ) );
	mArcball.setRadius( getWindowHeight() / 2.0f );
	
	mCam.lookAt( Vec3f( 0.0f, 0.0f, -450.0f ), Vec3f::zero() );
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.1f, 2000.0f );
	gl::setMatrices( mCam );
}

void gpuPSApp::update()
{	
	gl::setMatricesWindow( mFBO[0].getSize(), false ); // false to prevent vertical flipping
	gl::setViewport( mFBO[0].getBounds() );
	
	mFBO[ mCurrentFBO ].bindFramebuffer();
	
	GLenum buf[2] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
	glDrawBuffers(2, buf);
	mFBO[ mOtherFBO ].bindTexture(0, 0);
	mFBO[ mOtherFBO ].bindTexture(1, 1);
	mPosShader.bind();
	mPosShader.uniform( "posArray", 0 );
	mPosShader.uniform( "velArray", 1 );
	
	glBegin(GL_QUADS);
	glTexCoord2f( 0.0f, 0.0f); glVertex2f( 0.0f, 0.0f);
	glTexCoord2f( 0.0f, 1.0f); glVertex2f( 0.0f, SIDE);
	glTexCoord2f( 1.0f, 1.0f); glVertex2f( SIDE, SIDE);
	glTexCoord2f( 1.0f, 0.0f); glVertex2f( SIDE, 0.0f);
	glEnd();
	
	mPosShader.unbind();
	mFBO[ mOtherFBO ].unbindTexture();
	mFBO[ mCurrentFBO ].unbindFramebuffer();
	
	mCurrentFBO = ( mCurrentFBO + 1 ) % 2;
	mOtherFBO   = ( mCurrentFBO + 1 ) % 2;
}

void gpuPSApp::draw()
{
	gl::setMatrices( mCam );
	gl::setViewport( getWindowBounds() );
	gl::clear( Color::black() );
	
	mFBO[mCurrentFBO].bindTexture(0,0);
	mDisplShader.bind();
	mDisplShader.uniform("displacementMap", 0 );
	gl::pushModelView();
	gl::translate( Vec3f( 0.0f, 0.0f, getWindowHeight() / 2.0f ) );
	gl::rotate( mArcball.getQuat() );
	gl::draw( mVboMesh );
    gl::popModelView();
	
	mDisplShader.unbind();
	mFBO[mCurrentFBO].unbindTexture();
	
	gl::setMatricesWindow(getWindowSize());
	gl::drawString( toString( SIDE*SIDE ) + " vertices", Vec2f(32.0f, 32.0f));
	gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(32.0f, 52.0f));
}


void gpuPSApp::mouseDown( MouseEvent event )
{
    mArcball.mouseDown( event.getPos() );
}

void gpuPSApp::mouseDrag( MouseEvent event )
{
    mArcball.mouseDrag( event.getPos() );
}

void gpuPSApp::keyDown( KeyEvent event ){
	if( event.getChar() == 'r' ) {
		resetFBOs();
	}
}

CINDER_APP_BASIC( gpuPSApp, RendererGl )