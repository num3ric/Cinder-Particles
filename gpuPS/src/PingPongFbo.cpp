//
//  SwapTexture.cpp
//  gpuPS
//
//  Created by Éric Renaud-Houde on 2013-01-06.
//
//

#include "PingPongFbo.h"
#include <assert.h>

PingPongFbo::PingPongFbo(Vec2i textureSize, int nbAttachments)
: mTextureSize(textureSize)
, mNbAttachments(nbAttachments)
, mCurrentFbo(0)
{
    int max =gl::Fbo::getMaxAttachments();
    std::cout << "Maximum supported number of texture attachments: " << max << std::endl;
    assert(mNbAttachments < max);
    
    gl::Fbo::Format format;
	format.enableDepthBuffer(false);
	format.enableColorBuffer(true, mNbAttachments);
	format.setMinFilter( GL_NEAREST );
	format.setMagFilter( GL_NEAREST );
	format.setColorInternalFormat( GL_RGBA32F_ARB );
	mFbos[0] = gl::Fbo( textureSize.x, textureSize.y, format );
	mFbos[1] = gl::Fbo( textureSize.x, textureSize.y, format );
}

void PingPongFbo::addTexture(const Surface32f &surface)
{
    assert(mTextures.size() < mNbAttachments);
    assert(surface.getSize() == mTextureSize);
    
    gl::Texture::Format format;
	format.setInternalFormat(GL_RGBA32F_ARB);
    TextureRef tex = TextureRef(new gl::Texture( surface, format));
    tex->setWrap( GL_REPEAT, GL_REPEAT );
	tex->setMinFilter( GL_NEAREST );
	tex->setMagFilter( GL_NEAREST );
	mTextures.push_back(tex);
}

void PingPongFbo::initializeToTextures()
{
	mFbos[mCurrentFbo].bindFramebuffer();
	mFbos[!mCurrentFbo].bindFramebuffer();
	
    
    gl::setMatricesWindow( getSize(), false );
    gl::setViewport( getBounds() );
    gl::clear();
    for(int i=0; i<mNbAttachments; ++i) {
        glDrawBuffer(glAttachements[i]);
        
        mTextures[i]->enableAndBind();
        gl::draw( *mTextures[i], getBounds() );
        mTextures[i]->unbind();
        mTextures[i]->disable();
    }
	
	mFbos[!mCurrentFbo].unbindFramebuffer();
	mFbos[mCurrentFbo].unbindFramebuffer();
}

void PingPongFbo::swap()
{
	mCurrentFbo = !mCurrentFbo;
}

void PingPongFbo::updateBind()
{
    mFbos[ mCurrentFbo ].bindFramebuffer();
    
	GLenum buf[mNbAttachments];
    std::copy(glAttachements, glAttachements + mNbAttachments, buf);
	glDrawBuffers(mNbAttachments, buf);
    
    for(int i=0; i<mNbAttachments; ++i) {
        mFbos[!mCurrentFbo].bindTexture(i, i);
    }
}

void PingPongFbo::updateUnbind()
{
    mFbos[ !mCurrentFbo ].unbindTexture();
	mFbos[ mCurrentFbo ].unbindFramebuffer();
}

void PingPongFbo::bindTexture(int i)
{
    mFbos[mCurrentFbo].bindTexture(i, i);
}

void PingPongFbo::unbindTexture()
{
    mFbos[mCurrentFbo].unbindTexture();
}

Vec2i PingPongFbo::getSize() const
{
    return mTextureSize;
}


Area PingPongFbo::getBounds() const
{
    return mFbos[0].getBounds();
}

void PingPongFbo::drawTextureQuad() const
{
    glBegin(GL_QUADS);
	glTexCoord2f( 0.0f, 0.0f); glVertex2f( 0.0f, 0.0f);
	glTexCoord2f( 0.0f, 1.0f); glVertex2f( 0.0f, mTextureSize.y);
	glTexCoord2f( 1.0f, 1.0f); glVertex2f( mTextureSize.x, mTextureSize.y);
	glTexCoord2f( 1.0f, 0.0f); glVertex2f( mTextureSize.x, 0.0f);
	glEnd();
}