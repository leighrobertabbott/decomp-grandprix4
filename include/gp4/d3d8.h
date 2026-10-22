// Direct3D 8 interfaces as GP4 calls them (DirectX 8.1 SDK d3d8.h / d3d8types.h / d3d8caps.h),
// declared without <windows.h>: abstract structs whose __stdcall virtual methods are in the
// SDK's vtable order, so `dev->SetRenderState(s, v)` compiles to the original's
// `mov ecx,[eax]; push v; push s; push eax; call [ecx+0xc8]`. Methods return HRESULT (int32_t)
// unless noted. The oracle models every method in gp4re/com.py; give such objects the layout
// fill `com=<Interface>` (for example the device pointer at 0x00b2e808 that GP4's renderer uses).
#pragma once
#include <gp4/com.h>

struct RECT_ { int32_t left, top, right, bottom; };
struct POINT_ { int32_t x, y; };
struct PALETTEENTRY_ { uint8_t r, g, b, flags; };

struct D3DVECTOR { float x, y, z; };
struct D3DCOLORVALUE { float r, g, b, a; };
struct D3DRECT { int32_t x1, y1, x2, y2; };
struct D3DMATRIX { float m[4][4]; };
struct D3DVIEWPORT8 { uint32_t X, Y, Width, Height; float MinZ, MaxZ; };
struct D3DMATERIAL8 { D3DCOLORVALUE Diffuse, Ambient, Specular, Emissive; float Power; };
struct D3DLIGHT8 {
    uint32_t Type;                       // D3DLIGHTTYPE
    D3DCOLORVALUE Diffuse, Specular, Ambient;
    D3DVECTOR Position, Direction;
    float Range, Falloff, Attenuation0, Attenuation1, Attenuation2, Theta, Phi;
};
struct D3DCLIPSTATUS8 { uint32_t ClipUnion, ClipIntersection; };
struct D3DRASTER_STATUS { int32_t InVBlank; uint32_t ScanLine; };
struct D3DGAMMARAMP { uint16_t red[256], green[256], blue[256]; };
struct D3DDISPLAYMODE { uint32_t Width, Height, RefreshRate, Format; };
struct D3DDEVICE_CREATION_PARAMETERS { uint32_t AdapterOrdinal, DeviceType; void* hFocusWindow; uint32_t BehaviorFlags; };
struct D3DPRESENT_PARAMETERS {
    uint32_t BackBufferWidth, BackBufferHeight, BackBufferFormat, BackBufferCount, MultiSampleType, SwapEffect;
    void* hDeviceWindow;
    int32_t Windowed, EnableAutoDepthStencil;
    uint32_t AutoDepthStencilFormat, Flags, FullScreen_RefreshRateInHz, FullScreen_PresentationInterval;
};
struct D3DLOCKED_RECT { int32_t Pitch; void* pBits; };
struct D3DLOCKED_BOX { int32_t RowPitch, SlicePitch; void* pBits; };
struct D3DBOX { uint32_t Left, Top, Right, Bottom, Front, Back; };
struct D3DSURFACE_DESC { uint32_t Format, Type, Usage, Pool, Size, MultiSampleType, Width, Height; };
struct D3DVOLUME_DESC { uint32_t Format, Type, Usage, Pool, Size, Width, Height, Depth; };
struct D3DVERTEXBUFFER_DESC { uint32_t Format, Type, Usage, Pool, Size, FVF; };
struct D3DINDEXBUFFER_DESC { uint32_t Format, Type, Usage, Pool, Size; };
struct D3DADAPTER_IDENTIFIER8 {
    char Driver[512], Description[512];
    uint32_t DriverVersionLowPart, DriverVersionHighPart;   // LARGE_INTEGER DriverVersion
    uint32_t VendorId, DeviceId, SubSysId, Revision;
    GUID_ DeviceIdentifier;
    uint32_t WHQLLevel;
};
struct D3DCAPS8 {
    uint32_t DeviceType, AdapterOrdinal, Caps, Caps2, Caps3, PresentationIntervals, CursorCaps, DevCaps,
        PrimitiveMiscCaps, RasterCaps, ZCmpCaps, SrcBlendCaps, DestBlendCaps, AlphaCmpCaps, ShadeCaps, TextureCaps,
        TextureFilterCaps, CubeTextureFilterCaps, VolumeTextureFilterCaps, TextureAddressCaps,
        VolumeTextureAddressCaps, LineCaps, MaxTextureWidth, MaxTextureHeight, MaxVolumeExtent, MaxTextureRepeat,
        MaxTextureAspectRatio, MaxAnisotropy;
    float MaxVertexW, GuardBandLeft, GuardBandTop, GuardBandRight, GuardBandBottom, ExtentsAdjust;
    uint32_t StencilCaps, FVFCaps, TextureOpCaps, MaxTextureBlendStages, MaxSimultaneousTextures,
        VertexProcessingCaps, MaxActiveLights, MaxUserClipPlanes, MaxVertexBlendMatrices, MaxVertexBlendMatrixIndex;
    float MaxPointSize;
    uint32_t MaxPrimitiveCount, MaxVertexIndex, MaxStreams, MaxStreamStride, VertexShaderVersion,
        MaxVertexShaderConst, PixelShaderVersion;
    float MaxPixelShaderValue;
};
static_assert(sizeof(D3DCAPS8) == 212, "D3DCAPS8 is 212 bytes");
static_assert(sizeof(D3DLIGHT8) == 104 && sizeof(D3DMATERIAL8) == 68 && sizeof(D3DPRESENT_PARAMETERS) == 52,
              "D3D8 structure sizes");

struct IDirect3D8;
struct IDirect3DDevice8;
struct IDirect3DSurface8;
struct IDirect3DVolume8;
struct IDirect3DSwapChain8;
struct IDirect3DBaseTexture8;
struct IDirect3DTexture8;
struct IDirect3DVolumeTexture8;
struct IDirect3DCubeTexture8;
struct IDirect3DVertexBuffer8;
struct IDirect3DIndexBuffer8;

struct IDirect3D8 : IUnknown_ {
    virtual HRESULT_ __stdcall RegisterSoftwareDevice(void* pInitializeFunction) = 0;
    virtual uint32_t __stdcall GetAdapterCount() = 0;
    virtual HRESULT_ __stdcall GetAdapterIdentifier(uint32_t Adapter, uint32_t Flags, D3DADAPTER_IDENTIFIER8* pIdentifier) = 0;
    virtual uint32_t __stdcall GetAdapterModeCount(uint32_t Adapter) = 0;
    virtual HRESULT_ __stdcall EnumAdapterModes(uint32_t Adapter, uint32_t Mode, D3DDISPLAYMODE* pMode) = 0;
    virtual HRESULT_ __stdcall GetAdapterDisplayMode(uint32_t Adapter, D3DDISPLAYMODE* pMode) = 0;
    virtual HRESULT_ __stdcall CheckDeviceType(uint32_t Adapter, uint32_t CheckType, uint32_t DisplayFormat,
                                               uint32_t BackBufferFormat, int32_t Windowed) = 0;
    virtual HRESULT_ __stdcall CheckDeviceFormat(uint32_t Adapter, uint32_t DeviceType, uint32_t AdapterFormat,
                                                 uint32_t Usage, uint32_t RType, uint32_t CheckFormat) = 0;
    virtual HRESULT_ __stdcall CheckDeviceMultiSampleType(uint32_t Adapter, uint32_t DeviceType, uint32_t SurfaceFormat,
                                                          int32_t Windowed, uint32_t MultiSampleType) = 0;
    virtual HRESULT_ __stdcall CheckDepthStencilMatch(uint32_t Adapter, uint32_t DeviceType, uint32_t AdapterFormat,
                                                      uint32_t RenderTargetFormat, uint32_t DepthStencilFormat) = 0;
    virtual HRESULT_ __stdcall GetDeviceCaps(uint32_t Adapter, uint32_t DeviceType, D3DCAPS8* pCaps) = 0;
    virtual void* __stdcall GetAdapterMonitor(uint32_t Adapter) = 0;
    virtual HRESULT_ __stdcall CreateDevice(uint32_t Adapter, uint32_t DeviceType, void* hFocusWindow, uint32_t BehaviorFlags,
                                            D3DPRESENT_PARAMETERS* pPresentationParameters,
                                            IDirect3DDevice8** ppReturnedDeviceInterface) = 0;
};

struct IDirect3DDevice8 : IUnknown_ {
    virtual HRESULT_ __stdcall TestCooperativeLevel() = 0;
    virtual uint32_t __stdcall GetAvailableTextureMem() = 0;
    virtual HRESULT_ __stdcall ResourceManagerDiscardBytes(uint32_t Bytes) = 0;
    virtual HRESULT_ __stdcall GetDirect3D(IDirect3D8** ppD3D8) = 0;
    virtual HRESULT_ __stdcall GetDeviceCaps(D3DCAPS8* pCaps) = 0;
    virtual HRESULT_ __stdcall GetDisplayMode(D3DDISPLAYMODE* pMode) = 0;
    virtual HRESULT_ __stdcall GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters) = 0;
    virtual HRESULT_ __stdcall SetCursorProperties(uint32_t XHotSpot, uint32_t YHotSpot, IDirect3DSurface8* pCursorBitmap) = 0;
    virtual void __stdcall SetCursorPosition(int32_t X, int32_t Y, uint32_t Flags) = 0;
    virtual int32_t __stdcall ShowCursor(int32_t bShow) = 0;
    virtual HRESULT_ __stdcall CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,
                                                         IDirect3DSwapChain8** pSwapChain) = 0;
    virtual HRESULT_ __stdcall Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) = 0;
    virtual HRESULT_ __stdcall Present(const RECT_* pSourceRect, const RECT_* pDestRect, void* hDestWindowOverride,
                                       const void* pDirtyRegion) = 0;
    virtual HRESULT_ __stdcall GetBackBuffer(uint32_t BackBuffer, uint32_t Type, IDirect3DSurface8** ppBackBuffer) = 0;
    virtual HRESULT_ __stdcall GetRasterStatus(D3DRASTER_STATUS* pRasterStatus) = 0;
    virtual void __stdcall SetGammaRamp(uint32_t Flags, const D3DGAMMARAMP* pRamp) = 0;
    virtual void __stdcall GetGammaRamp(D3DGAMMARAMP* pRamp) = 0;
    virtual HRESULT_ __stdcall CreateTexture(uint32_t Width, uint32_t Height, uint32_t Levels, uint32_t Usage, uint32_t Format,
                                             uint32_t Pool, IDirect3DTexture8** ppTexture) = 0;
    virtual HRESULT_ __stdcall CreateVolumeTexture(uint32_t Width, uint32_t Height, uint32_t Depth, uint32_t Levels,
                                                   uint32_t Usage, uint32_t Format, uint32_t Pool,
                                                   IDirect3DVolumeTexture8** ppVolumeTexture) = 0;
    virtual HRESULT_ __stdcall CreateCubeTexture(uint32_t EdgeLength, uint32_t Levels, uint32_t Usage, uint32_t Format,
                                                 uint32_t Pool, IDirect3DCubeTexture8** ppCubeTexture) = 0;
    virtual HRESULT_ __stdcall CreateVertexBuffer(uint32_t Length, uint32_t Usage, uint32_t FVF, uint32_t Pool,
                                                  IDirect3DVertexBuffer8** ppVertexBuffer) = 0;
    virtual HRESULT_ __stdcall CreateIndexBuffer(uint32_t Length, uint32_t Usage, uint32_t Format, uint32_t Pool,
                                                 IDirect3DIndexBuffer8** ppIndexBuffer) = 0;
    virtual HRESULT_ __stdcall CreateRenderTarget(uint32_t Width, uint32_t Height, uint32_t Format, uint32_t MultiSample,
                                                  int32_t Lockable, IDirect3DSurface8** ppSurface) = 0;
    virtual HRESULT_ __stdcall CreateDepthStencilSurface(uint32_t Width, uint32_t Height, uint32_t Format,
                                                         uint32_t MultiSample, IDirect3DSurface8** ppSurface) = 0;
    virtual HRESULT_ __stdcall CreateImageSurface(uint32_t Width, uint32_t Height, uint32_t Format,
                                                  IDirect3DSurface8** ppSurface) = 0;
    virtual HRESULT_ __stdcall CopyRects(IDirect3DSurface8* pSourceSurface, const RECT_* pSourceRectsArray, uint32_t cRects,
                                         IDirect3DSurface8* pDestinationSurface, const POINT_* pDestPointsArray) = 0;
    virtual HRESULT_ __stdcall UpdateTexture(IDirect3DBaseTexture8* pSourceTexture, IDirect3DBaseTexture8* pDestinationTexture) = 0;
    virtual HRESULT_ __stdcall GetFrontBuffer(IDirect3DSurface8* pDestSurface) = 0;
    virtual HRESULT_ __stdcall SetRenderTarget(IDirect3DSurface8* pRenderTarget, IDirect3DSurface8* pNewZStencil) = 0;
    virtual HRESULT_ __stdcall GetRenderTarget(IDirect3DSurface8** ppRenderTarget) = 0;
    virtual HRESULT_ __stdcall GetDepthStencilSurface(IDirect3DSurface8** ppZStencilSurface) = 0;
    virtual HRESULT_ __stdcall BeginScene() = 0;
    virtual HRESULT_ __stdcall EndScene() = 0;
    virtual HRESULT_ __stdcall Clear(uint32_t Count, const D3DRECT* pRects, uint32_t Flags, uint32_t Color, float Z,
                                     uint32_t Stencil) = 0;
    virtual HRESULT_ __stdcall SetTransform(uint32_t State, const D3DMATRIX* pMatrix) = 0;
    virtual HRESULT_ __stdcall GetTransform(uint32_t State, D3DMATRIX* pMatrix) = 0;
    virtual HRESULT_ __stdcall MultiplyTransform(uint32_t State, const D3DMATRIX* pMatrix) = 0;
    virtual HRESULT_ __stdcall SetViewport(const D3DVIEWPORT8* pViewport) = 0;
    virtual HRESULT_ __stdcall GetViewport(D3DVIEWPORT8* pViewport) = 0;
    virtual HRESULT_ __stdcall SetMaterial(const D3DMATERIAL8* pMaterial) = 0;
    virtual HRESULT_ __stdcall GetMaterial(D3DMATERIAL8* pMaterial) = 0;
    virtual HRESULT_ __stdcall SetLight(uint32_t Index, const D3DLIGHT8* pLight) = 0;
    virtual HRESULT_ __stdcall GetLight(uint32_t Index, D3DLIGHT8* pLight) = 0;
    virtual HRESULT_ __stdcall LightEnable(uint32_t Index, int32_t Enable) = 0;
    virtual HRESULT_ __stdcall GetLightEnable(uint32_t Index, int32_t* pEnable) = 0;
    virtual HRESULT_ __stdcall SetClipPlane(uint32_t Index, const float* pPlane) = 0;
    virtual HRESULT_ __stdcall GetClipPlane(uint32_t Index, float* pPlane) = 0;
    virtual HRESULT_ __stdcall SetRenderState(uint32_t State, uint32_t Value) = 0;
    virtual HRESULT_ __stdcall GetRenderState(uint32_t State, uint32_t* pValue) = 0;
    virtual HRESULT_ __stdcall BeginStateBlock() = 0;
    virtual HRESULT_ __stdcall EndStateBlock(uint32_t* pToken) = 0;
    virtual HRESULT_ __stdcall ApplyStateBlock(uint32_t Token) = 0;
    virtual HRESULT_ __stdcall CaptureStateBlock(uint32_t Token) = 0;
    virtual HRESULT_ __stdcall DeleteStateBlock(uint32_t Token) = 0;
    virtual HRESULT_ __stdcall CreateStateBlock(uint32_t Type, uint32_t* pToken) = 0;
    virtual HRESULT_ __stdcall SetClipStatus(const D3DCLIPSTATUS8* pClipStatus) = 0;
    virtual HRESULT_ __stdcall GetClipStatus(D3DCLIPSTATUS8* pClipStatus) = 0;
    virtual HRESULT_ __stdcall GetTexture(uint32_t Stage, IDirect3DBaseTexture8** ppTexture) = 0;
    virtual HRESULT_ __stdcall SetTexture(uint32_t Stage, IDirect3DBaseTexture8* pTexture) = 0;
    virtual HRESULT_ __stdcall GetTextureStageState(uint32_t Stage, uint32_t Type, uint32_t* pValue) = 0;
    virtual HRESULT_ __stdcall SetTextureStageState(uint32_t Stage, uint32_t Type, uint32_t Value) = 0;
    virtual HRESULT_ __stdcall ValidateDevice(uint32_t* pNumPasses) = 0;
    virtual HRESULT_ __stdcall GetInfo(uint32_t DevInfoID, void* pDevInfoStruct, uint32_t DevInfoStructSize) = 0;
    virtual HRESULT_ __stdcall SetPaletteEntries(uint32_t PaletteNumber, const PALETTEENTRY_* pEntries) = 0;
    virtual HRESULT_ __stdcall GetPaletteEntries(uint32_t PaletteNumber, PALETTEENTRY_* pEntries) = 0;
    virtual HRESULT_ __stdcall SetCurrentTexturePalette(uint32_t PaletteNumber) = 0;
    virtual HRESULT_ __stdcall GetCurrentTexturePalette(uint32_t* PaletteNumber) = 0;
    virtual HRESULT_ __stdcall DrawPrimitive(uint32_t PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount) = 0;
    virtual HRESULT_ __stdcall DrawIndexedPrimitive(uint32_t PrimitiveType, uint32_t minIndex, uint32_t NumVertices,
                                                    uint32_t startIndex, uint32_t primCount) = 0;
    virtual HRESULT_ __stdcall DrawPrimitiveUP(uint32_t PrimitiveType, uint32_t PrimitiveCount,
                                               const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride) = 0;
    virtual HRESULT_ __stdcall DrawIndexedPrimitiveUP(uint32_t PrimitiveType, uint32_t MinVertexIndex,
                                                      uint32_t NumVertexIndices, uint32_t PrimitiveCount,
                                                      const void* pIndexData, uint32_t IndexDataFormat,
                                                      const void* pVertexStreamZeroData,
                                                      uint32_t VertexStreamZeroStride) = 0;
    virtual HRESULT_ __stdcall ProcessVertices(uint32_t SrcStartIndex, uint32_t DestIndex, uint32_t VertexCount,
                                               IDirect3DVertexBuffer8* pDestBuffer, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall CreateVertexShader(const uint32_t* pDeclaration, const uint32_t* pFunction,
                                                  uint32_t* pHandle, uint32_t Usage) = 0;
    virtual HRESULT_ __stdcall SetVertexShader(uint32_t Handle) = 0;
    virtual HRESULT_ __stdcall GetVertexShader(uint32_t* pHandle) = 0;
    virtual HRESULT_ __stdcall DeleteVertexShader(uint32_t Handle) = 0;
    virtual HRESULT_ __stdcall SetVertexShaderConstant(uint32_t Register, const void* pConstantData, uint32_t ConstantCount) = 0;
    virtual HRESULT_ __stdcall GetVertexShaderConstant(uint32_t Register, void* pConstantData, uint32_t ConstantCount) = 0;
    virtual HRESULT_ __stdcall GetVertexShaderDeclaration(uint32_t Handle, void* pData, uint32_t* pSizeOfData) = 0;
    virtual HRESULT_ __stdcall GetVertexShaderFunction(uint32_t Handle, void* pData, uint32_t* pSizeOfData) = 0;
    virtual HRESULT_ __stdcall SetStreamSource(uint32_t StreamNumber, IDirect3DVertexBuffer8* pStreamData, uint32_t Stride) = 0;
    virtual HRESULT_ __stdcall GetStreamSource(uint32_t StreamNumber, IDirect3DVertexBuffer8** ppStreamData, uint32_t* pStride) = 0;
    virtual HRESULT_ __stdcall SetIndices(IDirect3DIndexBuffer8* pIndexData, uint32_t BaseVertexIndex) = 0;
    virtual HRESULT_ __stdcall GetIndices(IDirect3DIndexBuffer8** ppIndexData, uint32_t* pBaseVertexIndex) = 0;
    virtual HRESULT_ __stdcall CreatePixelShader(const uint32_t* pFunction, uint32_t* pHandle) = 0;
    virtual HRESULT_ __stdcall SetPixelShader(uint32_t Handle) = 0;
    virtual HRESULT_ __stdcall GetPixelShader(uint32_t* pHandle) = 0;
    virtual HRESULT_ __stdcall DeletePixelShader(uint32_t Handle) = 0;
    virtual HRESULT_ __stdcall SetPixelShaderConstant(uint32_t Register, const void* pConstantData, uint32_t ConstantCount) = 0;
    virtual HRESULT_ __stdcall GetPixelShaderConstant(uint32_t Register, void* pConstantData, uint32_t ConstantCount) = 0;
    virtual HRESULT_ __stdcall GetPixelShaderFunction(uint32_t Handle, void* pData, uint32_t* pSizeOfData) = 0;
    virtual HRESULT_ __stdcall DrawRectPatch(uint32_t Handle, const float* pNumSegs, const void* pRectPatchInfo) = 0;
    virtual HRESULT_ __stdcall DrawTriPatch(uint32_t Handle, const float* pNumSegs, const void* pTriPatchInfo) = 0;
    virtual HRESULT_ __stdcall DeletePatch(uint32_t Handle) = 0;
};

struct IDirect3DSwapChain8 : IUnknown_ {
    virtual HRESULT_ __stdcall Present(const RECT_* pSourceRect, const RECT_* pDestRect, void* hDestWindowOverride,
                                       const void* pDirtyRegion) = 0;
    virtual HRESULT_ __stdcall GetBackBuffer(uint32_t BackBuffer, uint32_t Type, IDirect3DSurface8** ppBackBuffer) = 0;
};

struct IDirect3DResource8 : IUnknown_ {
    virtual HRESULT_ __stdcall GetDevice(IDirect3DDevice8** ppDevice) = 0;
    virtual HRESULT_ __stdcall SetPrivateData(const GUID_* refguid, const void* pData, uint32_t SizeOfData, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall GetPrivateData(const GUID_* refguid, void* pData, uint32_t* pSizeOfData) = 0;
    virtual HRESULT_ __stdcall FreePrivateData(const GUID_* refguid) = 0;
    virtual uint32_t __stdcall SetPriority(uint32_t PriorityNew) = 0;
    virtual uint32_t __stdcall GetPriority() = 0;
    virtual void __stdcall PreLoad() = 0;
    virtual uint32_t __stdcall GetType() = 0;
};

struct IDirect3DBaseTexture8 : IDirect3DResource8 {
    virtual uint32_t __stdcall SetLOD(uint32_t LODNew) = 0;
    virtual uint32_t __stdcall GetLOD() = 0;
    virtual uint32_t __stdcall GetLevelCount() = 0;
};

struct IDirect3DTexture8 : IDirect3DBaseTexture8 {
    virtual HRESULT_ __stdcall GetLevelDesc(uint32_t Level, D3DSURFACE_DESC* pDesc) = 0;
    virtual HRESULT_ __stdcall GetSurfaceLevel(uint32_t Level, IDirect3DSurface8** ppSurfaceLevel) = 0;
    virtual HRESULT_ __stdcall LockRect(uint32_t Level, D3DLOCKED_RECT* pLockedRect, const RECT_* pRect, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall UnlockRect(uint32_t Level) = 0;
    virtual HRESULT_ __stdcall AddDirtyRect(const RECT_* pDirtyRect) = 0;
};

struct IDirect3DCubeTexture8 : IDirect3DBaseTexture8 {
    virtual HRESULT_ __stdcall GetLevelDesc(uint32_t Level, D3DSURFACE_DESC* pDesc) = 0;
    virtual HRESULT_ __stdcall GetCubeMapSurface(uint32_t FaceType, uint32_t Level, IDirect3DSurface8** ppCubeMapSurface) = 0;
    virtual HRESULT_ __stdcall LockRect(uint32_t FaceType, uint32_t Level, D3DLOCKED_RECT* pLockedRect, const RECT_* pRect,
                                        uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall UnlockRect(uint32_t FaceType, uint32_t Level) = 0;
    virtual HRESULT_ __stdcall AddDirtyRect(uint32_t FaceType, const RECT_* pDirtyRect) = 0;
};

struct IDirect3DVolumeTexture8 : IDirect3DBaseTexture8 {
    virtual HRESULT_ __stdcall GetLevelDesc(uint32_t Level, D3DVOLUME_DESC* pDesc) = 0;
    virtual HRESULT_ __stdcall GetVolumeLevel(uint32_t Level, IDirect3DVolume8** ppVolumeLevel) = 0;
    virtual HRESULT_ __stdcall LockBox(uint32_t Level, D3DLOCKED_BOX* pLockedVolume, const D3DBOX* pBox, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall UnlockBox(uint32_t Level) = 0;
    virtual HRESULT_ __stdcall AddDirtyBox(const D3DBOX* pDirtyBox) = 0;
};

struct IDirect3DVertexBuffer8 : IDirect3DResource8 {
    virtual HRESULT_ __stdcall Lock(uint32_t OffsetToLock, uint32_t SizeToLock, uint8_t** ppbData, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall Unlock() = 0;
    virtual HRESULT_ __stdcall GetDesc(D3DVERTEXBUFFER_DESC* pDesc) = 0;
};

struct IDirect3DIndexBuffer8 : IDirect3DResource8 {
    virtual HRESULT_ __stdcall Lock(uint32_t OffsetToLock, uint32_t SizeToLock, uint8_t** ppbData, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall Unlock() = 0;
    virtual HRESULT_ __stdcall GetDesc(D3DINDEXBUFFER_DESC* pDesc) = 0;
};

struct IDirect3DSurface8 : IUnknown_ {
    virtual HRESULT_ __stdcall GetDevice(IDirect3DDevice8** ppDevice) = 0;
    virtual HRESULT_ __stdcall SetPrivateData(const GUID_* refguid, const void* pData, uint32_t SizeOfData, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall GetPrivateData(const GUID_* refguid, void* pData, uint32_t* pSizeOfData) = 0;
    virtual HRESULT_ __stdcall FreePrivateData(const GUID_* refguid) = 0;
    virtual HRESULT_ __stdcall GetContainer(const GUID_* riid, void** ppContainer) = 0;
    virtual HRESULT_ __stdcall GetDesc(D3DSURFACE_DESC* pDesc) = 0;
    virtual HRESULT_ __stdcall LockRect(D3DLOCKED_RECT* pLockedRect, const RECT_* pRect, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall UnlockRect() = 0;
};

struct IDirect3DVolume8 : IUnknown_ {
    virtual HRESULT_ __stdcall GetDevice(IDirect3DDevice8** ppDevice) = 0;
    virtual HRESULT_ __stdcall SetPrivateData(const GUID_* refguid, const void* pData, uint32_t SizeOfData, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall GetPrivateData(const GUID_* refguid, void* pData, uint32_t* pSizeOfData) = 0;
    virtual HRESULT_ __stdcall FreePrivateData(const GUID_* refguid) = 0;
    virtual HRESULT_ __stdcall GetContainer(const GUID_* riid, void** ppContainer) = 0;
    virtual HRESULT_ __stdcall GetDesc(D3DVOLUME_DESC* pDesc) = 0;
    virtual HRESULT_ __stdcall LockBox(D3DLOCKED_BOX* pLockedVolume, const D3DBOX* pBox, uint32_t Flags) = 0;
    virtual HRESULT_ __stdcall UnlockBox() = 0;
};

// GP4's global Direct3D objects (verified slot by slot against their call sites, 2026-10-22)
#define GP4_D3D_DEVICE GP4_GLOBAL(IDirect3DDevice8*, 0x00b2e808)
