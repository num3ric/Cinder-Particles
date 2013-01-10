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

using namespace ci;

//FIXME: Why can't I use unique_ptr here?
typedef std::shared_ptr<gl::Texture> TextureRef;

class PingPongFbo
{
    Vec2i mTextureSize;
    int mCurrentFbo; //either 0 or 1
    int mNbAttachments; //less than max allowed by the gpu
    
    /** Texture attachments per framebuffer. */
    std::vector<TextureRef> mTextures;
    /** Two alternating framebuffers */
    gl::Fbo	mFbos[2];
    
    GLenum glAttachements[16] = {
        GL_COLOR_ATTACHMENT0_EXT,
        GL_COLOR_ATTACHMENT1_EXT,
        GL_COLOR_ATTACHMENT2_EXT,
        GL_COLOR_ATTACHMENT3_EXT,
        GL_COLOR_ATTACHMENT4_EXT,
        GL_COLOR_ATTACHMENT5_EXT,
        GL_COLOR_ATTACHMENT6_EXT,
        GL_COLOR_ATTACHMENT7_EXT,
        GL_COLOR_ATTACHMENT8_EXT,
        GL_COLOR_ATTACHMENT9_EXT,
        GL_COLOR_ATTACHMENT10_EXT,
        GL_COLOR_ATTACHMENT11_EXT,
        GL_COLOR_ATTACHMENT12_EXT,
        GL_COLOR_ATTACHMENT13_EXT,
        GL_COLOR_ATTACHMENT14_EXT,
        GL_COLOR_ATTACHMENT15_EXT};
    /**
     * Add texture attachements to the ping-pong fbo.
     * @param surface Surface32f internally copied into a texture.
     */
    void addTexture(const Surface32f &surface);
public:
    PingPongFbo() {};
    
    /**
     * Create a ping-pong fbo with n texture attachments.
     */
    template <std::size_t n>
    PingPongFbo(const Surface32f (&surface)[n])
    : mCurrentFbo(0)
    , mNbAttachments(n)
    {
        if(n == 0) return;
        mTextureSize = surface[0].getSize();
        for(int i=0; i<n; i++) {
            addTexture(surface[i]);
        }
        
        int max =gl::Fbo::getMaxAttachments();
        std::cout << "Maximum supported number of texture attachments: " << max << std::endl;
        assert(n < max);
        
        gl::Fbo::Format format;
        format.enableDepthBuffer(false);
        format.enableColorBuffer(true, mNbAttachments);
        format.setMinFilter( GL_NEAREST );
        format.setMagFilter( GL_NEAREST );
        format.setColorInternalFormat( GL_RGBA32F_ARB );
        mFbos[0] = gl::Fbo( mTextureSize.x, mTextureSize.y, format );
        mFbos[1] = gl::Fbo( mTextureSize.x, mTextureSize.y, format );
        
        reloadTextures();
    }
    
    /// Render initial textures into both fbos.
    void reloadTextures();
    ///Swap the two alternating fbos.
    void swap();
    /// Bind one fbo as the source, and the other as a target texture to update the texture.
    void updateBind();
    /// Unbind both the source and the target texture.
    void updateUnbind();
    /**
     * Bind the texture of the current fbo.
     * @param i Texture unit.
     * @param i Texture attachment.
     */
    void bindTexture(int textureUnit, int attachment);
    /// Unbind the texture of the current fbo.
    void unbindTexture();
    /// Get the fbo/texture size.
    Vec2i getSize() const;
    /// Get the fbo/texture size.
    Area getBounds() const;
    /// Draw a quad the size of the fbos/textures.
    void drawTextureQuad() const;
};
