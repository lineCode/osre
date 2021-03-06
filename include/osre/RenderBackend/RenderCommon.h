
/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2018 OSRE ( Open Source Render Engine ) by Kim Kulling

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-----------------------------------------------------------------------------------------------*/
#pragma once

#include <osre/Common/osre_common.h>
#include <osre/RenderBackend/Parameter.h>
#include <osre/RenderBackend/FontBase.h>
#include <osre/IO/Uri.h>

#include <cppcore/Container/TArray.h>
#include <cppcore/Container/TStaticArray.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace OSRE {
namespace RenderBackend {

// Forward declarations
struct UniformVar;

/// Describes an unset id.
static const i32  UnsetHandle   = -1;

/// Upper limits for names.
static const ui32 MaxEntNameLen = 256;

///	@brief  This enum describes the usage of a buffer object.
enum class BufferType {
    EmptyBuffer = 0,    ///< Empty buffer, no special use.
    VertexBuffer,       ///< Vertex buffer, stores vertex data inside.
    IndexBuffer,        ///< Index buffer, stores indices inside.
    InstanceBuffer,     ///< Instance buffer, will store instance-specific data.
    ConstantBuffer,      ///< Uniform buffer, used for structured uniform data.
    NumBufferTypes,     ///< Number of enums.
    
    InvalidBufferType   ///< Enum for invalid enum.
};

/// @brief  This enum describes the supported access types for render buffers.
enum class BufferAccessType {
    ReadOnly = 0,               ///< Read only access.
    WriteOnly,                  ///< Write only access.
    ReadWrite,                  ///< Read and write access.
    NumBufferAccessTypes,       ///< Number of enum's.
    
    InvalidBufferAccessType     ///< Enum for invalid enum.
};

///	@brief  This enum describes the build-in vertex types provided by OSRE, mainly used for demos and examples.
enum class VertexType {
    ColorVertex = 0,            ///< A simple vertex consisting of position and color.
    RenderVertex,               ///< A render vertex with position, color, normals and texture coordinates.
    NumVertexTypes,             ///< Number of enums.
    
    InvalidVertexType           ///< Enum for invalid enum.
};

///	@brief  This enum describes the supported texture target types.
enum class TextureTargetType {
    Texture1D = 0,              ///< 1D-textures, used for simple arrays in shaders.
    Texture2D,                  ///< 2D-textures, used for images and render targets.
    Texture3D,                  ///< 3D-textures, used for volume rendering.
    NumTextureTargetTypes,      ///< Number of enums.
    
    InvalidTextureTargetType    ///< Enum for invalid enum.
};

///	@brief  This enum describes the supported texture stages. A texture stage describes one layer of 
/// a texture composition like a diffuse texture at stage0 and a lighting information at statge1.
enum class TextureStageType {
    TextureStage0 = 0,          ///< Texture state level 0
    TextureStage1,              ///< Texture state level 1
    TextureStage2,              ///< Texture state level 2
    TextureStage3,              ///< Texture state level 3
    NumTextureStageTypes,       ///< Number of enums.
    
    InvalidTextureStageType     ///< Enum for invalid enum.
};

///	@brief  This enum describes different texture parameter names.
enum class TextureParameterName {
    TextureParamMinFilter = 0,  ///< The min filter name.
    TextureParamMagFilter,      ///< The mag filter name.
    TextureParamWrapS,          ///< The wrap name for s
    TextureParamWrapT,          ///< The warp name for t.
    NumTextureParameterNames,   ///< Number of enums.
    
    InvalidTextureParameterName,///< Enum for invalid enum.
};

///	@brief  This enum describes the parameters which are related the the parameter names ( @see TextureParameterName ).
enum class TextureParameterType {
    TexturePTNearest = 0,       ///< Use nearest filter mode.
    TexturePTLinear,            ///< Use linear interpolation mode.
    TexturePTClamp,             ///< Use clamp mode, texture data will be clamped.
    TexturePTMirroredRepeat,    ///< Use mirror repeat mode, texture will be repeately mirrored.
    TexturePTRepeat,            ///< Use repeat mode, texture will be repeately mirrored.
    NumTextureParameterTypes,   ///< Number of enums.
    
    InvalidTextureParameterType,///< Enum for invalid enum.
};

///	@brief  This enum describes the index data type.
enum class IndexType {
    UnsignedByte = 0,       ///< Bytes are used for the index data.
    UnsignedShort,          ///< Unsigned short for the index data.
    UnsignedInt,            ///< Unsigned int for the index data.
    NumIndexTypes,          ///< Number of enums.
    
    InvalidIndexType,       ///< Enum for invalid enum.
};

///	@brief  This enum describes the primitive types for rendering vertex information.
enum class PrimitiveType {
    PointList = 0,          ///< A list of points, one index per point.
    LineList,		        ///< A list of separate lines, 2 indices per line.
    LineStrip,		        ///< A line strip, Start and end-index and all indices between.
    TriangleList,	        ///< A list of triangles, 3 indices per triangle.
    TriangelStrip,	        ///< A strip of triangles
    TriangleFan,	        ///< A triangle fan.
	NumPrimitiveTypes,      ///< Number of enums.
	
    InvalidPrimitiveType,   ///< Enum for invalid enum.
};

enum class MatrixType {
    Model = 0,
    View,
    Projection,
    NumMatrixTypes,
    InvalidMatrixType
};

///	@brief  This struct declares a render vertex for textured geometry.
struct OSRE_EXPORT ColorVert {
    glm::vec3 position;     ///< The position ( x|y|z )
    glm::vec3 normal;       ///< The normal vector ( x|y|z )
    glm::vec3 color0;       ///< The diffuse color ( r|g|b )

    ColorVert();

    /// @brief  Returns the number of attributes.
    static ui32 getNumAttributes();
    /// @brief  Returns the attribute array.
    static const String *getAttributes();
};

///	@brief  This struct declares a render vertex for textured geometry
struct OSRE_EXPORT RenderVert {
    glm::vec3 position; ///< The position ( x|y|z )
    glm::vec3 normal;   ///< The normal vector ( x|y|z )
    glm::vec3 color0;   ///< The diffuse color ( r|g|b )
    glm::vec2 tex0;     ///< The texture color ( r|g|b )

    RenderVert();
    ~RenderVert();
    bool operator == ( const RenderVert &rhs ) const;
    bool operator != ( const RenderVert &rhs ) const;

    /// @brief  Returns the number of attributes.
    static ui32          getNumAttributes(); 
    /// @brief  Returns the attribute array.
    static const String *getAttributes();
};

///	@brief  This enum to describes the type of the vertex attribute.
enum class VertexAttribute : int {
    Position = 0,       ///< "position"
    Normal,             ///< "normal"
    TexCoord0,          ///< "texcoord0"
    TexCoord1,          ///< "texcoord1"
    TexCoord2,          ///< "texcoord2"
    TexCoord3,          ///< "texcoord3"
    Tangent,            ///< "tangent
    Binormal,           ///< "binormal"
    Weights,            ///< "weights" (skin weights)
    Indices,            ///< "indices" (skin indices)
    Color0,             ///< "color0"
    Color1,             ///< "color1"
    Instance0,          ///< "instance0"
    Instance1,          ///< "instance1"
    Instance2,          ///< "instance2"
    Instance3,          ///< "instance3"
    NumVertexAttrs,     ///< Number of enums.

    InvalidVertexAttr   ///< Enum for invalid enum.
};

OSRE_EXPORT const String &getVertCompName( VertexAttribute attrib );

///	@brief  This enum describes the vertex data format.
enum class VertexFormat : int {
    Float,                  ///< single component float, expanded to (x, 0, 0, 1)
    Float2,                 ///< 2-component float, expanded to (x, y, 0, 1)
    Float3,                 ///< 3-component float, expanded to (x, y, z, 1)
    Float4,                 ///< 4-component float
    Byte4,                  ///< 4-component float (-128.0f..+127.0f) mapped to byte (-128..+127)
    UByte4,                 ///< 4-component float (0.0f..255.0f) mapped to byte (0..255)
    Short2,                 ///< 2-component float (-32768.0f..+32767.0f) mapped to short (-32768..+32768)
    Short4,                 ///< 4-component float (-32768.0f..+32767.0f) mapped to short (-32768..+32768)
    NumVertexFormats,       ///< Number of enums.

    InvalidVertexFormat,    ///< Enum for invalid enum.
};

///	@brief  Utility function for calculate the vertex format size.
inline
ui32 getVertexFormatSize( VertexFormat format ) {
    ui32 size( 0 );
    switch( format ) {
        case VertexFormat::Float:
            size = sizeof( f32 );
            break;
        case VertexFormat::Float2:
            size = sizeof( f32 )*2;
            break;
        case VertexFormat::Float3:
            size = sizeof( f32 )*3;
            break;
        case VertexFormat::Float4:
            size = sizeof( f32 )*4;
            break;
        case VertexFormat::Byte4:
            size = sizeof( c8 ) * 4;
            break;
        case VertexFormat::UByte4:
            size = sizeof( uc8 ) * 4;
            break;
        case VertexFormat::Short2:
            size = sizeof( ui16 ) * 2;
            break;
        case VertexFormat::Short4:
            size = sizeof( ui16 ) * 4;
            break;
        case VertexFormat::NumVertexFormats:
        case VertexFormat::InvalidVertexFormat:
            break;

        default:
            break;
    }

    return size;
}

/// @brief  This struct declares an extension description.
struct ExtensionProperty {
    c8   m_extensionName[ MaxEntNameLen ];
    ui32 m_version;

    ExtensionProperty() {
        ::memset( m_extensionName, '\0', sizeof( c8 ) * MaxEntNameLen );
        m_version = 0;
    }
};

///	@brief
struct OSRE_EXPORT VertComponent {
    VertexAttribute m_attrib;
    VertexFormat    m_format;

    VertComponent();    
    VertComponent( VertexAttribute attrib, VertexFormat format );
    ~VertComponent();

    OSRE_NON_COPYABLE( VertComponent )
};

///	@brief
struct OSRE_EXPORT VertexLayout {
    static VertComponent            ErrorComp;
    String                         *m_attributes;
    CPPCore::TArray<VertComponent*> m_components;
    CPPCore::TArray<ui32>           m_offsets;
    ui32                            m_currentOffset;
    ui32                            m_sizeInBytes;

    VertexLayout();     
    ~VertexLayout();
    void clear();
    ui32 numComponents() const;
    ui32 sizeInBytes();
    VertexLayout &add( VertComponent *comp );
    VertComponent &getAt( ui32 idx ) const;
    const String *getAttributes();

    OSRE_NON_COPYABLE( VertexLayout )
};

///	@brief  This struct is used to describe data for a GPU buffer.
struct OSRE_EXPORT BufferData {
    BufferType       m_type;    ///< The buffer type ( @see BufferType )
    void            *m_data;    ///< The buffer data
    ui32             m_size;    ///< The size of the buffer
    ui32             m_cap;
    BufferAccessType m_access;  ///< Access token ( @see BufferAccessType )

    BufferData();
    ~BufferData();
    static BufferData *alloc( BufferType type, ui32 sizeInBytes, BufferAccessType access );
	static void free( BufferData *data );
    void copyFrom( void *data, ui32 size );
    void attach( void *data, ui32 size );
    BufferType getBufferType() const;
    BufferAccessType getBufferAccessType() const;

    OSRE_NON_COPYABLE( BufferData )
};

///	@brief
struct OSRE_EXPORT PrimitiveGroup {
    PrimitiveType m_primitive;
    ui32          m_startIndex;
    ui32          m_numIndices;
    IndexType     m_indexType;

    PrimitiveGroup();
    ~PrimitiveGroup();
    void init( IndexType indexType, ui32 numPrimitives, PrimitiveType primType, ui32 startIdx );

    OSRE_NON_COPYABLE( PrimitiveGroup )
};

///	@brief  This enum describes the kind of build-in material.
enum class MaterialType {
    ShaderMaterial = 0,         ///< Material using a build-in shader assigned to its type of vertex.
    NumMaterialTypes,           ///< Number of enums.

    InvalidMaterialType         ///< Enum for invalid enum.
};

///	@brief  
struct OSRE_EXPORT Texture {
    String            m_textureName;
    IO::Uri           m_loc;
    TextureTargetType m_targetType;
    ui32              m_size;
    uc8              *m_data;
    ui32              m_width;
    ui32              m_height;
    ui32              m_channels;
    Handle            m_texHandle;

    Texture();
    ~Texture();

    OSRE_NON_COPYABLE( Texture )
};

///	@brief  This enum describes the different shader types, which are supported by the OSRE-engine.
enum class ShaderType : ui32 {
    SH_VertexShaderType = 0,    ///< The shader is a vertex shader, used for each vertex.
    SH_GeometryShaderType,      ///< The shader is a geometry shader, used for tesselation.
    SH_TesselationShaderType,   ///< The tesselation evaluation shader.
    SH_FragmentShaderType,      ///< The shader is a fragment shader, used for rasterization.
    NumShaderTypes,             ///< Number of enums.

    InvalidShaderType           ///< Enum for invalid enum.
};

static const ui32 MaxShaderTypes = static_cast<ui32>( ShaderType::NumShaderTypes );

///	@brief
struct OSRE_EXPORT Shader {
    CPPCore::TArray<String>  m_parameters;
    CPPCore::TArray<String>  m_attributes;
    String                   m_src[ MaxShaderTypes ];

    Shader();
    ~Shader();

    OSRE_NON_COPYABLE( Shader )
};

///	@brief
enum class MaterialColorType : ui32 {
    Mat_Diffuse = 0,            ///<
    Mat_Specular,               ///<
    Mat_Ambient,                ///<
    Mat_Emission,               ///<
    NumMaterialColorTypes,      ///< Number of enums.

    InvalidMaterialColorType    ///< Enum for invalid enum.
};

static const ui32 MaxMatColorType = static_cast<ui32>( MaterialColorType::NumMaterialColorTypes );

using ShaderSourceArray = CPPCore::TStaticArray<String, MaxShaderTypes>;

///	@brief
struct OSRE_EXPORT Material {
    String        m_name;
    MaterialType  m_type;
    ui32          m_numTextures;
    Texture     **m_textures;
    Shader       *m_shader;
    ui32          m_numParameters;
    UniformVar   *m_parameters;
    Color4        m_color[ MaxMatColorType ];
    
    Material( const String &name );
    Material( const String &name, MaterialType type );
    ~Material();
    void createShader(ShaderSourceArray &shaders );

    OSRE_NON_COPYABLE( Material )
};

///	@brief
///	@brief
struct OSRE_EXPORT GeoInstanceData {
    BufferData *m_data;

    GeoInstanceData();
    ~GeoInstanceData();

    OSRE_NON_COPYABLE( GeoInstanceData )
};

///	@brief
struct OSRE_EXPORT TransformState {
    glm::vec3 m_translate;
    glm::vec3 m_scale;
    glm::vec4 m_rotation;

    TransformState();
    ~TransformState();
    void setTranslation( f32 x, f32 y, f32 z );
    void setScale( f32 sx, f32 sy, f32 sz );
    void toMatrix(glm::mat4 &m) const;
    bool operator == ( const TransformState &rhs ) const;
    bool operator != ( const TransformState &rhs ) const;
};

struct OSRE_EXPORT UniformTransformBuffer {
    glm::mat4 m_projection;
    glm::mat4 m_model;
    glm::mat4 m_view;
};

///	@brief
struct OSRE_EXPORT TransformMatrixBlock {
    glm::mat4 m_projection;
    glm::mat4 m_model;
    glm::mat4 m_view;
    glm::mat4 m_mvp;

    TransformMatrixBlock();
    ~TransformMatrixBlock();
    void init();
    void update();
    const float *getMVP();

    OSRE_NON_COPYABLE( TransformMatrixBlock )
};

///	@brief
struct OSRE_EXPORT Viewport {
	i32 m_x;
	i32 m_y;
	i32 m_w;
	i32 m_h;

	Viewport();
	Viewport( i32 x, i32 y, i32 w, i32 h );
	~Viewport();
	bool operator == (const Viewport &rhs) const;
    bool operator != ( const Viewport &rhs ) const;
	
	OSRE_NON_COPYABLE( Viewport )
};

///	@brief
struct Canvas {
    ui32 m_x;
    ui32 m_y;
    ui32 m_width;
    ui32 m_height;
    ui32 m_z;

    OSRE_NON_COPYABLE( Canvas )
};

template<class T>
struct TVertexCache {
    using CacheType = ::CPPCore::TArray<T>;
    CacheType m_cache;

    TVertexCache( ui32 size )
    : m_cache( size ) {
        // empty
    }

    TVertexCache()
    : m_cache() {
        // empty
    }

    ~TVertexCache() {
        // empty
    }

    void clear() {
        m_cache.clear();
    }

    void increaseSize(ui32 newSize) {
        m_cache.reserve(m_cache.size() + newSize);
    }

    void add( const T &vertex ) {
        m_cache.add( vertex );
    }

    ui32 numVertices() const {
        return m_cache.size();
    }

    ui32 sizeInBytes() const {
        return m_cache.size() * sizeof( T );
    }
};

template<class T>
struct TIndexCache {
    using CacheType = ::CPPCore::TArray<T>;
    CacheType m_cache;

    TIndexCache( ui32 size ) 
    : m_cache( size ) {
        // empty
    }

    TIndexCache()
    : m_cache() {
        // empty
    }

    ~TIndexCache() {
        // empty
    }

    void clear() {
        m_cache.clear();
    }

    void increaseSize( ui32 newSize ) {
        m_cache.reserve(m_cache.size() + newSize);
    }

    void add( const T &vertex ) {
        m_cache.add( vertex );
    }

    ui32 numIndices() const {
        return m_cache.size();
    }

    ui32 sizeInBytes() const {
        return m_cache.size() * sizeof( T );
    }
};

///	@brief
struct RenderBatch {
    glm::mat4  m_model;
    ui32       m_numGeo;
    Geometry  *m_geoArray;

    RenderBatch();
    ~RenderBatch();

    OSRE_NON_COPYABLE( RenderBatch )
};

enum class LightType {
    Directional,
    Point,
    Spot,
    None
};

///	@brief
struct OSRE_EXPORT Light {
    glm::vec4 m_position;
    glm::vec3 m_specular;
    glm::vec3 m_diffuse;
    glm::vec3 m_ambient;
    glm::vec4 m_direction;
    f32       m_specularExp;
    LightType m_type;

    Light();
    ~Light();
};

} // Namespace RenderBackend
} // Namespace OSRE
