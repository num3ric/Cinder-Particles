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
public:
    PingPongFbo() {};
    
    /**
     * Create a ping-pong fbo with a fixed textureSize and
     * a fixed number of texture attachments which have to be
     * added using the addTexture method.
     */
    PingPongFbo(Vec2i textureSize, int nbAttachments);
    /**
     * Add texture attachements to the ping-pong fbo.
     * @param surface Surface32f internally copied into a texture.
     */
    void addTexture(const Surface32f &surface);
    /// Render textures into both fbos.
    void initializeToTextures();
    ///Swap the two alternating fbos.
    void swap();
    /// Bind one fbo as the source, and the other as a target texture to update the texture.
    void updateBind();
    /// Unbind both the source and the target texture.
    void updateUnbind();
    /**
     * Bind the texture of the current fbo.
     * @param i Texture index.
     */
    void bindTexture(int i);
    /// Unbind the texture of the current fbo.
    void unbindTexture();
    /// Get the fbo/texture size.
    Vec2i getSize() const;
    /// Get the fbo/texture size.
    Area getBounds() const;
    /// Draw a quad the size of the fbos/textures.
    void drawTextureQuad() const;
};
