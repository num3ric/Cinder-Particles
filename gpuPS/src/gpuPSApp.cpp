#include "cinder/app/AppNative.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/MayaCamUI.h"
#include "cinder/ImageIo.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "cinder/Utilities.h"
#include "cinder/Ray.h"

#include "PingPongFbo.h"

const int SIDE = 1024;

using namespace ci;
using namespace ci::app;
using namespace std;

class gpuPSApp : public AppNative {
private:
    MayaCamUI       mMayaCam;
	PingPongFbo     mPPFbo;
	gl::VboMesh		mVboMesh;
	gl::GlslProg	mParticlesShader, mDisplacementShader;
    Vec3f mAttractor;
    Vec2f mMousePos;
    
    void setupPingPongFbo();
	void setupVBO();
    void computeAttractorPosition();
public:
	void prepareSettings( Settings *settings );
	void setup();
	void resize( );
	void update();
	void draw();
    void mouseMove( MouseEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
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
    
    CameraPersp cam;
	cam.setEyePoint( Vec3f(5.0f, 10.0f, 10.0f) );
	cam.setCenterOfInterestPoint( Vec3f(0.0f, 0.0f, 0.0f) );
	cam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mMayaCam.setCurrentCam( cam );
    //	gl::enableDepthRead();
    //	gl::enableAlphaBlending();
}

void gpuPSApp::setupPingPongFbo()
{
    float scale = 8.0f;
    // TODO: Test with more than 2 texture attachments
    mPPFbo = PingPongFbo(Vec2i(SIDE, SIDE), 2);
    // Position 2D texture array
	Surface32f initPos = Surface32f( SIDE, SIDE, true);
	Surface32f::Iter pixelIter = initPos.getIter();
	while( pixelIter.line() ) {
		while( pixelIter.pixel() ) {
			/* Initial particle positions are passed in as R,G,B
			 float values. Alpha is used as particle invMass. */
			initPos.setPixel( pixelIter.getPos(),
                             ColorAf( scale*Rand::randFloat()-0.5f,
                                      scale*Rand::randFloat()-0.5f,
                                      scale*Rand::randFloat()-0.5f,
                                      Rand::randFloat(0.2f, 1.0f) ) );
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
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

void gpuPSApp::computeAttractorPosition()
{
    // The attractor is positioned at the intersection of a ray
    // from the mouse to a plane perpendicular to the camera.
    float t = 0;
    Vec3f right, up;
    mMayaCam.getCamera().getBillboardVectors(&right, &up);
    CameraPersp cam = mMayaCam.getCamera();
	// generate a ray from the camera into our world
	float u = mMousePos.x / (float) getWindowWidth();
	float v = mMousePos.y / (float) getWindowHeight();
	// because OpenGL and Cinder use a coordinate system
	// where (0, 0) is in the LOWERleft corner, we have to flip the v-coordinate
	Ray ray = cam.generateRay(u , 1.0f - v, cam.getAspectRatio() );
    if (ray.calcPlaneIntersection(Vec3f(0.0f,0.0f,0.0f), right.cross(up), &t)) {
        mAttractor.set(ray.calcPosition(t));
    }
}

void gpuPSApp::update()
{
    computeAttractorPosition();
    
	gl::setMatricesWindow( mPPFbo.getSize(), false ); // false to prevent vertical flipping
	gl::setViewport( mPPFbo.getBounds() );
	
	mPPFbo.updateBind();
	
    mParticlesShader.bind();
	mParticlesShader.uniform( "positions", 0 );
	mParticlesShader.uniform( "velocities", 1 );
    mParticlesShader.uniform( "attractorPos", mAttractor);
	mPPFbo.drawTextureQuad();
	mParticlesShader.unbind();
    
	mPPFbo.updateUnbind();
	mPPFbo.swap();
}

void gpuPSApp::draw()
{
	gl::setMatrices( mMayaCam.getCamera() );
	gl::setViewport( getWindowBounds() );
	gl::clear( Color::black() );
    
	mPPFbo.bindTexture(0);
	mDisplacementShader.bind();
	mDisplacementShader.uniform("displacementMap", 0 );
	gl::draw( mVboMesh );
	mDisplacementShader.unbind();
	mPPFbo.unbindTexture();
	
	gl::setMatricesWindow(getWindowSize());
	gl::drawString( toString( SIDE*SIDE ) + " vertices", Vec2f(32.0f, 32.0f));
	gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(32.0f, 52.0f));
}

void gpuPSApp::mouseMove( MouseEvent event )
{
    mMousePos.set(event.getPos());
}

void gpuPSApp::mouseDown( MouseEvent event )
{
    mMayaCam.mouseDown( event.getPos() );
}

void gpuPSApp::mouseDrag( MouseEvent event )
{
    mMousePos = event.getPos();
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void gpuPSApp::keyDown( KeyEvent event ){
	if( event.getChar() == 'r' ) {
		mPPFbo.initializeToTextures();
	}
}

CINDER_APP_NATIVE( gpuPSApp, RendererGl )