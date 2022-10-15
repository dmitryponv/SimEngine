#pragma once

#include <d3d11.h>
#include <memory>

#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)

class ScratchImage
{
public:
	//Temp move to another class
	enum TEX_DIMENSION
		// Subset here matches D3D10_RESOURCE_DIMENSION and D3D11_RESOURCE_DIMENSION
	{
		TEX_DIMENSION_TEXTURE1D = 2,
		TEX_DIMENSION_TEXTURE2D = 3,
		TEX_DIMENSION_TEXTURE3D = 4,
	};
	enum TEX_MISC_FLAG
		// Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG
	{
		TEX_MISC_TEXTURECUBE = 0x4L,
	};
	enum TEX_ALPHA_MODE
		// Matches DDS_ALPHA_MODE, encoded in MISC_FLAGS2
	{
		TEX_ALPHA_MODE_UNKNOWN = 0,
		TEX_ALPHA_MODE_STRAIGHT = 1,
		TEX_ALPHA_MODE_PREMULTIPLIED = 2,
		TEX_ALPHA_MODE_OPAQUE = 3,
		TEX_ALPHA_MODE_CUSTOM = 4,
	};
	enum TEX_MISC_FLAG2
	{
		TEX_MISC2_ALPHA_MODE_MASK = 0x7L,
	};
	struct TexMetadata
	{
		size_t          width;
		size_t          height;     // Should be 1 for 1D textures
		size_t          depth;      // Should be 1 for 1D or 2D textures
		size_t          arraySize;  // For cubemap, this is a multiple of 6
		size_t          mipLevels;
		uint32_t        miscFlags;
		uint32_t        miscFlags2;
		DXGI_FORMAT     format;
		TEX_DIMENSION   dimension;

		size_t __cdecl ComputeIndex(_In_ size_t mip, _In_ size_t item, _In_ size_t slice) const;
		// Returns size_t(-1) to indicate an out-of-range error

		bool __cdecl IsCubemap() const { return (miscFlags & TEX_MISC_TEXTURECUBE) != 0; }
		// Helper for miscFlags

		bool __cdecl IsPMAlpha() const { return ((miscFlags2 & TEX_MISC2_ALPHA_MODE_MASK) == TEX_ALPHA_MODE_PREMULTIPLIED) != 0; }
		void __cdecl SetAlphaMode(TEX_ALPHA_MODE mode) { miscFlags2 = (miscFlags2 & ~static_cast<uint32_t>(TEX_MISC2_ALPHA_MODE_MASK)) | static_cast<uint32_t>(mode); }
		TEX_ALPHA_MODE __cdecl GetAlphaMode() const { return static_cast<TEX_ALPHA_MODE>(miscFlags2 & TEX_MISC2_ALPHA_MODE_MASK); }
		// Helpers for miscFlags2

		bool __cdecl IsVolumemap() const { return (dimension == TEX_DIMENSION_TEXTURE3D); }
		// Helper for dimension
	};
	struct Image
	{
		size_t      width;
		size_t      height;
		DXGI_FORMAT format;
		size_t      rowPitch;
		size_t      slicePitch;
		uint8_t*    pixels;
	};
	enum CP_FLAGS
	{
		CP_FLAGS_NONE = 0x0,      // Normal operation
		CP_FLAGS_LEGACY_DWORD = 0x1,      // Assume pitch is DWORD aligned instead of BYTE aligned
		CP_FLAGS_PARAGRAPH = 0x2,      // Assume pitch is 16-byte aligned instead of BYTE aligned
		CP_FLAGS_YMM = 0x4,      // Assume pitch is 32-byte aligned instead of BYTE aligned
		CP_FLAGS_ZMM = 0x8,      // Assume pitch is 64-byte aligned instead of BYTE aligned
		CP_FLAGS_PAGE4K = 0x200,    // Assume pitch is 4096-byte aligned instead of BYTE aligned
		CP_FLAGS_BAD_DXTN_TAILS = 0x1000,   // BC formats with malformed mipchain blocks smaller than 4x4
		CP_FLAGS_24BPP = 0x10000,  // Override with a legacy 24 bits-per-pixel format size
		CP_FLAGS_16BPP = 0x20000,  // Override with a legacy 16 bits-per-pixel format size
		CP_FLAGS_8BPP = 0x40000,  // Override with a legacy 8 bits-per-pixel format size
	};

	ScratchImage() noexcept
		: m_nimages(0), m_size(0), m_metadata{}, m_image(nullptr), m_memory(nullptr) {}
	ScratchImage(ScratchImage&& moveFrom) noexcept
		: m_nimages(0), m_size(0), m_metadata{}, m_image(nullptr), m_memory(nullptr) { *this = std::move(moveFrom); }
	~ScratchImage() { Release(); }

	ScratchImage& __cdecl operator= (ScratchImage&& moveFrom) noexcept;

	ScratchImage(const ScratchImage&) = delete;
	ScratchImage& operator=(const ScratchImage&) = delete;

	HRESULT __cdecl Initialize(_In_ const TexMetadata& mdata, _In_ DWORD flags = CP_FLAGS_NONE);

	HRESULT __cdecl Initialize1D(_In_ DXGI_FORMAT fmt, _In_ size_t length, _In_ size_t arraySize, _In_ size_t mipLevels, _In_ DWORD flags = CP_FLAGS_NONE);
	HRESULT __cdecl Initialize2D(_In_ DXGI_FORMAT fmt, _In_ size_t width, _In_ size_t height, _In_ size_t arraySize, _In_ size_t mipLevels, _In_ DWORD flags = CP_FLAGS_NONE);
	HRESULT __cdecl Initialize3D(_In_ DXGI_FORMAT fmt, _In_ size_t width, _In_ size_t height, _In_ size_t depth, _In_ size_t mipLevels, _In_ DWORD flags = CP_FLAGS_NONE);
	HRESULT __cdecl InitializeCube(_In_ DXGI_FORMAT fmt, _In_ size_t width, _In_ size_t height, _In_ size_t nCubes, _In_ size_t mipLevels, _In_ DWORD flags = CP_FLAGS_NONE);

	HRESULT __cdecl InitializeFromImage(_In_ const Image& srcImage, _In_ bool allow1D = false, _In_ DWORD flags = CP_FLAGS_NONE);
	HRESULT __cdecl InitializeArrayFromImages(_In_reads_(nImages) const Image* images, _In_ size_t nImages, _In_ bool allow1D = false, _In_ DWORD flags = CP_FLAGS_NONE);
	HRESULT __cdecl InitializeCubeFromImages(_In_reads_(nImages) const Image* images, _In_ size_t nImages, _In_ DWORD flags = CP_FLAGS_NONE);
	HRESULT __cdecl Initialize3DFromImages(_In_reads_(depth) const Image* images, _In_ size_t depth, _In_ DWORD flags = CP_FLAGS_NONE);

	void __cdecl Release();

	bool __cdecl OverrideFormat(_In_ DXGI_FORMAT f);

	const TexMetadata& __cdecl GetMetadata() const { return m_metadata; }
	const Image* __cdecl GetImage(_In_ size_t mip, _In_ size_t item, _In_ size_t slice) const;

	const Image* __cdecl GetImages() const { return m_image; }
	size_t __cdecl GetImageCount() const { return m_nimages; }

	uint8_t* __cdecl GetPixels() const { return m_memory; }
	size_t __cdecl GetPixelsSize() const { return m_size; }

	bool __cdecl IsAlphaAllOpaque() const;

private:
	size_t      m_nimages;
	size_t      m_size;
	TexMetadata m_metadata;
	Image*      m_image;
	uint8_t*    m_memory;
};

class ImageExporter
{
public:
	ImageExporter();
	~ImageExporter();



	_Use_decl_annotations_	bool IsTypeless(DXGI_FORMAT fmt, bool partialTypeless = false)
	{
		switch (static_cast<int>(fmt))
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC7_TYPELESS:
			return true;

		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		//case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
		//case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
			return partialTypeless;

		default:
			return false;
		}
	}

	_Use_decl_annotations_	DXGI_FORMAT MakeTypelessUNORM(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			return DXGI_FORMAT_R10G10B10A2_UNORM;

		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case DXGI_FORMAT_R16G16_TYPELESS:
			return DXGI_FORMAT_R16G16_UNORM;

		case DXGI_FORMAT_R8G8_TYPELESS:
			return DXGI_FORMAT_R8G8_UNORM;

		case DXGI_FORMAT_R16_TYPELESS:
			return DXGI_FORMAT_R16_UNORM;

		case DXGI_FORMAT_R8_TYPELESS:
			return DXGI_FORMAT_R8_UNORM;

		case DXGI_FORMAT_BC1_TYPELESS:
			return DXGI_FORMAT_BC1_UNORM;

		case DXGI_FORMAT_BC2_TYPELESS:
			return DXGI_FORMAT_BC2_UNORM;

		case DXGI_FORMAT_BC3_TYPELESS:
			return DXGI_FORMAT_BC3_UNORM;

		case DXGI_FORMAT_BC4_TYPELESS:
			return DXGI_FORMAT_BC4_UNORM;

		case DXGI_FORMAT_BC5_TYPELESS:
			return DXGI_FORMAT_BC5_UNORM;

		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			return DXGI_FORMAT_B8G8R8X8_UNORM;

		case DXGI_FORMAT_BC7_TYPELESS:
			return DXGI_FORMAT_BC7_UNORM;

		default:
			return fmt;
		}
	}
	_Use_decl_annotations_ DXGI_FORMAT MakeTypelessFLOAT(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
			return DXGI_FORMAT_R32G32B32_FLOAT;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case DXGI_FORMAT_R32G32_TYPELESS:
			return DXGI_FORMAT_R32G32_FLOAT;

		case DXGI_FORMAT_R16G16_TYPELESS:
			return DXGI_FORMAT_R16G16_FLOAT;

		case DXGI_FORMAT_R32_TYPELESS:
			return DXGI_FORMAT_R32_FLOAT;

		case DXGI_FORMAT_R16_TYPELESS:
			return DXGI_FORMAT_R16_FLOAT;

		default:
			return fmt;
		}
	}

	_Use_decl_annotations_	size_t ComputeScanlines(DXGI_FORMAT fmt, size_t height)
	{
		switch (static_cast<int>(fmt))
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			//return std::max<size_t>(1, (height + 3) / 4);

		case DXGI_FORMAT_NV11:
		//case WIN10_DXGI_FORMAT_P208:
		//	assert(IsPlanar(fmt));
			return height * 2;

		//case WIN10_DXGI_FORMAT_V208:
		//	assert(IsPlanar(fmt));
			return height + (((height + 1) >> 1) * 2);

		//case WIN10_DXGI_FORMAT_V408:
		//	assert(IsPlanar(fmt));
			return height + ((height >> 1) * 4);

		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
		case DXGI_FORMAT_420_OPAQUE:
		//case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
		//case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
		//case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
		//	assert(IsPlanar(fmt));
			return height + ((height + 1) >> 1);

		default:
			//assert(IsValid(fmt));
			//assert(!IsCompressed(fmt) && !IsPlanar(fmt));
			return height;
		}
	}

	HRESULT Capture(
		_In_ ID3D11DeviceContext* pContext,
		_In_ ID3D11Resource* pSource,
		const ScratchImage::TexMetadata& metadata,
		const ScratchImage& result);

	HRESULT CaptureTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11Resource* pSource, ScratchImage& result);
};

