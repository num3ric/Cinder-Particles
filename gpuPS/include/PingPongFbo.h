//
//  SwapTexture.h
//  gpuPS
//
//  Created by Éric Renaud-Houde on 2013-01-06.
//
//

#pragma once
#include "cinder/Vector.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rect.h"

#include <vector>

using namespace ci;

class PingPongFbo
{
public:
    PingPongFbo() {};
    //! Create a ping-pong fbo with n texture attachments.
    PingPongFbo( const std::vector<Surface32f>& surfaces );
    //! Rerender initial textures into both fbos.
    void reset();

    //! Bind one fbo as the source, and the other as a target texture to update the texture.
    void bindUpdate();
    //! Unbind both the source and the target texture.
    void unbindUpdate();
	
    //! Bind the texture of the current fbo.
    void bindTexture(int textureUnit);
    //! Unbind the texture of the current fbo.
    void unbindTexture();
	
    //! Get the fbo/texture size.
    Vec2i getSize() const;
    //! Get the fbo/texture size.
    Area getBounds() const;

private:
	//! Swap the two alternating fbos.
    void swap();
	
	Vec2i mTextureSize;
    int mCurrentFbo; //either 0 or 1
    
    //! Texture attachments per framebuffer.
    std::vector<gl::Texture> mTextures;
    //! GLenum texure attachments */
    std::vector<GLenum> mAttachments;
    //! Two alternating framebuffers */
    gl::Fbo	mFbos[2];
    /*!
     * Add texture attachements to the ping-pong fbo.
     * @param surface Surface32f internally copied into a texture.
     */
    void addTexture(const Surface32f &surface);
};
