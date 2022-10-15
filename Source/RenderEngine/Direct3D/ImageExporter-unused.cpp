#include "ImageExporter.h"



ImageExporter::ImageExporter()
{
}


ImageExporter::~ImageExporter()
{
}

_Use_decl_annotations_ HRESULT ImageExporter::CaptureTexture(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, ID3D11Resource * pSource, ImageExporter::ScratchImage & result)
{

	if (!pDevice || !pContext || !pSource)
		return E_INVALIDARG;

	D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	pSource->GetType(&resType);

	HRESULT hr = E_UNEXPECTED;

	switch (resType)
	{
	case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
	{
		ID3D11Texture1D* pTexture;
		hr = pSource->QueryInterface(IID_GRAPHICS_PPV_ARGS(&pTexture));
		if (FAILED(hr))
			break;

		if (pTexture == NULL)
			return 0;

		D3D11_TEXTURE1D_DESC desc;
		pTexture->GetDesc(&desc);

		ID3D11Texture1D* pStaging;
		if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
		{
			// Handle case where the source is already a staging texture we can use directly
			pStaging = pTexture;
		}
		else
		{
			desc.BindFlags = 0;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.Usage = D3D11_USAGE_STAGING;

			hr = pDevice->CreateTexture1D(&desc, nullptr, &pStaging);
			if (FAILED(hr))
				break;

			if (pStaging == NULL)
				return 0;

			pContext->CopyResource(pStaging, pSource);
		}

		TexMetadata mdata;
		mdata.width = desc.Width;
		mdata.height = mdata.depth = 1;
		mdata.arraySize = desc.ArraySize;
		mdata.mipLevels = desc.MipLevels;
		mdata.miscFlags = 0;
		mdata.miscFlags2 = 0;
		mdata.format = desc.Format;
		mdata.dimension = TEX_DIMENSION_TEXTURE1D;

		hr = result.Initialize(mdata);
		if (FAILED(hr))
			break;

		hr = Capture(pContext, pStaging, mdata, result);
	}
	break;

	case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
	{
		ID3D11Texture2D* pTexture;
		hr = pSource->QueryInterface(IID_GRAPHICS_PPV_ARGS(&pTexture));
		if (FAILED(hr))
			break;
		
		if (pTexture == NULL)
			return 0;

		D3D11_TEXTURE2D_DESC desc;
		pTexture->GetDesc(&desc);

		ID3D11Texture2D* pStaging;
		if (desc.SampleDesc.Count > 1)
		{
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;

			ID3D11Texture2D* pTemp;
			hr = pDevice->CreateTexture2D(&desc, nullptr, &pTemp);
			if (FAILED(hr))
				break;
			
			if (pTemp == NULL)
				return 0;

			DXGI_FORMAT fmt = desc.Format;
			if (IsTypeless(fmt))
			{
				// Assume a UNORM if it exists otherwise use FLOAT
				fmt = MakeTypelessUNORM(fmt);
				fmt = MakeTypelessFLOAT(fmt);
			}

			UINT support = 0;
			hr = pDevice->CheckFormatSupport(fmt, &support);
			if (FAILED(hr))
				break;

			if (!(support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE))
			{
				hr = E_FAIL;
				break;
			}

			for (UINT item = 0; item < desc.ArraySize; ++item)
			{
				for (UINT level = 0; level < desc.MipLevels; ++level)
				{
					UINT index = D3D11CalcSubresource(level, item, desc.MipLevels);
					pContext->ResolveSubresource(pTemp, index, pSource, index, fmt);
				}
			}

			desc.BindFlags = 0;
			desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.Usage = D3D11_USAGE_STAGING;

			hr = pDevice->CreateTexture2D(&desc, nullptr, &pStaging);
			if (FAILED(hr))
				break;

			if (pStaging == NULL)
				return 0;

			pContext->CopyResource(pStaging, pTemp);
		}
		else if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
		{
			// Handle case where the source is already a staging texture we can use directly
			pStaging = pTexture;
		}
		else
		{
			desc.BindFlags = 0;
			desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.Usage = D3D11_USAGE_STAGING;

			hr = pDevice->CreateTexture2D(&desc, nullptr, &pStaging);
			if (FAILED(hr))
				break;

			if (pStaging == NULL)
				return 0;

			pContext->CopyResource(pStaging, pSource);
		}

		TexMetadata mdata;
		mdata.width = desc.Width;
		mdata.height = desc.Height;
		mdata.depth = 1;
		mdata.arraySize = desc.ArraySize;
		mdata.mipLevels = desc.MipLevels;
		mdata.miscFlags = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) ? TEX_MISC_TEXTURECUBE : 0u;
		mdata.miscFlags2 = 0;
		mdata.format = desc.Format;
		mdata.dimension = TEX_DIMENSION_TEXTURE2D;

		hr = result.Initialize(mdata);
		if (FAILED(hr))
			break;

		hr = Capture(pContext, pStaging, mdata, result);
	}
	break;

	/*
	case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
	{
		ComPtr<ID3D11Texture3D> pTexture;
		hr = pSource->QueryInterface(IID_GRAPHICS_PPV_ARGS(pTexture.GetAddressOf()));
		if (FAILED(hr))
			break;

		assert(pTexture);

		D3D11_TEXTURE3D_DESC desc;
		pTexture->GetDesc(&desc);

		ComPtr<ID3D11Texture3D> pStaging;
		if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
		{
			// Handle case where the source is already a staging texture we can use directly
			pStaging = pTexture;
		}
		else
		{
			desc.BindFlags = 0;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.Usage = D3D11_USAGE_STAGING;

			hr = pDevice->CreateTexture3D(&desc, nullptr, pStaging.GetAddressOf());
			if (FAILED(hr))
				break;

			assert(pStaging);

			pContext->CopyResource(pStaging.Get(), pSource);
		}

		TexMetadata mdata;
		mdata.width = desc.Width;
		mdata.height = desc.Height;
		mdata.depth = desc.Depth;
		mdata.arraySize = 1;
		mdata.mipLevels = desc.MipLevels;
		mdata.miscFlags = 0;
		mdata.miscFlags2 = 0;
		mdata.format = desc.Format;
		mdata.dimension = TEX_DIMENSION_TEXTURE3D;

		hr = result.Initialize(mdata);
		if (FAILED(hr))
			break;

		hr = Capture(pContext, pStaging.Get(), mdata, result);
	}
	break;
	*/
	default:
		hr = E_FAIL;
		break;
	}

	if (FAILED(hr))
	{
		result.Release();
		return hr;
	}

	return S_OK;
}

HRESULT ImageExporter::Capture(
	_In_ ID3D11DeviceContext* pContext,
	_In_ ID3D11Resource* pSource,
	const ScratchImage::TexMetadata& metadata,
	const ScratchImage& result)
{
	if (!pContext || !pSource || !result.GetPixels())
		return E_POINTER;

#if defined(_XBOX_ONE) && defined(_TITLE)

	ComPtr<ID3D11Device> d3dDevice;
	pContext->GetDevice(d3dDevice.GetAddressOf());

	if (d3dDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_IMMEDIATE_CONTEXT_FAST_SEMANTICS)
	{
		ComPtr<ID3D11DeviceX> d3dDeviceX;
		HRESULT hr = d3dDevice.As(&d3dDeviceX);
		if (FAILED(hr))
			return hr;

		ComPtr<ID3D11DeviceContextX> d3dContextX;
		hr = pContext->QueryInterface(IID_GRAPHICS_PPV_ARGS(d3dContextX.GetAddressOf()));
		if (FAILED(hr))
			return hr;

		UINT64 copyFence = d3dContextX->InsertFence(0);

		while (d3dDeviceX->IsFencePending(copyFence))
		{
			SwitchToThread();
		}
	}

#endif

	if (metadata.IsVolumemap())
	{
		//--- Volume texture ----------------------------------------------------------
		if (metadata.arraySize != 1)
			return 0;

		size_t height = metadata.height;
		size_t depth = metadata.depth;

		for (size_t level = 0; level < metadata.mipLevels; ++level)
		{
			UINT dindex = D3D11CalcSubresource(static_cast<UINT>(level), 0, static_cast<UINT>(metadata.mipLevels));

			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = pContext->Map(pSource, dindex, D3D11_MAP_READ, 0, &mapped);
			if (FAILED(hr))
				return hr;

			auto pslice = static_cast<const uint8_t*>(mapped.pData);
			if (!pslice)
			{
				pContext->Unmap(pSource, dindex);
				return E_POINTER;
			}

			size_t lines = ComputeScanlines(metadata.format, height);
			if (!lines)
			{
				pContext->Unmap(pSource, dindex);
				return E_UNEXPECTED;
			}

			for (size_t slice = 0; slice < depth; ++slice)
			{
				const ScratchImage::Image* img = result.GetImage(level, 0, slice);
				if (!img)
				{
					pContext->Unmap(pSource, dindex);
					return E_FAIL;
				}

				if (!img->pixels)
				{
					pContext->Unmap(pSource, dindex);
					return E_POINTER;
				}

				const uint8_t* sptr = pslice;
				uint8_t* dptr = img->pixels;
				for (size_t h = 0; h < lines; ++h)
				{
					size_t msize = min(img->rowPitch, mapped.RowPitch);
					memcpy_s(dptr, img->rowPitch, sptr, msize);
					sptr += mapped.RowPitch;
					dptr += img->rowPitch;
				}

				pslice += mapped.DepthPitch;
			}

			pContext->Unmap(pSource, dindex);

			if (height > 1)
				height >>= 1;
			if (depth > 1)
				depth >>= 1;
		}
	}
	else
	{
		//--- 1D or 2D texture --------------------------------------------------------
		if (metadata.depth != 1)
			return 0;

		for (size_t item = 0; item < metadata.arraySize; ++item)
		{
			size_t height = metadata.height;

			for (size_t level = 0; level < metadata.mipLevels; ++level)
			{
				UINT dindex = D3D11CalcSubresource(static_cast<UINT>(level), static_cast<UINT>(item), static_cast<UINT>(metadata.mipLevels));

				D3D11_MAPPED_SUBRESOURCE mapped;
				HRESULT hr = pContext->Map(pSource, dindex, D3D11_MAP_READ, 0, &mapped);
				if (FAILED(hr))
					return hr;

				const ScratchImage::Image* img = result.GetImage(level, item, 0);
				if (!img)
				{
					pContext->Unmap(pSource, dindex);
					return E_FAIL;
				}

				if (!img->pixels)
				{
					pContext->Unmap(pSource, dindex);
					return E_POINTER;
				}

				size_t lines = ComputeScanlines(metadata.format, height);
				if (!lines)
				{
					pContext->Unmap(pSource, dindex);
					return E_UNEXPECTED;
				}

				auto sptr = static_cast<const uint8_t*>(mapped.pData);
				uint8_t* dptr = img->pixels;
				for (size_t h = 0; h < lines; ++h)
				{
					size_t msize = min(img->rowPitch, mapped.RowPitch);
					memcpy_s(dptr, img->rowPitch, sptr, msize);
					sptr += mapped.RowPitch;
					dptr += img->rowPitch;
				}

				pContext->Unmap(pSource, dindex);

				if (height > 1)
					height >>= 1;
			}
		}
	}

	return S_OK;
}


//=====================================================================================
// ScratchImage - Bitmap image container
//=====================================================================================

ScratchImage& ScratchImage::operator= (ScratchImage&& moveFrom) noexcept
{
	if (this != &moveFrom)
	{
		Release();

		m_nimages = moveFrom.m_nimages;
		m_size = moveFrom.m_size;
		m_metadata = moveFrom.m_metadata;
		m_image = moveFrom.m_image;
		m_memory = moveFrom.m_memory;

		moveFrom.m_nimages = 0;
		moveFrom.m_size = 0;
		moveFrom.m_image = nullptr;
		moveFrom.m_memory = nullptr;
	}
	return *this;
}


//-------------------------------------------------------------------------------------
// Methods
//-------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT ScratchImage::Initialize(const TexMetadata& mdata, DWORD flags)
{
	if (!IsValid(mdata.format))
		return E_INVALIDARG;

	if (IsPalettized(mdata.format))
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

	size_t mipLevels = mdata.mipLevels;

	switch (mdata.dimension)
	{
	case TEX_DIMENSION_TEXTURE1D:
		if (!mdata.width || mdata.height != 1 || mdata.depth != 1 || !mdata.arraySize)
			return E_INVALIDARG;

		if (!_CalculateMipLevels(mdata.width, 1, mipLevels))
			return E_INVALIDARG;
		break;

	case TEX_DIMENSION_TEXTURE2D:
		if (!mdata.width || !mdata.height || mdata.depth != 1 || !mdata.arraySize)
			return E_INVALIDARG;

		if (mdata.IsCubemap())
		{
			if ((mdata.arraySize % 6) != 0)
				return E_INVALIDARG;
		}

		if (!_CalculateMipLevels(mdata.width, mdata.height, mipLevels))
			return E_INVALIDARG;
		break;

	//case TEX_DIMENSION_TEXTURE3D:
	//	if (!mdata.width || !mdata.height || !mdata.depth || mdata.arraySize != 1)
	//		return E_INVALIDARG;
	//
	//	if (!_CalculateMipLevels3D(mdata.width, mdata.height, mdata.depth, mipLevels))
	//		return E_INVALIDARG;
	//	break;

	default:
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	Release();

	m_metadata.width = mdata.width;
	m_metadata.height = mdata.height;
	m_metadata.depth = mdata.depth;
	m_metadata.arraySize = mdata.arraySize;
	m_metadata.mipLevels = mipLevels;
	m_metadata.miscFlags = mdata.miscFlags;
	m_metadata.miscFlags2 = mdata.miscFlags2;
	m_metadata.format = mdata.format;
	m_metadata.dimension = mdata.dimension;

	size_t pixelSize, nimages;
	if (!_DetermineImageArray(m_metadata, flags, nimages, pixelSize))
		return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

	m_image = new (std::nothrow) Image[nimages];
	if (!m_image)
		return E_OUTOFMEMORY;

	m_nimages = nimages;
	memset(m_image, 0, sizeof(Image) * nimages);

	m_memory = static_cast<uint8_t*>(_aligned_malloc(pixelSize, 16));
	if (!m_memory)
	{
		Release();
		return E_OUTOFMEMORY;
	}
	m_size = pixelSize;
	if (!_SetupImageArray(m_memory, pixelSize, m_metadata, flags, m_image, nimages))
	{
		Release();
		return E_FAIL;
	}

	return S_OK;
}

_Use_decl_annotations_
HRESULT ScratchImage::Initialize1D(DXGI_FORMAT fmt, size_t length, size_t arraySize, size_t mipLevels, DWORD flags)
{
	if (!length || !arraySize)
		return E_INVALIDARG;

	// 1D is a special case of the 2D case
	HRESULT hr = Initialize2D(fmt, length, 1, arraySize, mipLevels, flags);
	if (FAILED(hr))
		return hr;

	m_metadata.dimension = TEX_DIMENSION_TEXTURE1D;

	return S_OK;
}

_Use_decl_annotations_
HRESULT ScratchImage::Initialize2D(DXGI_FORMAT fmt, size_t width, size_t height, size_t arraySize, size_t mipLevels, DWORD flags)
{
	if (!IsValid(fmt) || !width || !height || !arraySize)
		return E_INVALIDARG;

	if (IsPalettized(fmt))
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

	if (!_CalculateMipLevels(width, height, mipLevels))
		return E_INVALIDARG;

	Release();

	m_metadata.width = width;
	m_metadata.height = height;
	m_metadata.depth = 1;
	m_metadata.arraySize = arraySize;
	m_metadata.mipLevels = mipLevels;
	m_metadata.miscFlags = 0;
	m_metadata.miscFlags2 = 0;
	m_metadata.format = fmt;
	m_metadata.dimension = TEX_DIMENSION_TEXTURE2D;

	size_t pixelSize, nimages;
	if (!_DetermineImageArray(m_metadata, flags, nimages, pixelSize))
		return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

	m_image = new (std::nothrow) Image[nimages];
	if (!m_image)
		return E_OUTOFMEMORY;

	m_nimages = nimages;
	memset(m_image, 0, sizeof(Image) * nimages);

	m_memory = static_cast<uint8_t*>(_aligned_malloc(pixelSize, 16));
	if (!m_memory)
	{
		Release();
		return E_OUTOFMEMORY;
	}
	m_size = pixelSize;
	if (!_SetupImageArray(m_memory, pixelSize, m_metadata, flags, m_image, nimages))
	{
		Release();
		return E_FAIL;
	}

	return S_OK;
}

_Use_decl_annotations_
HRESULT ScratchImage::Initialize3D(DXGI_FORMAT fmt, size_t width, size_t height, size_t depth, size_t mipLevels, DWORD flags)
{
	if (!IsValid(fmt) || !width || !height || !depth)
		return E_INVALIDARG;

	if (IsPalettized(fmt))
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

	if (!_CalculateMipLevels3D(width, height, depth, mipLevels))
		return E_INVALIDARG;

	Release();

	m_metadata.width = width;
	m_metadata.height = height;
	m_metadata.depth = depth;
	m_metadata.arraySize = 1;    // Direct3D 10.x/11 does not support arrays of 3D textures
	m_metadata.mipLevels = mipLevels;
	m_metadata.miscFlags = 0;
	m_metadata.miscFlags2 = 0;
	m_metadata.format = fmt;
	m_metadata.dimension = TEX_DIMENSION_TEXTURE3D;

	size_t pixelSize, nimages;
	if (!_DetermineImageArray(m_metadata, flags, nimages, pixelSize))
		return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

	m_image = new (std::nothrow) Image[nimages];
	if (!m_image)
	{
		Release();
		return E_OUTOFMEMORY;
	}
	m_nimages = nimages;
	memset(m_image, 0, sizeof(Image) * nimages);

	m_memory = static_cast<uint8_t*>(_aligned_malloc(pixelSize, 16));
	if (!m_memory)
	{
		Release();
		return E_OUTOFMEMORY;
	}
	m_size = pixelSize;

	if (!_SetupImageArray(m_memory, pixelSize, m_metadata, flags, m_image, nimages))
	{
		Release();
		return E_FAIL;
	}

	return S_OK;
}

_Use_decl_annotations_
HRESULT ScratchImage::InitializeCube(DXGI_FORMAT fmt, size_t width, size_t height, size_t nCubes, size_t mipLevels, DWORD flags)
{
	if (!width || !height || !nCubes)
		return E_INVALIDARG;

	// A DirectX11 cubemap is just a 2D texture array that is a multiple of 6 for each cube
	HRESULT hr = Initialize2D(fmt, width, height, nCubes * 6, mipLevels, flags);
	if (FAILED(hr))
		return hr;

	m_metadata.miscFlags |= TEX_MISC_TEXTURECUBE;

	return S_OK;
}

_Use_decl_annotations_
HRESULT ScratchImage::InitializeFromImage(const Image& srcImage, bool allow1D, DWORD flags)
{
	HRESULT hr = (srcImage.height > 1 || !allow1D)
		? Initialize2D(srcImage.format, srcImage.width, srcImage.height, 1, 1, flags)
		: Initialize1D(srcImage.format, srcImage.width, 1, 1, flags);

	if (FAILED(hr))
		return hr;

	size_t rowCount = ComputeScanlines(srcImage.format, srcImage.height);
	if (!rowCount)
		return E_UNEXPECTED;

	const uint8_t* sptr = srcImage.pixels;
	if (!sptr)
		return E_POINTER;

	uint8_t* dptr = m_image[0].pixels;
	if (!dptr)
		return E_POINTER;

	size_t spitch = srcImage.rowPitch;
	size_t dpitch = m_image[0].rowPitch;

	size_t size = min(dpitch, spitch);

	for (size_t y = 0; y < rowCount; ++y)
	{
		memcpy_s(dptr, dpitch, sptr, size);
		sptr += spitch;
		dptr += dpitch;
	}

	return S_OK;
}

_Use_decl_annotations_
HRESULT ScratchImage::InitializeArrayFromImages(const Image* images, size_t nImages, bool allow1D, DWORD flags)
{
	if (!images || !nImages)
		return E_INVALIDARG;

	DXGI_FORMAT format = images[0].format;
	size_t width = images[0].width;
	size_t height = images[0].height;

	for (size_t index = 0; index < nImages; ++index)
	{
		if (!images[index].pixels)
			return E_POINTER;

		if (images[index].format != format || images[index].width != width || images[index].height != height)
		{
			// All images must be the same format, width, and height
			return E_FAIL;
		}
	}

	HRESULT hr = (height > 1 || !allow1D)
		? Initialize2D(format, width, height, nImages, 1, flags)
		: Initialize1D(format, width, nImages, 1, flags);

	if (FAILED(hr))
		return hr;

	size_t rowCount = ComputeScanlines(format, height);
	if (!rowCount)
		return E_UNEXPECTED;

	for (size_t index = 0; index < nImages; ++index)
	{
		const uint8_t* sptr = images[index].pixels;
		if (!sptr)
			return E_POINTER;

		//assert(index < m_nimages);
		uint8_t* dptr = m_image[index].pixels;
		if (!dptr)
			return E_POINTER;

		size_t spitch = images[index].rowPitch;
		size_t dpitch = m_image[index].rowPitch;

		size_t size = std::min<size_t>(dpitch, spitch);

		for (size_t y = 0; y < rowCount; ++y)
		{
			memcpy_s(dptr, dpitch, sptr, size);
			sptr += spitch;
			dptr += dpitch;
		}
	}

	return S_OK;
}

_Use_decl_annotations_
HRESULT ScratchImage::InitializeCubeFromImages(const Image* images, size_t nImages, DWORD flags)
{
	if (!images || !nImages)
		return E_INVALIDARG;

	// A DirectX11 cubemap is just a 2D texture array that is a multiple of 6 for each cube
	if ((nImages % 6) != 0)
		return E_INVALIDARG;

	HRESULT hr = InitializeArrayFromImages(images, nImages, false, flags);
	if (FAILED(hr))
		return hr;

	m_metadata.miscFlags |= TEX_MISC_TEXTURECUBE;

	return S_OK;
}

_Use_decl_annotations_
HRESULT ScratchImage::Initialize3DFromImages(const Image* images, size_t depth, DWORD flags)
{
	if (!images || !depth)
		return E_INVALIDARG;

	DXGI_FORMAT format = images[0].format;
	size_t width = images[0].width;
	size_t height = images[0].height;

	for (size_t slice = 0; slice < depth; ++slice)
	{
		if (!images[slice].pixels)
			return E_POINTER;

		if (images[slice].format != format || images[slice].width != width || images[slice].height != height)
		{
			// All images must be the same format, width, and height
			return E_FAIL;
		}
	}

	HRESULT hr = Initialize3D(format, width, height, depth, 1, flags);
	if (FAILED(hr))
		return hr;

	size_t rowCount = ComputeScanlines(format, height);
	if (!rowCount)
		return E_UNEXPECTED;

	for (size_t slice = 0; slice < depth; ++slice)
	{
		const uint8_t* sptr = images[slice].pixels;
		if (!sptr)
			return E_POINTER;

		//assert(slice < m_nimages);
		uint8_t* dptr = m_image[slice].pixels;
		if (!dptr)
			return E_POINTER;

		size_t spitch = images[slice].rowPitch;
		size_t dpitch = m_image[slice].rowPitch;

		size_t size = std::min<size_t>(dpitch, spitch);

		for (size_t y = 0; y < rowCount; ++y)
		{
			memcpy_s(dptr, dpitch, sptr, size);
			sptr += spitch;
			dptr += dpitch;
		}
	}

	return S_OK;
}

void ScratchImage::Release()
{
	m_nimages = 0;
	m_size = 0;

	if (m_image)
	{
		delete[] m_image;
		m_image = nullptr;
	}

	if (m_memory)
	{
		_aligned_free(m_memory);
		m_memory = nullptr;
	}

	memset(&m_metadata, 0, sizeof(m_metadata));
}

_Use_decl_annotations_
bool ScratchImage::OverrideFormat(DXGI_FORMAT f)
{
	if (!m_image)
		return false;

	if (!IsValid(f) || IsPlanar(f) || IsPalettized(f))
		return false;

	for (size_t index = 0; index < m_nimages; ++index)
	{
		m_image[index].format = f;
	}

	m_metadata.format = f;

	return true;
}

_Use_decl_annotations_
const ScratchImage::Image* ScratchImage::GetImage(size_t mip, size_t item, size_t slice) const
{
	if (mip >= m_metadata.mipLevels)
		return nullptr;

	size_t index = 0;

	switch (m_metadata.dimension)
	{
	case TEX_DIMENSION_TEXTURE1D:
	case TEX_DIMENSION_TEXTURE2D:
		if (slice > 0)
			return nullptr;

		if (item >= m_metadata.arraySize)
			return nullptr;

		index = item*(m_metadata.mipLevels) + mip;
		break;

	case TEX_DIMENSION_TEXTURE3D:
		if (item > 0)
		{
			// No support for arrays of volumes
			return nullptr;
		}
		else
		{
			size_t d = m_metadata.depth;

			for (size_t level = 0; level < mip; ++level)
			{
				index += d;
				if (d > 1)
					d >>= 1;
			}

			if (slice >= d)
				return nullptr;

			index += slice;
		}
		break;

	default:
		return nullptr;
	}

	return &m_image[index];
}

bool ScratchImage::IsAlphaAllOpaque() const
{
	if (!m_image)
		return false;

	if (!HasAlpha(m_metadata.format))
		return true;

	if (IsCompressed(m_metadata.format))
	{
		for (size_t index = 0; index < m_nimages; ++index)
		{
			if (!_IsAlphaAllOpaqueBC(m_image[index]))
				return false;
		}
	}
	else
	{
		ScopedAlignedArrayXMVECTOR scanline(static_cast<XMVECTOR*>(_aligned_malloc((sizeof(XMVECTOR)*m_metadata.width), 16)));
		if (!scanline)
			return false;

		static const XMVECTORF32 threshold = { { { 0.997f, 0.997f, 0.997f, 0.997f } } };

		for (size_t index = 0; index < m_nimages; ++index)
		{
#pragma warning( suppress : 6011 )
			const Image& img = m_image[index];

			const uint8_t *pPixels = img.pixels;
			//assert(pPixels);

			for (size_t h = 0; h < img.height; ++h)
			{
				if (!_LoadScanline(scanline.get(), img.width, pPixels, img.rowPitch, img.format))
					return false;

				const XMVECTOR* ptr = scanline.get();
				for (size_t w = 0; w < img.width; ++w)
				{
					XMVECTOR alpha = XMVectorSplatW(*ptr);
					if (XMVector4Less(alpha, threshold))
						return false;
					++ptr;
				}

				pPixels += img.rowPitch;
			}
		}
	}

	return true;
}