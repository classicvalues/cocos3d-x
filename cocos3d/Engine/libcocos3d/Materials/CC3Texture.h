/*
 * Cocos3D-X 1.0.0
 * Author: Bill Hollings
 * Copyright (c) 2010-2014 The Brenwill Workshop Ltd. All rights reserved.
 * http://www.brenwill.com
 *
 * Copyright (c) 2014-2015 Jason Wang
 * http://www.cocos3dx.org/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * http://en.wikipedia.org/wiki/MIT_License
 */
#ifndef _CC3_TEXTURE_H_
#define _CC3_TEXTURE_H_

NS_COCOS3D_BEGIN
/** 
 * The root class of a class cluster representing textures.
 *
 * Since a single texture can be used by many nodes and materials, textures can be cached.
 * The application can use the class-side getTextureNamed: method to retrieve a loaded texture
 * from the cache, and the class-side addTexture: method to add a new texture to the cache.
 * See the notes of those two methods for more details.
 * 
 * When creating an instance, several of the class-side texture... family of methods
 * (particularly those loading from files) automatically check the cache for an existing
 * instance, based on the filename, and will use that cached instance instead of loading
 * the file again. If the texture is not in the cache, these methods will load it and place
 * it in the cache automatically. These methods can therefore be invoked repeatedly without
 * having to be concerned whether multiple copies of the same texture content will be loaded.
 * Check the notes for the creation methods to verify which methods make use of the cache.
 *
 * CC3Texture is the root of a class cluster organized for loading different texture types,
 * for both 2D and cube textures. Use the creation and initialization methods from this root
 * CC3Texture class. The initializer will ensure that the correct subclass for the texture
 * type, and in some cases, the texture file type, is created and returned. Because of this
 * class-cluster structure, be aware that the class of the instance returned by an instance
 * creation or initialization method may be different than the receiver of that method.
 *
 * There is one exception to this paradigm. Under fixed-pipeline rendering, such as in 
 * OpenGL ES 1.1 under iOS, or OpenGL without shaders under OSX, multi-texturing is handled
 * using configurable texture units. In order to assign a texture unit to a CC3Texture, you
 * must directly instatiate an instance of CC3TextureUnitTexture, and then assign a texture
 * unit to it, instead of letting the CC3Texture creation and initialization methods handle it.
 *
 * To improve both performance and texture quality, by default, instances whose width and height
 * are a power-of-two (see the isPOT property) automatically generate a mipmap when a texture is
 * loaded. If you do not want mipmaps to be generated automatically, set the class-side
 * shouldGenerateMipmaps property to NO. With automatic mipmap generation turned off, you can
 * selectively generate a mipmap on any single CC3Texture instance by using the generateMipmap
 * method. In addition, textures that contain mipmaps within the file content (PVR files may contain
 * mipmaps) will retain and use this mipmap. See the shouldGenerateMipmaps and hasMipmap properties,
 * and the generateMipmap method for more information.
 *
 * Under iOS and OSX, most texture formats are loaded updside-down. This is because the vertical
 * axis of the coordinate system of OpenGL is inverted relative to the iOS or OSX view coordinate
 * system. Subclasses that may loaded upside-down can be configured to automatically flip the texture
 * right-way up during loading. In addition, the isFlippedVerically property indicates whether the
 * texture is upside down. This can be used to ensure that textures are displayed with the correct
 * orientation. When a CC3Texture is applied to a mesh, the mesh will be adjusted automatically if
 * the texture is upside down.
 *
 * When building for iOS, raw PNG and TGA images are pre-processed by Xcode to pre-multiply alpha,
 * and to reorder the pixel component byte order, to optimize the image for the iOS platform.
 * If you want to avoid this pre-processing for PNG or TGA files, for textures such as normal maps
 * or lighting maps, that you don't want to be modified, you can prepend a 'p' to the file
 * extension ("ppng" or "ptga") to cause Xcode to skip this pre-processing and to use a loader
 * that does not pre-multiply the alpha. You can also use this for other file types as well.
 * See the notes for the CC3STBImage useForFileExtensions class-side property for more info.
 */

class CC3CCTexture;
class CC3Texture : public CC3Identifiable 
{
	DECLARE_SUPER( CC3Identifiable ); 
public:
	CC3Texture();
	virtual ~CC3Texture();

	/** The texture ID used to identify this texture to the GL engine. */
	virtual GLuint			getTextureID();

	/**
	 * If the GL texture is also tracked by a CCTexture, the CCTexture will delete the GL texture
	 * when it is deallocated, but we must tell the 3D state engine to stop tracking this texture.
	 * Otherwise, if no CCTexture is tracking the GL texture, delete it from the GL engine now.
	 */
	virtual void			deleteGLTexture();
	virtual void			ensureGLTexture();

	/** If the texture has been created, set its debug label as well. */
	virtual void			setName( const std::string& name );

	/** The size of this texture in pixels. */
	virtual CC3IntSize		getSize();

	/** Returns whether the width of this texture is a power-of-two. */
	bool					isPOTWidth();

	/** Returns whether the height of this texture is a power-of-two. */
	bool					isPOTHeight();

	/** Returns whether both the width and the height of this texture is a power-of-two. */
	bool					isPOT();

	/** Returns whether this texture is a standard two-dimentional texture. */
	virtual bool			isTexture2D();

	/** Returns whether this texture is a six-sided cube-map texture. */
	virtual bool			isTextureCube();

	/**
	 * Returns the proportional size of the usable image in the texture, relative to its physical size.
	 *
	 * Depending on the environment, the physical size of textures may be some power-of-two (POT), even
	 * when the texture dimensions are not. In this case, the usable image size is the actual portion
	 * of it that contains the image. This property contains two fractional floats (width & height),
	 * each between zero and one, representing the proportional size of the usable image
	 *
	 * As an example, an image whose dimensions are actually 320 x 480 pixels may be loaded into a
	 * texture that is 512 x 512 pixels. In that case, the value returned by this property will be
	 * {0.625, 0.9375}, as calculated from {320/512, 480/512}.
	 */
	virtual CCSize			getCoverage();

	/** 
	 * Returns the pixel format of the texture.
	 *
	 * The returned value may be one of the following:
	 *   - GL_RGBA
	 *   - GL_RGB
	 *   - GL_ALPHA
	 *   - GL_LUMINANCE
	 *   - GL_LUMINANCE_ALPHA
	 *   - GL_DEPTH_COMPONENT
	 *   - GL_DEPTH_STENCIL
	 */
	virtual GLenum			getPixelFormat();

	/** 
	 * Returns the pixel data type.
	 *
	 * Possible values depend on the value of the pixelFormat property as follows:
	 *
	 *   pixelFormat                pixelType
	 *   -----------                ---------
	 *   GL_RGBA                    GL_UNSIGNED_BYTE
	 *                              GL_UNSIGNED_SHORT_4_4_4_4
	 *                              GL_UNSIGNED_SHORT_5_5_5_1
	 *   GL_RGB                     GL_UNSIGNED_BYTE
	 *                              GL_UNSIGNED_SHORT_5_6_5
	 *   GL_ALPHA                   GL_UNSIGNED_BYTE
	 *   GL_LUMINANCE               GL_UNSIGNED_BYTE
	 *   GL_LUMINANCE_ALPHA         GL_UNSIGNED_BYTE
	 *   GL_DEPTH_COMPONENT         GL_UNSIGNED_SHORT
	 *                              GL_UNSIGNED_INT
	 *   GL_DEPTH_STENCIL           GL_UNSIGNED_INT_24_8
	 */
	virtual GLenum			getPixelType();

	/**
	 * Indicates whether this texture has an alpha channel, representing opacity.
	 *
	 * The value of this property is determined from the contents of the texture file,
	 * but you can set this property directly to override the value determined from the file.
	 */
	virtual bool			hasAlpha();
	virtual void			setHasAlpha( bool hasAlpha );

	/**
	 * Indicates whether the alpha channel of this texture has already been multiplied
	 * into each of the RGB color channels.
	 *
	 * The value of this property is determined from the contents of the texture file,
	 * but you can set this property directly to override the value determined from the file.
	 */
	virtual bool			hasPremultipliedAlpha();
	virtual void			setHasPremultipliedAlpha( bool hasAlpha );

	/**
	 * Indicates whether this texture is flipped upside-down.
	 *
	 * The vertical axis of the coordinate system of OpenGL is inverted relative to the
	 * CoreGraphics view coordinate system. As a result, some texture file formats may be
	 * loaded upside down. Most common file formats, including JPG, PNG & PVR are loaded
	 * right-way up, but using proprietary texture formats developed for other platforms
	 * may result in textures being loaded upside-down.
	 *
	 * The value of this property is determined from the contents of the texture file, but
	 * you can set this property directly to override the value determined from the file.
	 */
	virtual bool			isUpsideDown();
	virtual void			setIsUpsideDown( bool isUpsideDown );

	/**
	 * Returns the GL target of this texture.
	 *
	 * Returns GL_TEXTURE_2D if this is a 2D texture, or GL_TEXTURE_CUBE_MAP
	 * if this is a cube map texture.
	 */
	virtual GLenum			getTextureTarget();

	/**
	 * Returns the GL face to use when initially attaching this texture to a framebuffer.
	 *
	 * Returns GL_TEXTURE_2D if this is a 2D texture, or GL_TEXTURE_CUBE_MAP_POSITIVE_X 
	 * if this is a cube map texture.
	 */
	virtual GLenum			getInitialAttachmentFace();

	/**
	 * When using multiple textures with fixed-pipeline rendering, as in OpenGL ES 1.1, 
	 * textures are combined using environmental settings applied via a texture unit.
	 *
	 * When using OpenGL ES 2.0, or OpenGL on OSX, texture units are not typically used,
	 * but in some circumstances can be used to carry certain additional configuration
	 * information for the texture.
	 *
	 * In this implementation, setting this property has no effect, and reading this property
	 * will always return nil. Subclasses, such as CC3TextureUnitTexture, will override to
	 * make use of this property. When making use of texture units, be sure to instantiate
	 * an instance of a subclass that supports texture units, such as CC3TextureUnitTexture.
	 */
	virtual CC3TextureUnit*	getTextureUnit();
	virtual void			setTextureUnit( CC3TextureUnit* unit );

	/**
	 * The direction, in local node coordinates, of the light source that is to interact
	 * with this texture if the texture has been configured as an object-space bump-map.
	 *
	 * Object-space bump-maps are textures that store a normal vector (XYZ coordinates), in
	 * object-space coordinates, in the RGB components of each texture pixel, instead of color
	 * information. These per-pixel normals interact with the value of this lightDirection
	 * property (through a dot-product), to determine the luminance of the pixel. 
	 *
	 * Object-space bump-maps are used primarily with multi-texturing in a fixed-pipeline
	 * rendering environment such as OpenGL ES 1.1. Bump-maps in a programmable-pipeline,
	 * such as OpenGL ES 2.0, more commonly use tangent-space normal mapping, which does
	 * not make use of this property.
	 *
	 * Most textures ignore this property. In this implementation, setting this property
	 * has no effect, and reading this property always returns kCC3VectorZero.
	 *
	 * Subclasses, such as CC3TextureUnitTexture may override to make use of this property.
	 */
	virtual CC3Vector		getLightDirection();
	virtual void			setLightDirection( const CC3Vector& direction );

	/**
	 * Returns whether this texture is configured as an object-space bump-map.
	 *
	 * Returns NO. Subclasses, such as CC3TextureUnitTexture may override.
	 */
	virtual bool			isBumpMap();

	/**
	 * Some texture types wrap a base internal texture. This property returns that wrapped texture,
	 * or, if this instance does not wrap another texture, this property returns this instance.
	 *
	 * This property provides polymorphic compatibility with CC3Texture subclasses, notably
	 * CC3TextureUnitTexture, that contain another, underlying texture.
	 */
	virtual CC3Texture*		getTexture();

	/**
	 * Indicates whether this instance will flip the texture vertically during loading, in order
	 * to ensure that the texture is oriented upside up.
	 *
	 * Under iOS and OSX, most textures are loaded into memory upside-down because of the
	 * difference in vertical orientation between the OpenGL and CoreGraphics coordinate systems.
	 *
	 * If this property is set to YES during loading, and the texture has been loaded upside down,
	 * the texture will be flipped in memory so that it is oriented the right way up.
	 *
	 * If this property is set to NO during loading, and the texture has been loaded upside up,
	 * the texture will be flipped in memory so that it is oriented upside down.
	 *
	 * It is possible to compensate for an upside-down using texture coordinates. You can set
	 * this property to NO prior to loading in order to leave the texture upside-down and use
	 * texture coordinates to compensate.
	 *
	 * The initial value of this property is set to the value of the class-side
	 * defaultShouldFlipVerticallyOnLoad property.
	 */
	virtual bool			shouldFlipVerticallyOnLoad();
	virtual void			setShouldFlipVerticallyOnLoad( bool filp );

	/**
	 * Indicates whether this instance will flip the texture horizontally during loading.
	 *
	 * Some types of textures (notably cube-map textures) are stored in GL memory horizontally flipped.
	 *
	 * If this property is set to YES during loading, the texture will be flipped horizontally in memory.
	 *
	 * The initial value of this property is set to the value of the class-side
	 * defaultShouldFlipHorizontallyOnLoad property.
	 */
	virtual bool			shouldFlipHorizontallyOnLoad();
	virtual void			setShouldFlipHorizontallyOnLoad( bool flip );

	/**
	 * This class-side property determines the initial value of the shouldFlipVerticallyOnLoad
	 * for instances of this class.
	 *
	 * Each subclass can have a different value for this class-side property. See the notes for
	 * this property on each subclass to understand the initial value.
	 */
	static bool				defaultShouldFlipVerticallyOnLoad();

	/**
	 * This class-side property determines the initial value of the shouldFlipVerticallyOnLoad
	 * for instances of this class.
	 *
	 * Each subclass can have a different value for this class-side property. See the notes for
	 * this property on each subclass to understand the initial value.
	 */
	static void				setDefaultShouldFlipVerticallyOnLoad( bool shouldFlip );

	/**
	 * This class-side property determines the initial value of the shouldFlipHorizontallyOnLoad
	 * for instances of this class.
	 *
	 * Each subclass can have a different value for this class-side property. See the notes for
	 * this property on each subclass to understand the initial value.
	 */
	static bool				defaultShouldFlipHorizontallyOnLoad();

	/**
	 * This class-side property determines the initial value of the shouldFlipVerticallyOnLoad
	 * for instances of this class.
	 *
	 * Each subclass can have a different value for this class-side property. See the notes for
	 * this property on each subclass to understand the initial value.
	 */
	static void				setDefaultShouldFlipHorizontallyOnLoad( bool shouldFlip );

	virtual void			bindTextureContent( CC3CCTexture* texContent, GLenum target );
	virtual GLuint			getByteAlignment();


	/**
	 * Returns whether a mipmap has been generated for this texture.
	 *
	 * If the class-side shouldGenerateMipmaps property is YES, mipmaps are generated 
	 * automatically after the texture data has been loaded.
	 *
	 * Mipmaps can also be generated manually by invoking the generateMipmap method.
	 */
	virtual bool			hasMipmap();

	/**
	 * Generates a mipmap for this texture, if needed.
	 *
	 * It is safe to invoke this method more than once, because it will only generate
	 * a mipmap if it does not yet exist.
	 *
	 * Mipmaps can only be generated for textures whose width and height are are a power-of-two
	 * (see the isPOT property).
	 */
	virtual void			generateMipmap();

	/**
	 * Returns whether a mipmap should be generated automatically for each instance when
	 * the texture is loaded.
	 *
	 * If this property is set to YES, mipmap will only be generated if the texture
	 * file does not already contain a mipmap.
	 *
	 * The value of this property affects all textures loaded while that value is in effect.
	 * You can set this property to the desired value prior to loading one or more textures.
	 *
	 * The default value of this class-side property is YES, indicating that mipmaps
	 * will be generated for any texture loaded whose dimensions are a power-of-two.
	 */
	static bool				shouldGenerateMipmaps();

	/**
	 * Sets whether a mipmap should be generated automatically for each instance when
	 * the texture is loaded.
	 *
	 * If this property is set to YES, mipmap will only be generated if the texture
	 * file does not already contain a mipmap.
	 *
	 * The value of this property affects all textures loaded while that value is in effect.
	 * You can set this property to the desired value prior to loading one or more textures.
	 *
	 * The default value of this class-side property is YES, indicating that mipmaps
	 * will be generated for any texture loaded whose dimensions are a power-of-two.
	 */
	static void				setShouldGenerateMipmaps( bool shouldMipmap );

	/**
	 * The minifying function to be used whenever a pixel being textured maps
	 * to an area greater than one texel.
	 *
	 * This property must be one of the following values:
	 *   - GL_NEAREST:                Uses the texel nearest to the center of the pixel.
	 *   - GL_LINEAR:                 Uses a weighted average of the four closest texels.
	 *   - GL_NEAREST_MIPMAP_NEAREST: Uses GL_NEAREST on the mipmap that is closest in size.
	 *   - GL_LINEAR_MIPMAP_NEAREST:  Uses GL_LINEAR on the mipmap that is closest in size.
	 *   - GL_NEAREST_MIPMAP_LINEAR:  Uses GL_NEAREST on the two mipmaps that are closest in size,
	 *                                then uses the weighted average of the two results.
	 *   - GL_LINEAR_MIPMAP_LINEAR:   Uses GL_LINEAR on the two mipmaps that are closest in size,
	 *                                then uses the weighted average of the two results.
	 *
	 * The last four values above require that a mipmap be available, as indicated by the hasMipmap
	 * property. If one of those values is set in this property, this property will only return either
	 * GL_NEAREST (for all GL_NEAREST... values) or GL_LINEAR (for all GL_LINEAR... values) until a
	 * mipmap has been created. See the hasMipmap property for more information about mipmaps.
	 *
	 * The initial value of this property is set by the defaultTextureParameters class-side property,
	 * and defaults to GL_LINEAR_MIPMAP_NEAREST, or GL_LINEAR if the texture does not have a mipmap.
	 */
	virtual GLenum			getMinifyingFunction();
	virtual void			setMinifyingFunction( GLenum function );

	/**
	 * The magnifying function to be used whenever a pixel being textured maps
	 * to an area less than or equal to one texel.
	 *
	 * This property must be one of the following values:
	 *   - GL_NEAREST: Uses the texel nearest to the center of the pixel.
	 *   - GL_LINEAR:  Uses a weighted average of the four closest texels.
	 *
	 * The initial value of this property is set by the defaultTextureParameters
	 * class-side property, and defaults to GL_LINEAR.
	 */
	virtual GLenum			getMagnifyingFunction();
	virtual void			setMagnifyingFunction( GLenum function );

	/**
	 * The method used to detemine the texel to use when a texture coordinate has
	 * a value less than zero or greater than one in the horizontal (S) direction.
	 *
	 * This property must be one of the following values:
	 *   - GL_CLAMP_TO_EDGE:   Uses the nearest texel from the nearest edge, effectively
	 *                         extending this texel across the mesh.
	 *   - GL_REPEAT:          Repeats the texture across the mesh.
	 *   - GL_MIRRORED_REPEAT: Repeats the texture across the mesh, altering between
	 *                         the texture and a mirror-image of the texture.
	 *
	 * The values GL_REPEAT and GL_MIRRORED_REPEAT can only be set if the isPOT property returns
	 * YES, indicating that both width and height dimensions of this texture are a power-of-two.
	 * Otherwise, this property will always return GL_CLAMP_TO_EDGE.
	 *
	 * This property must be set to GL_CLAMP_TO_EDGE when using this texture as a rendering target
	 * as an attachment to a rendering surface such as a framebuffer ("render-to-texture").
	 *
	 * The initial value of this property is set by the defaultTextureParameters class-side
	 * property, and will be GL_REPEAT if the dimensions of this texture are a power-of-two,
	 * or GL_CLAMP_TO_EDGE if not.
	 */
	virtual GLenum			getHorizontalWrappingFunction();
	virtual void			setHorizontalWrappingFunction( GLenum function );

	/**
	 * The method used to detemine the texel to use when a texture coordinate has
	 * a value less than zero or greater than one in the vertical (T) direction.
	 *
	 * This property must be one of the following values:
	 *   - GL_CLAMP_TO_EDGE:   Uses the nearest texel from the nearest edge, effectively
	 *                         extending this texel across the mesh.
	 *   - GL_REPEAT:          Repeats the texture across the mesh.
	 *   - GL_MIRRORED_REPEAT: Repeats the texture across the mesh, altering between
	 *                         the texture and a mirror-image of the texture.
	 *
	 * The values GL_REPEAT and GL_MIRRORED_REPEAT can only be set if the isPOT property returns
	 * YES, indicating that both width and height dimensions of this texture are a power-of-two.
	 * Otherwise, this property will always return GL_CLAMP_TO_EDGE.
	 *
	 * This property must be set to GL_CLAMP_TO_EDGE when using this texture as a rendering target
	 * as an attachment to a rendering surface such as a framebuffer ("render-to-texture").
	 *
	 * The initial value of this property is set by the defaultTextureParameters class-side
	 * property, and will be GL_REPEAT if the dimensions of this texture are a power-of-two,
	 * or GL_CLAMP_TO_EDGE if not.
	 */
	virtual GLenum			getVerticalWrappingFunction();
	virtual void			setVerticalWrappingFunction( GLenum function );

	/**
	 * A convenience method to accessing the following four texture parameter properties
	 * using a Cocos2D ccTexParams structure:
	 *   - minifyingFunction
	 *   - magnifyingFunction
	 *   - horizontalWrappingFunction
	 *   - verticalWrappingFunction
	 *
	 * The value of each component of this structure will be the same as the corresponding
	 * property on this instance. See the notes for each of those properties for an indication
	 * of the initial values for each of those properties.
	 */
	virtual ccTexParams		getTextureParameters();
	virtual void			setTextureParameters( const ccTexParams& parameters );

	/**
	 * The default values for the textureParameters property 
	 * (with the initial values of this class-side property):
	 *   - minifyingFunction (GL_LINEAR_MIPMAP_NEAREST)
	 *   - magnifyingFunction (GL_LINEAR)
	 *   - horizontalWrappingFunction (GL_REPEAT)
	 *   - verticalWrappingFunction (GL_REPEAT)
	 */
	static ccTexParams		defaultTextureParameters();

	/**
	 * The default values for the textureParameters property
	 * (with the initial values of this class-side property):
	 *   - minifyingFunction (GL_LINEAR_MIPMAP_NEAREST)
	 *   - magnifyingFunction (GL_LINEAR)
	 *   - horizontalWrappingFunction (GL_REPEAT)
	 *   - verticalWrappingFunction (GL_REPEAT)
	 *
	 * You can change the value of this class-side property to affect
	 * any textures subsequently created or loaded from a file.
	 */
	static void				setDefaultTextureParameters( const ccTexParams& texParams );

	/**
	 * Binds this texture to the GL engine.
	 *
	 * If any of the texture parameter properties have been changed since the last time this
	 * texture was bound, they are updated in the GL engine at this time.
	 */
	virtual void			drawWithVisitor( CC3NodeDrawingVisitor* visitor );

	/** Returns the GLSL uniform texture sampler semantic for this type of texture. */
	virtual GLenum			getSamplerSemantic();

	/**
	 * Replaces a portion of the content of this texture by writing the specified array of pixels
	 * into the specified rectangular area within the specified target for this texture, The specified
	 * content replaces the texture data within the specified rectangle. The specified content array
	 * must be large enough to contain content for the number of pixels in the specified rectangle.
	 *
	 * If this is a standard 2D texture, the target must be GL_TEXTURE_2D. If this is a cube-map
	 * texture, the specified target can be one of the following:
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_X
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_X
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	 *
	 * Content is read from the specified array left to right across each row of pixels within
	 * the specified image rectangle, starting at the row at the bottom of the rectangle, and
	 * ending at the row at the top of the rectangle.
	 *
	 * Within the specified array, the pixel content should be packed tightly, with no gaps left
	 * at the end of each row. The last pixel of one row should immediately be followed by the
	 * first pixel of the next row.
	 *
	 * The pixels in the specified array are in standard 32-bit RGBA. If the pixelFormat and
	 * pixelType properties of this texture are not GL_RGBA and GL_UNSIGNED_BYTE, respectively,
	 * the pixels in the specified array will be converted to the format and type of this texture
	 * before being inserted into the texture. Be aware that this conversion will reduce the
	 * performance of this method. For maximum performance, match the format and type of this
	 * texture to the 32-bit RGBA format of the specified array, by setting the pixelFormat
	 * property to GL_RGBA and the pixelType property to GL_UNSIGNED_BYTE. However, keep in mind
	 * that the 32-bit RGBA format consumes more memory than most other formats, so if performance
	 * is of lesser concern, you may choose to minimize the memory requirements of this texture
	 * by setting the pixelFormat and pixelType properties to values that consume less memory.
	 *
	 * If this texture has mipmaps, they are not automatically updated. Once all desired content
	 * has been replaced, invoke the generateMipmap method to regenerate the mipmaps.
	 */
	virtual void			replacePixels( const CC3Viewport& rect, GLenum target, ccColor4B* colorArray );

	/** Resizes this texture to the specified dimensions and clears all texture content. */
	virtual void			resizeTo( const CC3IntSize& size );

	/**
	 * Returns an empty content of the same size as this texture. If this texture already has a
	 * content object already, it is resized and returned. Otherwise, a new content object, of
	 * the size, pixel format and type of this texture is created an returned.
	 */
	virtual CC3CCTexture*	getSizedContent();

	/** 
	 * Returns a Cocos2D-compatible 2D texture, that references the same GL texture.
	 *
	 * The value of the class-side shouldCacheAssociatedCCTextures property determines whether
	 * the CCTexture returned by this method will automatically be added to the CCTextureCache.
	 *
	 * With the class-side shouldCacheAssociatedCCTextures property set to NO, you can still 
	 * add any CCTexture retrieved from this property to the CCTextureCache using the 
	 * CCTextureCache addTexture:named: method.
	 *
	 * Although a CCTexture can be retrieved for any type of CC3Texture, including cube-maps,
	 * using a cube-mapped texture as a Cocos2D texture may lead to unexpected behavour.
	 */
	virtual CC3CCTexture*	getCCTexture();
	virtual void			setCCTexture( CC3CCTexture* texture );

	/**
	 * Indicates whether the associated Cocos2D CCTexture, available through the ccTexture 
	 * property, should be automatically added to the Cocos2D CCTextureCache.
	 *
	 * The initial value of this property is NO. If you intend to share many of the same textures
	 * between Cocos3D and Cocos2D objects, you may want to set this property to YES.
	 *
	 * With this property set to NO, you can still add any CCTexture retrieved from the ccTexture
	 * property to the CCTextureCache using the CCTexture addToCacheWithName: method.
	 */
	static bool				shouldCacheAssociatedCCTextures();

	/**
	 * Indicates whether the associated Cocos2D CCTexture, available through the ccTexture
	 * property, should be automatically added to the Cocos2D CCTextureCache.
	 *
	 * The initial value of this property is NO. If you intend to share many of the same textures
	 * between Cocos3D and Cocos2D objects, you may want to set this property to YES.
	 *
	 * With this property set to NO, you can still add any CCTexture retrieved from the ccTexture
	 * property to the CCTextureCache using the CCTexture addToCacheWithName: method.
	 */
	static void				setShouldCacheAssociatedCCTextures( bool shouldCache );

	/**
	 * Returns an instance initialized by loading the single texture file at the specified file path.
	 *
	 * The specified file path may be either an absolute path, or a path relative to the
	 * application resource directory. If the file is located directly in the application
	 * resources directory, the specified file path can simply be the name of the file.
	 *
	 * The name of this instance is set to the unqualified file name from the specified
	 * file path and the tag is set to an automatically generated unique tag value.
	 *
	 * This method can be used to load a single standard 2D texture. It can also be used to load
	 * cube-map textures contained within a single PVR texture file.
	 *
	 * This method cannot be used to load cube-maps that require more than one file to be loaded.
	 *
	 * CC3Texture is the root of a class cluster for loading different file types. Depending on the
	 * file type of the specified file, this method may return an instance of a class that is different
	 * than the class of the receiver. You can use the textureClassForFile: method to determine the 
	 * cluster subclass whose instance will be returned by this method for the specified file.
	 *
	 * Normally, you should use the textureFromFile: method to reuse any cached instance instead of
	 * creating and loading a new instance. The textureFromFile: method automatically invokes this
	 * method if an instance does not exist in the texture cache, in order to create and load the
	 * texture from the file, and after doing so, places the newly loaded instance into the cache.
	 *
	 * However, by invoking the alloc method and then invoking this method directly, the application
	 * can load the texture without first checking the texture cache. The texture can then be placed
	 * in the cache using the addTexture: method. If you load two separate textures from the same
	 * file, be sure to set a distinct name for each before adding each to the cache.
	 *
	 * If the class-side shouldGenerateMipmaps property is set to YES, and the texture file does
	 * not already contain a mipmap, a mipmap will be generated for the texture automatically.
	 *
	 * Returns nil if the file could not be loaded.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initFromFile( const std::string& filePath );

	/**
	 * Returns an instance initialized by loading the single texture file at the specified file path.
	 *
	 * The specified file path may be either an absolute path, or a path relative to the
	 * application resource directory. If the file is located directly in the application
	 * resources directory, the specified file path can simply be the name of the file.
	 *
	 * The name of this instance is set to the unqualified file name from the specified
	 * file path and the tag is set to an automatically generated unique tag value.
	 *
	 * This method can be used to load a single standard 2D texture. It can also be used to load
	 * cube-map textures contained within a single PVR texture file.
	 *
	 * This method cannot be used to load cube-maps that require more than one file to be loaded.
	 *
	 * CC3Texture is the root of a class cluster for loading different file types. Depending on the
	 * file type of the specified file, this method may return an instance of a class that is different
	 * than the receiver. You can use the textureClassForFile: method to determine the cluster subclass
	 * whose instance will be returned by this method for the specified file.
	 *
	 * Textures loaded through this method are cached. If the texture was already loaded and is in
	 * the cache, it is retrieved and returned. If the texture has not in the cache, it is loaded
	 * from the specified file, placed into the cache, and returned. It is therefore safe to invoke
	 * this method any time the texture is needed, without having to worry that the texture will
	 * be repeatedly loaded from file.
	 *
	 * To clear a texture instance from the cache, use the removeTexture: method.
	 *
	 * To load the file directly, bypassing the cache, use the alloc and initFromFile: methods.
	 * This technique can be used to load the same texture twice, if needed for some reason.
	 * Each distinct instance can then be given its own name, and added to the cache separately.
	 * However, when choosing to do so, be aware that textures often consume significant memory.
	 *
	 * If the class-side shouldGenerateMipmaps property is set to YES, and the texture file does
	 * not already contain a mipmap, a mipmap will be generated for the texture automatically.
	 *
	 * Returns nil if the texture is not in the cache and could not be loaded.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be an instance of a
	 * different class than the receiver.
	 */
	static CC3Texture*		textureFromFile( const char* filePath );

	/**
	 * Initializes this instance from the specified texture properties, without providing content.
	 *
	 * Once initialized, the texture will be bound to the GL engine when the resizeTo: method is
	 * invoked, providing the texture with a size.
	 *
	 * See the notes for the pixelFormat and pixelType properties for the range of values permitted
	 * for the corresponding format and type parameters here.
	 *
	 * The name property of this instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initWithPixelFormat( GLenum format, GLenum pixelType );
	virtual bool			initWithPixelFormat( GLenum pixelType );

	/**
	 * Allocates and initializes an autoreleased instance from the specified texture properties,
	 * without providing content.
	 *
	 * Once initialized, the texture will be bound to the GL engine when the resizeTo: method is
	 * invoked, providing the texture with a size.
	 *
	 * See the notes for the pixelFormat and pixelType properties for the range of values permitted
	 * for the corresponding format and type parameters here.
	 *
	 * The name property of the instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be an instance of a
	 * different class than the receiver.
	 */
	static CC3Texture*		textureWithPixelFormat( GLenum format, GLenum pixelType );

	/**
	 * Initializes this instance from the specified texture properties, without providing content.
	 *
	 * Once initialized, the texture will be bound to the GL engine, with space allocated for a
	 * texture of the specified size and pixel content. Content can be added later by using this
	 * texture as a rendering surface.
	 *
	 * See the notes for the pixelFormat and pixelType properties for the range of values permitted
	 * for the corresponding format and type parameters here.
	 *
	 * The name property of this instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initWithSize( const CC3IntSize& size, GLenum format, GLenum pixelType );

	/**
	 * Allocates and initializes an autoreleased instance from the specified texture properties,
	 * without providing content.
	 *
	 * Once initialized, the texture will be bound to the GL engine, with space allocated for a
	 * texture of the specified size and pixel content. Content can be added later by using this
	 * texture as a rendering surface.
	 *
	 * See the notes for the pixelFormat and pixelType properties for the range of values permitted
	 * for the corresponding format and type parameters here.
	 *
	 * The name property of the instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be an instance of a
	 * different class than the receiver.
	 */
	static CC3Texture*		textureWithSize( const CC3IntSize& size, GLenum format, GLenum pixelType );

	/**
	 * Initializes this instance containing pixel content of the specified size and solid, 
	 * uniform color. This method can be useful for creating a test texture.
	 *
	 * Since the texture is just a solid color, a mipmap is not created.
	 *
	 * The name property of this instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initWithSize( const CC3IntSize& size, const ccColor4B& color );

	/**
	 * Allocates and initializes an autoreleased instance containing pixel content of the specified
	 * size and solid, uniform color. This method can be useful for creating a test texture.
	 *
	 * Since the texture is just a solid color, a mipmap is not created.
	 *
	 * The name property of this instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	static CC3Texture*		textureWithSize( const CC3IntSize& size, const ccColor4B& color );

	/**
	 * Initializes this instance from the specified Cocos2D CCTexture.
	 *
	 * This instance will use the same GL texture object as the specified CCTexture. The specified
	 * CCTexture can be retrieved from this instance using the ccTexture property.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initWithCCTexture( CC3CCTexture* ccTexture );

	/**
	 * Allocates and initializes an instance from the specified Cocos2D CCTexture.
	 *
	 * The instance will use the same GL texture object as the specified CCTexture. The specified
	 * CCTexture can be retrieved from this instance using the ccTexture property.
	 *
	 * The name property of the instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be an instance of a
	 * different class than the receiver.
	 */
	static CC3Texture*		textureWithCCTexture( CC3CCTexture* ccTexture );

	/**
	 * Initializes this instance by loading the six cube face textures at the specified file paths,
	 * and returns whether all six files were successfully loaded.
	 *
	 * Each of the specified file paths may be either an absolute path, or a path relative to
	 * the application resource directory. If the file is located directly in the application
	 * resources directory, the corresponding file path can simply be the name of the file.
	 *
	 * The name of this instance is set to the unqualified file name of the specified posXFilePath file path.
	 *
	 * If the class-side shouldGenerateMipmaps property is set to YES, a mipmap will be generated
	 * for the texture automatically.
	 *
	 * Returns nil if any of the six files could not be loaded.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initCubeFromFiles( const std::string& posXFilePath, const std::string& negXFilePath, 
		const std::string& posYFilePath, const std::string& negYFilePath, 
		const std::string& posZFilePath, const std::string& negZFilePath );

	/**
	 * Returns an instance initialized by loading the six cube face textures at the specified
	 * file paths, and returns whether all six files were successfully loaded.
	 *
	 * Each of the specified file paths may be either an absolute path, or a path relative to
	 * the application resource directory. If the file is located directly in the application
	 * resources directory, the corresponding file path can simply be the name of the file.
	 *
	 * If the class-side shouldGenerateMipmaps property is set to YES, a mipmap will be generated
	 * for the texture automatically.
	 *
	 * The name of this instance is set to the unqualified file name of the specified posXFilePath file path.
	 *
	 * Textures loaded through this method are cached. If the texture was already loaded and is in
	 * the cache, it is retrieved and returned. If the texture has not in the cache, it is loaded,
	 * placed into the cache, indexed by its name, and returned. It is therefore safe to invoke this
	 * method any time the texture is needed, without having to worry that the texture will be
	 * repeatedly loaded from file.
	 *
	 * To clear a texture instance from the cache, use the removeTexture: method.
	 *
	 * To load the file directly, bypassing the cache, use the alloc and initFromFilesPosX:negX:posY:negY:posZ:negZ:
	 * methods. This technique can be used to load the same texture twice, if needed for some reason.
	 * Each distinct instance can then be given its own name, and added to the cache separately.
	 * However, when choosing to do so, be aware that textures often consume significant memory.
	 *
	 * Returns nil if the texture is not in the cache and any of the six files could not be loaded.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be an instance of a
	 * different class than the receiver.
	 */
	static CC3Texture*		textureCubeFromFiles( const char* posXFilePathm, const char* negXFilePath, 
		const char* posYFilePath, const char* negYFilePath, 
		const char* posZFilePath, const char* negZFilePath );

	/**
	 * Initializes this instance by loading the six cube face textures using the specified pattern
	 * string as a string format template to derive the names of the six textures, and returns whether
	 * all six files were successfully loaded.
	 *
	 * This method expects the six required files to have identical paths and names, except that
	 * each should contain one of the following character substrings in the same place in each
	 * file path: "PosX", "NegX", "PosY", "NegY", "PosZ", "NegZ".
	 *
	 * The specified file path pattern should include one standard NSString format marker %@ at
	 * the point where one of the substrings in the list above should be substituted.
	 *
	 * As an example, the file path pattern MyCubeTex%@.png would be expanded by this method
	 * to load the following six textures:
	 *  - MyCubeTexPosX.png
	 *  - MyCubeTexNegX.png
	 *  - MyCubeTexPosY.png
	 *  - MyCubeTexNegY.png
	 *  - MyCubeTexPosZ.png
	 *  - MyCubeTexNegZ.png
	 *
	 * The format marker can occur anywhere in the file name. It does not need to occur at the
	 * end as in this example.
	 *
	 * The specified file path pattern may be either an absolute path, or a path relative to
	 * the application resource directory. If the file is located directly in the application
	 * resources directory, the specified file path pattern can simply be the file name pattern.
	 *
	 * If the class-side shouldGenerateMipmaps property is set to YES, a mipmap will be generated
	 * for the texture automatically.
	 *
	 * The name of this instance is set to the unqualified file name derived from substituting
	 * an empty string into the format marker in the specified file path pattern string.
	 *
	 * Returns nil if any of the six files could not be loaded.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initCubeFromFilePattern( const std::string& aFilePathPattern );

	/**
	 * Returns an instance initialized by loading the six cube face textures using the specified pattern
	 * string as a string format template to derive the names of the six textures, and returns whether
	 * all six files were successfully loaded.
	 *
	 * This method expects the six required files to have identical paths and names, except that
	 * each should contain one of the following character substrings in the same place in each
	 * file path: "PosX", "NegX", "PosY", "NegY", "PosZ", "NegZ".
	 *
	 * The specified file path pattern should include one standard NSString format marker %@ at
	 * the point where one of the substrings in the list above should be substituted.
	 *
	 * As an example, the file path pattern MyCubeTex%@.png would be expanded by this method
	 * to load the following six textures:
	 *  - MyCubeTexPosX.png
	 *  - MyCubeTexNegX.png
	 *  - MyCubeTexPosY.png
	 *  - MyCubeTexNegY.png
	 *  - MyCubeTexPosZ.png
	 *  - MyCubeTexNegZ.png
	 *
	 * The format marker can occur anywhere in the file name. It does not need to occur at the
	 * end as in this example.
	 *
	 * The specified file path pattern may be either an absolute path, or a path relative to
	 * the application resource directory. If the file is located directly in the application
	 * resources directory, the specified file path pattern can simply be the file name pattern.
	 *
	 * If the class-side shouldGenerateMipmaps property is set to YES, a mipmap will be generated
	 * for the texture automatically.
	 *
	 * The name of this instance is set to the unqualified file name derived from substituting
	 * an empty string into the format marker in the specified file path pattern string.
	 *
	 * Textures loaded through this method are cached. If the texture was already loaded and is in
	 * the cache, it is retrieved and returned. If the texture has not in the cache, it is loaded,
	 * placed into the cache, indexed by its name, and returned. It is therefore safe to invoke this
	 * method any time the texture is needed, without having to worry that the texture will be
	 * repeatedly loaded from file.
	 *
	 * To clear a texture instance from the cache, use the removeTexture: method.
	 *
	 * To load the file directly, bypassing the cache, use the alloc and initFromFilePattern:
	 * methods. This technique can be used to load the same texture twice, if needed for some reason.
	 * Each distinct instance can then be given its own name, and added to the cache separately.
	 * However, when choosing to do so, be aware that textures often consume significant memory.
	 *
	 * Returns nil if the texture is not in the cache and any of the six files could not be loaded.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be an instance of a
	 * different class than the receiver.
	 */
	static CC3Texture*		textureCubeFromFilePattern( const std::string& aFilePathPattern );

	/**
	 * Initializes this instance from the specified texture properties, without providing content.
	 *
	 * Once initialized, the texture will be bound to the GL engine when the resizeTo: method is
	 * invoked, providing the texture with a size.
	 *
	 * See the notes for the pixelFormat and pixelType properties for the range of values permitted
	 * for the corresponding format and type parameters here.
	 *
	 * The name property of this instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initCubeWithPixelFormat( GLenum format, GLenum type );

	/**
	 * Allocates and initializes an autoreleased instance from the specified texture properties,
	 * without providing content.
	 *
	 * Once initialized, the texture will be bound to the GL engine when the resizeTo: method is
	 * invoked, providing the texture with a size.
	 *
	 * See the notes for the pixelFormat and pixelType properties for the range of values permitted
	 * for the corresponding format and type parameters here.
	 *
	 * The name property of this instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be an instance of a
	 * different class than the receiver.
	 */
	static CC3Texture*		textureCubeWithPixelFormat( GLenum format, GLenum type );

	/**
	 * Initializes this instance from the specified texture properties, without providing content.
	 *
	 * The sideLength argument indicates the length, in pixels, of each side of the texture.
	 *
	 * Once initialized, the texture will be bound to the GL engine, with space allocated for six
	 * texture faces of the specified size and pixel content. Content can be added later by using
	 * this texture as a rendering surface.
	 *
	 * See the notes for the pixelFormat and pixelType properties for the range of values permitted
	 * for the corresponding format and type parameters here.
	 *
	 * The name property of this instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initCubeWithSideLength( GLuint sideLength, GLenum format, GLenum type );

	/**
	 * Allocates and initializes an autoreleased instance from the specified texture properties,
	 * without providing content.
	 *
	 * The sideLength argument indicates the length, in pixels, of each side of the texture.
	 *
	 * Once initialized, the texture will be bound to the GL engine, with space allocated for a
	 * texture of the specified size and pixel content. Content can be added later by using this
	 * texture as a rendering surface.
	 *
	 * See the notes for the pixelFormat and pixelType properties for the range of values permitted
	 * for the corresponding format and type parameters here.
	 *
	 * The name property of this instance will be nil.
	 *
	 * Since textures can consume significant resources, you should assign this instance a name
	 * and add it to the texture cache by using the class-side addTexture: method. You can then
	 * retrieve the texture from the cache via the getTextureNamed: method to apply this texture
	 * to multple meshes.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be an instance of a
	 * different class than the receiver.
	 */
	static CC3Texture*		textureCubeWithSideLength( GLuint sideLength, GLenum format, GLenum type );

	/**
	 * Initializes this instance to have a unique solid color for each side of the cube.
	 *
	 * The sides of the cube are colored using an easy (RGB <=> XYZ) mnemonic as follows:
	 *   - +X-axis: Red
	 *   - -X-axis: Cyan (inverse of Red)
	 *   - +Y-axis: Green
	 *   - -Y-axis: Magenta (inverse of Green)
	 *   - +Z-axis: Blue
	 *   - -Z-axis: Yellow (inverse of Blue)
	 *
	 * Once initialized, the texture will be bound to the GL engine.
	 *
	 * The name property of this instance will be nil.
	 *
	 * Since the texture is just a solid color, a mipmap is not created.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be a different
	 * instance of a different class than the receiver.
	 */
	virtual bool			initCubeColoredForAxes();

	/**
	 * Returns an instance initialized to have a unique solid color for each side of the cube.
	 *
	 * The sides of the cube are colored using an easy (RGB <=> XYZ) mnemonic as follows:
	 *   - +X-axis: Red
	 *   - -X-axis: Cyan (inverse of Red)
	 *   - +Y-axis: Green
	 *   - -Y-axis: Magenta (inverse of Green)
	 *   - +Z-axis: Blue
	 *   - -Z-axis: Yellow (inverse of Blue)
	 *
	 * Once initialized, the texture will be bound to the GL engine.
	 *
	 * Since the texture is just a solid color, a mipmap is not created.
	 *
	 * The name of the instance is set to @"Axes-Colored-Cube".
	 *
	 * Textures loaded through this method are cached. If the texture was already loaded and is in
	 * the cache, it is retrieved and returned. If the texture has not in the cache, it is loaded,
	 * placed into the cache, indexed by its name, and returned. It is therefore safe to invoke this
	 * method any time the texture is needed, without having to worry that the texture will be
	 * repeatedly loaded from file.
	 *
	 * To clear a texture instance from the cache, use the removeTexture: method.
	 *
	 * To create a texture directly, bypassing the cache, use the alloc and initCubeColoredForAxes
	 * methods. This technique can be used to create the same texture twice, if needed for some reason.
	 * Each distinct instance can then be given its own name, and added to the cache separately.
	 * However, when choosing to do so, be aware that textures often consume significant memory.
	 *
	 * CC3Texture is the root of a class-cluster. The object returned may be an instance of a
	 * different class than the receiver.
	 */
	static CC3Texture*		textureCubeColoredForAxes();

	/**
	 * Returns a texture name derived from the specified file path.
	 *
	 * This method is used to standardize the naming of textures, to ease in adding and retrieving
	 * textures to and from the cache, and is used to create the name for each texture that is
	 * loaded from a file.
	 *
	 * This implementation returns the lastComponent of the specified file path.
	 */
	static std::string		textureNameFromFilePath( const std::string& filePath );

	/**
	 * Returns a description formatted as a source-code line for loading this texture from a file.
	 *
	 * During development time, you can log this string, then copy and paste it into a pre-loading
	 * function within your app code.
	 */
	virtual std::string		constructorDescription();

	/** Removes this texture instance from the cache. */
	virtual void			remove();

	/**
	 * Adds the specified texture to the collection of loaded textures.
	 *
	 * Textures are accessible via their names through the getTextureNamed: method, and each
	 * texture name should be unique. If a texture with the same name as the specified texture
	 * already exists in this cache, an assertion error is raised.
	 *
	 * This cache is a weak cache, meaning that it does not hold strong references to the textures
	 * that are added to it. As a result, the specified texture will automatically be deallocated
	 * and removed from this cache once all external strong references to it have been released.
	 */
	static void				addTexture( CC3Texture* texture );

	/** Returns the texture with the specified name, or nil if a texture with that name has not been added. */
	static CC3Texture*		getTextureNamed( const std::string& name );

	/** Removes the specified texture from the texture cache. */
	static void				removeTexture( CC3Texture* texture );

	/** Removes the texture with the specified name from the texture cache. */
	static void				removeTextureNamed( const std::string& name );

	/**
	 * Removes from the cache all textures that are instances of any subclass of the receiver.
	 *
	 * You can use this method to selectively remove specific types of texturs, based on
	 * the texture class, by invoking this method on that class. If you invoke this method
	 * on the CC3Texture class, this cache will be compltely cleared. However, if you invoke
	 * this method on one of its subclasses, only those textures that are instances of that
	 * subclass (or one of its subclasses in turn) will be removed, leaving the remaining
	 * textures in the cache.
	 */
	static void				removeAllTextures();

	/**
	 * Returns whether textures are being pre-loaded.
	 *
	 * See the setIsPreloading setter method for a description of how and when to use this property.
	 */
	static bool				isPreloading();

	/**
	 * Sets whether textures are being pre-loaded.
	 *
	 * Textures that are added to this cache while the value of this property is YES will be
	 * strongly cached and cannot be deallocated until specifically removed from this cache.
	 * You must manually remove any textures added to this cache while the value of this
	 * property is YES.
	 *
	 * Textures that are added to this cache while the value of this property is NO will be
	 * weakly cached, and will automatically be deallocated and removed from this cache once
	 * all references to the resource outside this cache are released.
	 *
	 * You can set the value of this property at any time, and can vary it between YES and NO
	 * to accomodate your specific loading patterns.
	 *
	 * The initial value of this property is NO, meaning that textures will be weakly cached
	 * in this cache, and will automatically be removed if not used in the scene. You can set
	 * this property to YES in order to pre-load textures that will not be immediately used
	 * in the scene, but which you wish to keep in the cache for later use.
	 */
	static void				setIsPreloading( bool bPreloading );

	/**
	 * Returns a description of the contents of this cache, with each entry formatted as a
	 * source-code line for loading the texture from a file.
	 *
	 * During development time, you can log this string, then copy and paste it into a
	 * pre-loading function within your app code.
	 */
	virtual std::string		cachedTexturesDescription();

	/** Sets the GL debug label, if required. */
	virtual void			checkGLDebugLabel();
	virtual void			checkTextureOrientation( CC3CCTexture* texContent );
	/** If the texture parameters are dirty, binds them to the GL texture unit state. */
	virtual void			bindTextureParametersAt( GLuint tuIdx, CC3OpenGL* gl );
	/** Binds the default texture unit environment to the GL engine. */
	virtual void			bindTextureEnvironmentWithVisitor( CC3NodeDrawingVisitor* visitor );
	virtual void			markTextureParametersDirty();
	virtual GLuint			getTextureUnitFromVisitor( CC3NodeDrawingVisitor* visitor );
	virtual void			incrementTextureUnitInVisitor( CC3NodeDrawingVisitor* visitor );
	virtual void			initWithTag( GLuint aTag, const std::string& aName );

	virtual void			populateFrom( CC3Texture* another );
	virtual CCObject*		copyWithZone( CCZone* zone );
	/**
	 * Converts the pixels in the specified array to the format and type used by this texture.
	 * Upon completion, the specified pixel array will contain the converted pixels.
	 *
	 * Since the pixels in any possible converted format will never consume more memory than
	 * the pixels in the incoming 32-bit RGBA format, the conversion is perfomed in-place.
	 */
	virtual void			convertContent( ccColor4B* colorArray, GLuint pixCount );

	/**
	 * If the class-side shouldCacheAssociatedCCTextures propery is set to YES, and a CCTexture
	 * with the same name as this texture does not already exist in the CCTextureCache, adds the
	 * CCTexture returned by the ccTexture property to the CCTextureCache.
	 */
	virtual void			cacheCCTexture2D();

protected:
	virtual bool			loadTarget( GLenum target, const std::string& filePath );
	virtual bool			loadFromFile( const std::string& filePath );

	virtual void			bindTextureOfColor( const ccColor4B& color, const CC3IntSize& size, GLenum target );

protected:
	GLuint					m_textureID;
	CC3IntSize				m_size;
	CCSize					m_coverage;
	GLenum					m_pixelFormat;
	GLenum					m_pixelType;
	GLenum					m_minifyingFunction;
	GLenum					m_magnifyingFunction;
	GLenum					m_horizontalWrappingFunction;
	GLenum					m_verticalWrappingFunction;
	CC3CCTexture*			m_ccTexture;
	bool					m_texParametersAreDirty : 1;
	bool					m_hasMipmap : 1;
	bool					m_isUpsideDown : 1;
	bool					m_shouldFlipVerticallyOnLoad : 1;
	bool					m_shouldFlipHorizontallyOnLoad : 1;
	bool					m_hasAlpha : 1;
	bool					m_hasPremultipliedAlpha : 1;
};

/**
 * The representation of a 2D texture loaded into the GL engine.
 *
 * This class is used for all 2D texture types except PVR.
 *
 * This class is part of a class-cluster under the parent CC3Texture class. Although you can
 * invoke an instance creation method on this class directly, you will more commonly invoke
 * them on the CC3Texture class instead. The creation and initialization methods will ensure
 * that the correct subclass for the texture type, and in some cases, the texture file type,
 * is created and returned. Because of this class-cluster structure, be aware that the class
 * of the instance returned by an instance creation or initialization method may be different
 * than the receiver of that method.
 */
class CC3Texture2D : public CC3Texture
{
	DECLARE_SUPER( CC3Texture );
public:
	CC3Texture2D();

	GLenum					getSamplerSemantic();
	bool					isTexture2D();
	GLenum					getTextureTarget();
	GLenum					getInitialAttachmentFace();

	/** If the specified texture content is new to this texture, the contained content is updated. */
	void					bindTextureContent( CC3CCTexture* texContent, GLenum target );
	void					resizeTo( const CC3IntSize& size );
	/**
	 * Replaces a portion of the content of this texture by writing the specified array of pixels
	 * into the specified rectangular area within this texture, The specified content replaces
	 * the texture data within the specified rectangle. The specified content array must be large
	 * enough to contain content for the number of pixels in the specified rectangle.
	 *
	 * Content is read from the specified array left to right across each row of pixels within
	 * the specified image rectangle, starting at the row at the bottom of the rectangle, and
	 * ending at the row at the top of the rectangle.
	 *
	 * Within the specified array, the pixel content should be packed tightly, with no gaps left
	 * at the end of each row. The last pixel of one row should immediately be followed by the
	 * first pixel of the next row.
	 *
	 * The pixels in the specified array are in standard 32-bit RGBA. If the pixelFormat and
	 * pixelType properties of this texture are not GL_RGBA and GL_UNSIGNED_BYTE, respectively,
	 * the pixels in the specified array will be converted to the format and type of this texture
	 * before being inserted into the texture. Be aware that this conversion will reduce the
	 * performance of this method. For maximum performance, match the format and type of this
	 * texture to the 32-bit RGBA format of the specified array, by setting the pixelFormat
	 * property to GL_RGBA and the pixelType property to GL_UNSIGNED_BYTE. However, keep in mind
	 * that the 32-bit RGBA format consumes more memory than most other formats, so if performance
	 * is of lesser concern, you may choose to minimize the memory requirements of this texture
	 * by setting the pixelFormat and pixelType properties to values that consume less memory.
	 *
	 * If this texture has mipmaps, they are not automatically updated. Once all desired content
	 * has been replaced, invoke the generateMipmap method to regenerate the mipmaps.
	 */
	void					replacePixels( const CC3Viewport& rect, ccColor4B* colorArray );

	GLuint					getTextureUnitFromVisitor( CC3NodeDrawingVisitor* visitor );
	void					incrementTextureUnitInVisitor( CC3NodeDrawingVisitor* visitor );
	bool					initWithPixelFormat( GLenum format, GLenum type );
	bool					initWithSize( const CC3IntSize& size, GLenum format, GLenum type );
	bool					initWithSize( const CC3IntSize& size, const ccColor4B& color );
	bool					initWithCCTexture( CC3CCTexture* ccTexture );

	/**
	 * This class-side property determines the initial value of the shouldFlipVerticallyOnLoad
	 * for instances of this class.
	 *
	 * The initial value for 2D textures is YES, indicating that a 2D texture that has been loaded
	 * in upsdide-down will be fipped the right way up.
	 */
	static bool				defaultShouldFlipVerticallyOnLoad();

	/**
	 * This class-side property determines the initial value of the shouldFlipVerticallyOnLoad
	 * for instances of this class.
	 *
	 * The initial value for 2D textures is YES, indicating that a 2D texture that has been loaded
	 * in upsdide-down will be fipped the right way up.
	 */
	static void				setDefaultShouldFlipVerticallyOnLoad( bool shouldFlip );

	/**
	 * This class-side property determines the initial value of the shouldFlipHorizontallyOnLoad
	 * for instances of this class. The initial value for 2D textures is NO.
	 */
	static bool				defaultShouldFlipHorizontallyOnLoad();

	/**
	 * This class-side property determines the initial value of the shouldFlipHorizontallyOnLoad
	 * for instances of this class. The initial value for 2D textures is NO.
	 */
	static void				setDefaultShouldFlipHorizontallyOnLoad( bool shouldFlip );
};

/** 
 * The representation of a 3D cube-map texture loaded into the GL engine.
 *
 * This class is used for all cube-map texture types except PVR.
 *
 * This class is part of a class-cluster under the parent CC3Texture class. Although you can
 * invoke an instance creation method on this class directly, you will more commonly invoke
 * them on the CC3Texture class instead. The creation and initialization methods will ensure 
 * that the correct subclass for the texture type, and in some cases, the texture file type,
 * is created and returned. Because of this class-cluster structure, be aware that the class
 * of the instance returned by an instance creation or initialization method may be different
 * than the receiver of that method.
 */
class CC3TextureCube : public CC3Texture
{
	DECLARE_SUPER( CC3Texture );
public:
	/**
	 * Loads the texture file at the specified file path into the specified cube face target,
	 * and returns whether the loading was successful.
	 *
	 * The specified file path may be either an absolute path, or a path relative to the
	 * application resource directory. If the file is located directly in the application
	 * resources directory, the specified file path can simply be the name of the file.
	 *
	 * The specified cube face target can be one of the following:
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_X
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_X
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	 *
	 * In order to complete this cube texture, this method should be invoked once for each
	 * of these six face targets.
	 *
	 * If this instance has not been assigned a name, it is set to the unqualified file name
	 * from the specified file path.
	 *
	 * This method does not automatically generate a mipmap. If you want a mipmap, you should
	 * invoke the generateMipmap method once all six faces have been loaded.
	 */
	bool					loadCubeFace( GLenum faceTarget, const char* filePath );

	/**
	 * Loads the six cube face textures at the specified file paths, and returns whether all
	 * six files were successfully loaded.
	 *
	 * If this instance has not been assigned a name, it is set to the unqualified file name
	 * of the specified posXFilePath file path.
	 *
	 * If the class-side shouldGenerateMipmaps property is set to YES, a mipmap will be generated
	 * for the texture automatically.
	 *
	 * If the instance is instantiated via initFromFilesPosX:negX:posY:negY:posZ:negZ: or
	 * textureFromFilesPosX:negX:posY:negY:posZ:negZ:, this method is invoked automatically
	 * during instance initialization. If the instance is instantiated without using one of
	 * those file-loading initializers, this method can be invoked directly to load the files.
	 *
	 * Each of the specified file paths may be either an absolute path, or a path relative to
	 * the application resource directory. If the file is located directly in the application
	 * resources directory, the corresponding file path can simply be the name of the file.
	 */
	bool					loadFromFiles( const std::string& posXFilePath, const std::string& negXFilePath, 
		const std::string& posYFilePath, const std::string& negYFilePath,
		const std::string& posZFilePath, const std::string& negZFilePath );

	/**
	 * Loads the six cube face textures using the specified pattern string as a string format
	 * template to derive the names of the six textures, and returns whether all six files were
	 * successfully loaded.
	 *
	 * If the class-side shouldGenerateMipmaps property is set to YES, a mipmap will be generated
	 * for the texture automatically.
	 *
	 * If the instance is instantiated via initFromFilePattern: or textureFromFilePattern:, 
	 * this method is invoked automatically during instance initialization. If the instance
	 * is instantiated without using one of those file-loading initializers, this method can
	 * be invoked directly to load the files.
	 *
	 * This method expects the six required files to have identical paths and names, except that
	 * each should contain one of the following character substrings in the same place in each
	 * file path: "PosX", "NegX", "PosY", "NegY", "PosZ", "NegZ".
	 *
	 * The specified file path pattern should include one standard NSString format marker %@ at
	 * the point where one of the substrings in the list above should be substituted.
	 *
	 * As an example, the file path pattern MyCubeTex%@.png would be expanded by this method
	 * to load the following six textures:
	 *  - MyCubeTexPosX.png
	 *  - MyCubeTexNegX.png
	 *  - MyCubeTexPosY.png
	 *  - MyCubeTexNegY.png
	 *  - MyCubeTexPosZ.png
	 *  - MyCubeTexNegZ.png
	 *
	 * The format marker can occur anywhere in the file name. It does not need to occur at the
	 * end as in this example.
	 *
	 * The specified file path pattern may be either an absolute path, or a path relative to 
	 * the application resource directory. If the file is located directly in the application
	 * resources directory, the specified file path pattern can simply be the file name pattern.
	 *
	 * If this instance has not been assigned a name, it is set to the unqualified file name
	 * derived from substituting an empty string into the format marker in the specified file
	 * path pattern string.
	 */
	bool					loadFromFilePattern( const std::string& aFilePathPattern );

	/**
	 * Replaces a portion of the content of this texture by writing the specified array of pixels
	 * into the specified rectangular area within the specified face of this texture, The specified
	 * content replaces the texture data within the specified rectangle. The specified content array
	 * must be large enough to contain content for the number of pixels in the specified rectangle.
	 *
	 * The specified cube face target can be one of the following:
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_X
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_X
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	 *
	 * Content is read from the specified array left to right across each row of pixels within
	 * the specified image rectangle, starting at the row at the bottom of the rectangle, and
	 * ending at the row at the top of the rectangle.
	 *
	 * Within the specified array, the pixel content should be packed tightly, with no gaps left
	 * at the end of each row. The last pixel of one row should immediately be followed by the
	 * first pixel of the next row.
	 *
	 * The pixels in the specified array are in standard 32-bit RGBA. If the pixelFormat and
	 * pixelType properties of this texture are not GL_RGBA and GL_UNSIGNED_BYTE, respectively,
	 * the pixels in the specified array will be converted to the format and type of this texture
	 * before being inserted into the texture. Be aware that this conversion will reduce the
	 * performance of this method. For maximum performance, match the format and type of this
	 * texture to the 32-bit RGBA format of the specified array, by setting the pixelFormat
	 * property to GL_RGBA and the pixelType property to GL_UNSIGNED_BYTE. However, keep in mind
	 * that the 32-bit RGBA format consumes more memory than most other formats, so if performance
	 * is of lesser concern, you may choose to minimize the memory requirements of this texture
	 * by setting the pixelFormat and pixelType properties to values that consume less memory.
	 *
	 * If this texture has mipmaps, they are not automatically updated. Once all desired content
	 * has been replaced, invoke the generateMipmap method to regenerate the mipmaps.
	 */
	void					replacePixels( const CC3Viewport& rect, GLenum faceTarget, ccColor4B* colorArray );

	/**
	 * This class-side property determines the initial value of the shouldFlipVerticallyOnLoad
	 * for instances of this class.
	 *
	 * The initial value for cube-map textures is NO, indicating that a cube-map texture that
	 * has been loaded in upsdide-down will be left upside-down. This is because cube-mapped
	 * textures need to be stored in GL memory rotated by 180 degrees (flipped both vertically
	 * and horizontally).
	 */
	static bool				defaultShouldFlipVerticallyOnLoad();

	/**
	 * This class-side property determines the initial value of the shouldFlipVerticallyOnLoad
	 * for instances of this class.
	 *
	 * The initial value for cube-map textures is NO, indicating that a cube-map texture that
	 * has been loaded in upsdide-down will be left upside-down. This is because cube-mapped
	 * textures need to be stored in GL memory rotated by 180 degrees (flipped both vertically
	 * and horizontally).
	 */
	static void				setDefaultShouldFlipVerticallyOnLoad( bool shouldFlip );

	/**
	 * This class-side property determines the initial value of the shouldFlipHorizontallyOnLoad
	 * for instances of this class. 
	 * 
	 * The initial value for cube-map textures is YES, indicating that the texture will be flipped
	 * horizontally. This is because cube-mapped textures need to be stored in GL memory rotated
	 * by 180 degrees (flipped both vertically and horizontally).
	 */
	static bool				defaultShouldFlipHorizontallyOnLoad();

	/**
	 * This class-side property determines the initial value of the shouldFlipHorizontallyOnLoad
	 * for instances of this class.
	 *
	 * The initial value for cube-map textures is YES, indicating that the texture will be flipped
	 * horizontally. This is because cube-mapped textures need to be stored in GL memory rotated
	 * by 180 degrees (flipped both vertically and horizontally).
	 */
	static void				setDefaultShouldFlipHorizontallyOnLoad( bool shouldFlip );

	GLenum					getSamplerSemantic();
	bool					isTextureCube();
	GLenum					getTextureTarget();
	GLenum					getInitialAttachmentFace();

	static ccTexParams		defaultTextureParameters();
	static void				setDefaultTextureParameters( const ccTexParams& texParams );

	void					resizeTo( const CC3IntSize& size );
	bool					loadFromFile( const std::string& filePath );
	bool					loadCubeFace( GLenum faceTarget, const std::string& filePath );

	GLuint					getTextureUnitFromVisitor( CC3NodeDrawingVisitor* visitor );
	void					incrementTextureUnitInVisitor( CC3NodeDrawingVisitor* visitor );

	bool					initCubeFromFiles( const std::string& posXFilePath, const std::string& negXFilePath, 
		const std::string& posYFilePath, const std::string& negYFilePath,
		const std::string& posZFilePath, const std::string& negZFilePath );

	bool					initCubeFromFilePattern( const std::string& aFilePathPattern );
	bool					initCubeWithPixelFormat( GLenum format, GLenum type );

	bool					initCubeWithSideLength( GLuint sideLength, GLenum format, GLenum type );
	bool					initCubeColoredForAxes();
	std::string				constructorDescription();
};

/**
 * CC3TextureUnitTexture is a specialized CC3Texture subclass that actually wraps another
 * texture instance and combines it with an instance of a texture unit to define additional
 * environmental configuration information about the use of the texture in multi-texturing
 * under fixed-pipeline rendering used by OpenGL ES 1.1 on iOS, or OpenGL on OSX without shaders.
 *
 * This class is generally not used for multi-texturing under programmable-pipeline rendering
 * used by OpenGL ES 2.0 on, or OpenGL on OSX with shaders, as you will generally handle
 * multitexturing in the shader code. However, it is possible to use an instance of this
 * class with a programmable-pipeline shader if your shader is designed to make use of the
 * texture unit configuration content. This can be used as a mechanism for supporting the
 * same multi-texturing configuration between both fixed and programmable pipelines.
 *
 * You instantiate an instance of CC3TextureUnitTexture directly, using any of the instance
 * creation or initializaton methods defined by the CC3Texture superclass. Or, if you already
 * have an instance of a CC3Texture, you can wrap it in an instance of CC3TextureUnitTexture
 * by using the textureWithTexture: or initWithTexture: creation and initialization methods
 * of this class.
 *
 * You can then create an instance of CC3TextureUnit, configure it appropriately, and set it
 * into the textureUnit property of your CC3TextureUnitTexture instance. By adding multiple
 * CC3TextureUnitTexture instances to your CC3Material, you can combine textures creatively.
 *
 * For example, to configure a material for bump-mapping, add a texture that contains a
 * normal vector at each pixel instead of a color, and set the textureUnit property of
 * the texture to a CC3BumpMapTextureUnit. Then add another texture, containing the image
 * that will be visible, to the material. The material will combine these two textures,
 * as specified by the CC3TextureUnit held by the second texture.
 */
class CC3TextureUnitTexture : public CC3Texture 
{
	DECLARE_SUPER( CC3Texture );
public:
	CC3TextureUnitTexture();
	~CC3TextureUnitTexture();
	/**
	 * The CC3Texture texture being managed by this instance.
	 *
	 * This property is populated automatically during instance creation and loading.
	 */
	CC3Texture*				getTexture();

	/**
	 * The texture environment settings that are applied to the texture unit that draws this
	 * texture, when this texture participates in multi-texturing under fixed-pipeline rendering.
	 *
	 * The texture unit is optional, and this propety may be left as nil to provide standard
	 * single texture rendering. The default value of this property is nil.
	 *
	 * The texture unit can be used to configure how the texture will be combined with other
	 * textures when using multi-texturing. When the material supports multiple textures, each
	 * texture should contain a texture unit that describes how the GL engine should combine
	 * that texture with the textures that have already been applied.
	 *
	 * Different subclasses of CC3TextureUnit provide different customizations for combining
	 * textures. The CC3BumpMapTextureUnit provides easy settings for DOT3 bump-mapping, and
	 * CC3ConfigurableTextureUnit provides complete flexibility in setting texture environment
	 * settings.
	 */
	CC3TextureUnit*			getTextureUnit();
	void					setTextureUnit( CC3TextureUnit* unit );

	/**
	 * Initializes this instance on the specified underlying texture.
	 *
	 * The name property of this instance will set to that of the specified texture.
	 */
	void					initWithTexture( CC3Texture* texture );

	/**
	 * Allocates and initializes an autoreleased instance on the specified underlying texture.
	 *
	 * The name property of this instance will set to that of the specified texture.
	 */
	static CC3TextureUnitTexture* textureWithTexture( CC3Texture* texture );


	void					setTexture( CC3Texture* texture );
	GLuint					getTextureID();
	CC3IntSize				getSize();
	bool					isPOTWidth();
	bool					isPOTHeight();
	bool					isPOT();
	GLenum					getSamplerSemantic();
	bool					isTexture2D();
	bool					isTextureCube();
	CCSize					getCoverage();
	GLenum					getPsixelFormat();
	GLenum					getPixelType();
	bool					hasAlpha();
	void					setHasAlpha( bool hasAlpha );
	bool					hasPremultipliedAlpha();
	void					setHasPremultipliedAlpha( bool hasPremultipliedAlpha );
	bool					isUpsideDown();
	void					setIsUpsideDown( bool isUpsideDown );
	GLenum					getTextureTarget();
	GLenum					getInitialAttachmentFace();
	bool					shouldFlipVerticallyOnLoad();
	void					setShouldFlipVerticallyOnLoad( bool shouldFlipVerticallyOnLoad );
	bool					shouldFlipHorizontallyOnLoad();
	void					setShouldFlipHorizontallyOnLoad( bool shouldFlipHorizontallyOnLoad );
	bool					hasMipmap();
	void					generateMipmap();
	GLenum					getMinifyingFunction();
	void					setMinifyingFunction( GLenum minifyingFunction );
	GLenum					getMagnifyingFunction();
	void					setMagnifyingFunction( GLenum magnifyingFunction );
	GLenum					getHorizontalWrappingFunction();
	void					setHorizontalWrappingFunction( GLenum horizontalWrappingFunction );
	GLenum					getVerticalWrappingFunction();
	void					setVerticalWrappingFunction( GLenum verticalWrappingFunction );
	ccTexParams				getTextureParameters();
	void					setTextureParameters( const ccTexParams& textureParameters );
	void					replacePixels( const CC3Viewport& rect, GLenum target, ccColor4B* colorArray );
	void					resizeTo( const CC3IntSize& size );
	CC3Vector				getLightDirection();
	void					setLightDirection( const CC3Vector& aDirection );
	bool					isBumpMap();
	void					bindTextureParametersAt( GLuint tuIdx, CC3OpenGL* gl );
	/** Binds texture unit environment to the GL engine. */
	void					bindTextureEnvironmentWithVisitor( CC3NodeDrawingVisitor* visitor );
	GLuint					getTextureUnitFromVisitor( CC3NodeDrawingVisitor* visitor );
	void					incrementTextureUnitInVisitor( CC3NodeDrawingVisitor* visitor );
	void					initWithTag( GLuint aTag, const std::string& aName );
	bool					initFromFile( const std::string& filePath );
	static CC3TextureUnitTexture*	textureFromFile( const std::string& filePath );
	bool					initWithPixelFormat( GLenum format, GLenum type );
	static CC3TextureUnitTexture*	textureWithPixelFormat( GLenum format, GLenum type );
	bool					initWithSize( const CC3IntSize& size, GLenum format, GLenum type );
	static CC3TextureUnitTexture*	textureWithSize( const CC3IntSize& size, GLenum format, GLenum type );
	bool					initCubeFromFiles( const std::string& posXFilePath, const std::string& negXFilePath, 
		const std::string& posYFilePath, const std::string& negYFilePath, 
		const std::string& posZFilePath, const std::string& negZFilePath );
	static CC3TextureUnitTexture* textureCubeFromFiles( const std::string& posXFilePath, const std::string& negXFilePath, 
		const std::string& posYFilePath, const std::string& negYFilePath, 
		const std::string& posZFilePath, const std::string& negZFilePath );

	bool					initCubeFromFilePattern( const std::string& aFilePathPattern );

	static CC3TextureUnitTexture* textureCubeFromFilePattern( const std::string& aFilePathPattern );
	bool					initCubeWithPixelFormat( GLenum format, GLenum type );
	static CC3TextureUnitTexture* textureCubeWithPixelFormat( GLenum format, GLenum type );
	bool					initCubeWithSize( const CC3IntSize& size, GLenum format, GLenum type );
	static CC3TextureUnitTexture* textureCubeWithSize( const CC3IntSize& size, GLenum format, GLenum type );

	/** Don't invoke super, because normal textures are not copyable */
	void					populateFrom( CC3TextureUnitTexture* another );
	virtual CCObject*		copyWithZone( CCZone* zone );

	std::string				fullDescription();

protected:
	CC3Texture*				m_pTexture;
	CC3TextureUnit*			m_pTextureUnit;
};


/** Extension category to support Cocos3D functionality. */
class CC3CCTexture : public CCTexture
{
public:
	/** Sets the GL texture ID. */
	virtual void			setName( GLuint name );

	/**
	 * Returns the GL engine pixel format of the texture.
	 *
	 * See the pixelFormat property of CC3Texture for the range of possible values.
	 */
	virtual GLenum			getPixelGLFormat();

	/**
	 * Returns the pixel data type.
	 *
	 * Possible values depend on the value of the pixelFormat property. See the pixelType
	 * property of CC3Texture for the range of possible values.
	 */
	virtual GLenum			getPixelGLType();

	/**
	 * Indicates whether this texture has an alpha channel, representing opacity.
	 *
	 * The value of this property is derived from the value of the pixelGLFomat property.
	 */
	virtual bool			hasAlpha();

	/** Returns the number of bytes in each pixel of content. */
	virtual GLuint			getBytesPerPixel();

	/**
	 * Returns whether a mipmap has been generated for this texture.
	 *
	 * Mipmaps can also be generated by invoking the generateMipmap method.
	 */
	virtual bool			hasMipmap();

	/**
	 * Indicates whether this texture is upside-down.
	 *
	 * The vertical axis of the coordinate system of OpenGL is inverted relative to the CoreGraphics
	 * view coordinate system. As a result, texture content can be initially loaded upside down.
	 * When this happens, this property will return YES, otherwise, it will return NO.
	 */
	virtual bool			isUpsideDown();

	/**
	 * Indicates whether texture are loaded upside-down.
	 *
	 * For Cocos2D 3.0 and before, textures are loaded and applied upside-down. 
	 * For Cocos2D 3.1 and after, textures are loaded and applied right-side-up.
	 */
	static bool				texturesAreLoadedUpsideDown();

	/** Returns NULL. For compatibility with CC3Texture2DContent. */
	virtual const GLvoid*	getImageData();

	/** Does nothing. For compatibility with CC3Texture2DContent. */
	virtual void			flipVertically();

	/** Does nothing. For compatibility with CC3Texture2DContent. */
	virtual void			flipHorizontally();

	/** Does nothing. For compatibility with CC3Texture2DContent. */
	virtual void			rotateHalfCircle();

	/**
	 * Resizes this texture to the specified dimensions.
	 *
	 * This method changes the values of the size, width, height, maxS & maxT properties, and
	 * deletes any contained image data, but does not make any changes to the texture within the
	 * GL engine. This method is invoked during the resizing of a texture that backs a surface.
	 */
	virtual void			resizeTo( const CC3IntSize& size );

	/** Does nothing. For compatibility with CC3Texture2DContent. */
	virtual void			deleteImageData();

	/**
	 * If a CCTexture with the specified name does not already exist in the CCTextureCache,
	 * this texture is added to the CCTextureCache under that name.
	 *
	 * If a texture already exists in the cache under the specified name, or if the specified
	 * name is nil, this texture is not added to the cache.
	 */
	virtual void			addToCacheWithName( const char* texName );

	/** Legacy support for renamed pixelsWide property. */
	virtual unsigned int	getPixelWidth();

	/** Legacy support for renamed pixelsHigh property. */
	virtual unsigned int	getPixelHeight();

	virtual bool			initFromFile( const std::string& file );

	virtual bool			init();
};

/**
 * A CCTexture subclass used by the CC3Texture class cluster during the loading of a 2D
 * texture, and when extracting a CCTexture from the CC3Texture ccTexture property.
 *
 * PVR texture files cannot be loaded using this class.
 */
class CC3Texture2DContent : public CC3CCTexture 
{
	DECLARE_SUPER( CC3CCTexture );
public:
	CC3Texture2DContent();
	~CC3Texture2DContent();

	/** 
	 * Flips this texture vertically, to compensate for the opposite orientation
	 * of vertical graphical coordinates between OpenGL and iOS & OSX.
	 *
	 * The value of the isUpsideDown property is toggled after flipping.
	 */
	void					flipVertically();

	/** Flips this texture horizontally. */
	void					flipHorizontally();

	/**
	 * Rotates the image by 180 degrees. 
	 *
	 * This is equivalent to combined vertical and horizontal flips, but is executed
	 * in one pass for efficiency.
	 *
	 * The value of the isUpsideDown property is toggled after rotating.
	 */
	void					rotateHalfCircle();

	/** 
	 * Resizes this texture to the specified dimensions.
	 *
	 * This method changes the values of the size, width, height, maxS & maxT properties, 
	 * but does not make any changes to the texture within the GL engine. This method is
	 * invoked during the resizing of a texture that backs a surface.
	 */
	void					resizeTo( const CC3IntSize& size );

	const GLvoid*			getImageData();

	/**
	 * Deletes the texture content from main memory. This should be invoked
	 * once the texture is bound to the GL engine. 
	 */
	void					deleteImageData();

	/**
	 * Initializes this instance with content loaded from the specified file.
	 *
	 * The specified file path may be either an absolute path, or a path relative to the
	 * application resource directory. If the file is located directly in the application
	 * resources directory, the specified file path can simply be the name of the file.
	 *
	 * Returns nil if the file could not be loaded.
	 *
	 * The value of the isUpsideDown is set to YES.
	 */
	bool					initFromFile( const std::string& filePath );

	/** 
	 * Initializes this instance to define the properties of a texture, without defining any
	 * specific content.
	 *
	 * This instance can be used to initialize an empty CC3Texture, to which content can be added later.
	 *
	 * The value of the isUpsideDown is set to NO.
	 */
	bool					initWithSize( const CC3IntSize& size, GLenum format, GLenum type );

	/**
	 * Initializes this instance containing pixel content of the specified size and solid, uniform color.
	 *
	 * This method is useful for creating a blank tetxure canvas of a particular size and color.
	 * By accessing the imageData property, the application can then draw pixels to this canvas.
	 */
	bool					initWithSize( const CC3IntSize& size, const ccColor4B& color );

	/**
	 * Allocates and initializes an autoreleased instance containing pixel content of the 
	 * specified size and solid, uniform color.
	 *
	 * This method is useful for creating a blank tetxure canvas of a particular size and color.
	 * By accessing the imageData property, the application can then draw pixels to this canvas.
	 */
	static CC3Texture2DContent*		textureWithSize( const CC3IntSize& size, const ccColor4B& color );

	/** Initializes this instance to represent the same GL texture as the specified CC3Texture. */
	void					initFromCC3Texture( CC3Texture* texture );

	/** Allocates and initializes an instance to represent the same GL texture as the specified CC3Texture. */
	static CC3Texture2DContent*		textureFromCC3Texture( CC3Texture* texture );

	void					updatePixelFormat();

	GLenum					getPixelGLFormat();
	GLenum					getPixelGLType();
	bool					isUpsideDown();

protected:
	bool					initFromSTBIFile( const std::string& filePath );
	bool					initFromOSFile( const std::string& filePath );

	bool					initWithData( const void* data, CCTexture2DPixelFormat pixelFormat, unsigned int pixelsWide, unsigned int pixelsHigh, const CCSize& contentSize );
	GLuint					getImageDataSize( CCImage *image, unsigned int width, unsigned int height );

protected:
	GLvoid*					m_imageData;
	GLuint					m_imageDataSize;

	GLenum					m_pixelGLFormat;
	GLenum					m_pixelGLType;
	bool					m_isUpsideDown : 1;
};

/** Extension category to support Cocos3D functionality. */
class CC3CCTextureCache : public CCTextureCache
{
public:
	/**
	* If a texture with the specified name does not already exist in this cache, the specified
	* texture is added under the specified name.
	*
	* If a texture already exists in this cache under the specified name, or if either the
	* specified texture or specified name is nil, the texture is not added to the cache.
	*/
	void addTexture( CCTexture* tex2D, const char* texName );
};

/** Returns the OpenGL pixel format corresponding to the specfied CCTexturePixelFormat. */
static GLenum CC3PixelGLFormatFromCCTexturePixelFormat(CCTexturePixelFormat pixelFormat);

/** Returns the OpenGL pixel type corresponding to the specfied CCTexturePixelFormat. */
static GLenum CC3PixelGLTypeFromCCTexturePixelFormat(CCTexturePixelFormat pixelFormat);

/**
 * Returns the CCTexturePixelFormat corresponding to the specified OpenGL pixel format and type.
 *
 * Not all combinations of OpenGL pixel format and type can be mapped to a corresponding
 * CCTexturePixelFormat value. In those cases, this function returns CCTexturePixelFormat_Default.
 */
static CCTexturePixelFormat CCTexturePixelFormatFromGLFormatAndType(GLenum pixelFormat, GLenum pixelType);


// Macros for legacy references to removed classes and methods
#define CC3GLTexture			CC3Texture
#define CC3GLTexture2D			CC3Texture2D
#define CC3GLTextureCube		CC3TextureCube
#define CC3PVRGLTexture			CC3PVRTexture
#define addGLTexture			addTexture
#define getGLTextureNamed		getTextureNamed
#define removeGLTexture			removeTexture

NS_COCOS3D_END

#endif
