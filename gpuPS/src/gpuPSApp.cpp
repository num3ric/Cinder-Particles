#include "cinder/app/AppNative.h"
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

#include "PingPongFbo.h"

const int SIDE = 1024;

using namespace ci;
using namespace ci::app;
using namespace std;

class gpuPSApp : public AppNative {
public:
	void	prepareSettings( Settings *settings );
    void    setupPingPongFbo();
	void	setupVBO();
	void	setup();
	void	resize( );
	void	update();
	void	draw();
	void    mouseDown( MouseEvent event );
	void    mouseDrag( MouseEvent event );
	void	keyDown( KeyEvent event );
	CameraPersp		mCam;
	Arcball			mArcball;

	PingPongFbo     mPPFbo;
	gl::VboMesh		mVboMesh;
	gl::GlslProg	mParticlesShader, mDisplacementShader;
};

void gpuPSApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1000, 800 );
	//settings->setFullScreen(true);
}


void gpuPSApp::setup()
{
	gl::clear();
	try {
		// Multiple render targets shader updates the positions/velocities
		mParticlesShader = gl::GlslProg( loadResource("passThrough.vert"), loadResource("particles.frag"));
		// Vertex displacement shader
		mDisplacementShader = gl::GlslProg( loadResource( "vertexDisplacement.vert" ), loadResource( "vertexDisplacement.frag" ));
	}
	catch( ci::gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << endl;
	}
	setupPingPongFbo();
	// THE VBO HAS TO BE DRAWN AFTER FBO!
	setupVBO();
    
    //	gl::enableDepthRead();
    //	gl::enableAlphaBlending();
}

void gpuPSApp::setupPingPongFbo()
{
    // TODO: Test with more than 2 texture attachments
    mPPFbo = PingPongFbo(Vec2i(SIDE, SIDE), 2);
    // Position 2D texture array
	Surface32f initPos = Surface32f( SIDE, SIDE, true);
	Surface32f::Iter pixelIter = initPos.getIter();
	while( pixelIter.line() ) {
		while( pixelIter.pixel() ) {
			/* Initial particle positions are passed in as R,G,B
			 float values. Alpha is used as particle mass. */
			initPos.setPixel( pixelIter.getPos(), ColorAf( Rand::randFloat()-0.5f, Rand::randFloat()-0.5f, Rand::randFloat()-0.5f, Rand::randFloat(0.2f, 1.0f) ) );
		}
	}
    mPPFbo.addTexture(initPos);
	
	//Velocity 2D texture array
	Surface32f initVel = Surface32f( SIDE, SIDE, true);
	pixelIter = initVel.getIter();
	while( pixelIter.line() ) {
		while( pixelIter.pixel() ) {
			/* Initial particle velocities are
			 passed in as R,G,B float values. */
			initVel.setPixel( pixelIter.getPos(), ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
		}
	}
	mPPFbo.addTexture(initVel);
    
	mPPFbo.initializeToTextures();
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

void gpuPSApp::resize()
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
	gl::setMatricesWindow( mPPFbo.getSize(), false ); // false to prevent vertical flipping
	gl::setViewport( mPPFbo.getBounds() );
	
	mPPFbo.updateBind();
	
    mParticlesShader.bind();
	mParticlesShader.uniform( "positions", 0 );
	mParticlesShader.uniform( "velocities", 1 );
	mPPFbo.drawTextureQuad();
	mParticlesShader.unbind();
    
	mPPFbo.updateUnbind();
	mPPFbo.swap();
}

void gpuPSApp::draw()
{
	gl::setMatrices( mCam );
	gl::setViewport( getWindowBounds() );
	gl::clear( Color::black() );
	
	mPPFbo.bindTexture(0);
	mDisplacementShader.bind();
	mDisplacementShader.uniform("displacementMap", 0 );
	gl::pushModelView();
	gl::translate( Vec3f( 0.0f, 0.0f, getWindowHeight() / 2.0f ) );
	gl::rotate( mArcball.getQuat() );
	gl::draw( mVboMesh );
    gl::popModelView();
	
	mDisplacementShader.unbind();
	mPPFbo.unbindTexture();
	
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
		mPPFbo.initializeToTextures();
	}
}

CINDER_APP_NATIVE( gpuPSApp, RendererGl )