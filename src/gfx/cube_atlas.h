/*
* Copyright 2013 Jeremie Roy. All rights reserved.
* License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
*/

#ifndef CUBE_ATLAS_H_HEADER_GUARD
#define CUBE_ATLAS_H_HEADER_GUARD

#include <cstdint>
#include <array>
#include <SDL3/SDL.h>

// this is the minimum size to be able to load 3000 glyphs at 24 pts
const int ATLAS_DEFAULT = 512 + 128;

/// Inspired from texture-atlas from freetype-gl (http://code.google.com/p/freetype-gl/)
/// by Nicolas Rougier (Nicolas.Rougier@inria.fr)
/// The actual implementation is based on the article by Jukka Jylänki : "A
/// Thousand Ways to Pack the Bin - A Practical Approach to Two-Dimensional
/// Rectangle Bin Packing", February 27, 2010.
/// More precisely, this is an implementation of the Skyline Bottom-Left
/// algorithm based on C++ sources provided by Jukka Jylänki at:
/// http://clb.demon.fi/files/RectangleBinPack/

struct AtlasRegion
{
	enum Type
	{
		TYPE_GRAY = 1, // 1 component
		TYPE_BGRA8 = 4  // 4 components
	};

	uint16_t x, y;
	uint16_t width, height;
	uint32_t mask; //encode the region type, the face index and the component index in case of a gray region

	Type getType() const
	{
		return (Type)((mask >> 0) & 0x0000000F);
	}

	uint32_t getFaceIndex() const
	{
		return (mask >> 4) & 0x0000000F;
	}

	uint32_t getComponentIndex() const
	{
		return (mask >> 8) & 0x0000000F;
	}

	void setMask(Type _type, uint32_t _faceIndex, uint32_t _componentIndex)
	{
		mask = (_componentIndex << 8) + (_faceIndex << 4) + (uint32_t)_type;
	}
};

class Atlas
{
public:
	/// create an empty dynamic atlas (region can be updated and added)
	/// @param _renderer The SDL_Renderer to use for creating and updating textures.
	/// @param _textureSize An atlas creates 6 texture faces, each with a size of (_textureSize * _textureSize).
	/// @param _maxRegionsCount Maximum number of regions allowed in the atlas.
	Atlas(SDL_Renderer* _renderer, uint16_t _textureSize = ATLAS_DEFAULT, uint16_t _maxRegionsCount = 4096);

	/// initialize a static atlas with serialized data (region can be updated but not added)
	/// @param _renderer The SDL_Renderer to use for creating and updating textures.
	/// @param _textureSize An atlas creates 6 texture faces, each with a size of (_textureSize * _textureSize).
	/// @param _textureBuffer Buffer of size 6*_textureSize*_textureSize*4 (will be copied).
	/// @param _regionCount Number of regions in the Atlas.
	/// @param _regionBuffer Buffer containing the region data (will be copied).
	/// @param _maxRegionsCount Maximum number of regions allowed in the atlas.
	Atlas(SDL_Renderer* _renderer, uint16_t _textureSize, const uint8_t* _textureBuffer, uint16_t _regionCount, const uint8_t* _regionBuffer, uint16_t _maxRegionsCount = 4096);
	~Atlas();

	// Atlas manages GPU resources and raw pointers, so it should not be copyable.
	Atlas(const Atlas&) = delete;
	Atlas& operator=(const Atlas&) = delete;

	/// Add a region to the atlas and copy the content of the bitmap to the underlying texture.
	uint16_t addRegion(uint16_t _width, uint16_t _height, const uint8_t* _bitmapBuffer, AtlasRegion::Type _type = AtlasRegion::TYPE_BGRA8, uint16_t outline = 0);

	/// Update a pre-allocated region with new bitmap data.
	void updateRegion(const AtlasRegion& _region, const uint8_t* _bitmapBuffer);

	/// Pack the UV coordinates of the four corners of a region to a vertex buffer.
	/// v0 -- v3
	/// |     |     encoded in that order:  v0,v1,v2,v3
	/// v1 -- v2
	/// @remark The UVs are four signed short normalized components.
	/// @remark The x,y,z components encode cube UV coordinates. The w component encodes the color channel if any.
	/// @param _regionHandle Handle to the region of interest.
	/// @param _vertexBuffer Address of the first vertex to update.
	/// @param _offset Byte offset to the UV data within a vertex.
	/// @param _stride Stride between vertices in the buffer.
	void packUV(uint16_t _regionHandle, uint8_t* _vertexBuffer, uint32_t _offset, uint32_t _stride) const;
	void packUV(const AtlasRegion& _region, uint8_t* _vertexBuffer, uint32_t _offset, uint32_t _stride) const;

	/// Same as packUV but for a whole face of the atlas cube, mostly for debugging.
	void packFaceLayerUV(uint32_t _idx, uint8_t* _vertexBuffer, uint32_t _offset, uint32_t _stride) const;

	/// Return the SDL_Texture for a specific face of the cube atlas.
	SDL_Texture* getFaceTexture(uint32_t _faceIndex) const
	{
		return (_faceIndex < 6) ? m_textureHandles[_faceIndex] : nullptr;
	}

	/// Retrieve a region's metadata.
	const AtlasRegion& getRegion(uint16_t _handle) const
	{
		return m_regions[_handle];
	}

	/// Retrieve the side length of a texture face in pixels.
	uint16_t getTextureSize() const
	{
		return m_textureSize;
	}

	/// Retrieve the number of regions in the atlas.
	uint16_t getRegionCount() const
	{
		return m_regionCount;
	}

	/// Retrieve a pointer to the region buffer for serialization.
	const AtlasRegion* getRegionBuffer() const
	{
		return m_regions;
	}

	/// Retrieve the byte size of the CPU-side texture buffer.
	uint32_t getTextureBufferSize() const
	{
		return 6 * m_textureSize * m_textureSize * 4;
	}

	/// Retrieve the CPU-side texture buffer for serialization.
	const uint8_t* getTextureBuffer() const
	{
		return m_textureBuffer;
	}

	/// Get the total usage of the atlas across all faces as a percentage [0-100].
	float getTotalRegionUsage();

private:
	struct PackedLayer;
	PackedLayer* m_layers;
	AtlasRegion* m_regions;
	uint8_t* m_textureBuffer;

	uint32_t m_usedLayers;
	uint32_t m_usedFaces;

	SDL_Renderer* m_renderer;
	std::array<SDL_Texture*, 6> m_textureHandles;

	uint16_t m_textureSize;
	float m_texelSize;

	uint16_t m_regionCount;
	uint16_t m_maxRegionCount;
};

#endif // CUBE_ATLAS_H_HEADER_GUARD