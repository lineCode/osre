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
#include <osre/RenderBackend/RenderCommon.h>
#include <osre/Common/Logger.h>
#include <osre/Common/Ids.h>
#include <glm/gtc/matrix_transform.inl>

namespace OSRE {
namespace RenderBackend {

using namespace ::OSRE::Common;
using namespace ::glm;

VertComponent VertexLayout::ErrorComp;

// The log tag for messages
static const String Tag = "RenderCommon";

/// @brief  The corresponding names for vertex components in a vertex layout
static const String VertCompName[ static_cast<ui32>(VertexAttribute::NumVertexAttrs) ] = {
    "position",     ///< Position
    "normal",       ///< Normal
    "texcoord0",    ///< TexCoord0
    "texcoord1",    ///< TexCoord1
    "texcoord2",    ///< TexCoord2
    "texcoord3",    ///< TexCoord3
    "tangent",      ///< Tangent
    "binormal",     ///< Binormal
    "weights",      ///< Weights
    "indices",      ///< Indices
    "color0",       ///< Color0
    "color1",       ///< Color1
    "instance0",    ///< Instance0
    "instance1",    ///< Instance1
    "instance2",    ///< Instance2
    "instance3"     ///< Instance3
};

static const String ErrorCmpName = "Error";

// List of attributes for color vertices
static const ui32 NumColorVertAttributes = 3;
static const String ColorVertAttributes[ NumColorVertAttributes ] = {
    "position",
    "normal",
    "color0"
};

ColorVert::ColorVert()
: position()
, normal()
, color0( 1, 1, 1 ) {
    // empty
}

ui32 ColorVert::getNumAttributes() {
    return NumColorVertAttributes;
}

const String *ColorVert::getAttributes() {
    return ColorVertAttributes;
}

// List of attributes for render vertices
static const ui32 NumRenderVertAttributes = 4;

static const String RenderVertAttributes[ NumRenderVertAttributes ] = {
    "position",
    "normal",
    "color0",
    "texcoord0"
};

RenderVert::RenderVert()
: position()
, normal()
, color0( 1,1,1 )
, tex0() {
    // empty
}

RenderVert::~RenderVert() {
    // empty
}
bool RenderVert::operator == ( const RenderVert &rhs ) const {
    return ( position == rhs.position && normal == rhs.normal && color0 == rhs.color0 && tex0 == rhs.tex0 );
}

bool RenderVert::operator != ( const RenderVert &rhs ) const {
    return !( *this == rhs );
}

ui32 RenderVert::getNumAttributes() {
    return NumRenderVertAttributes;
}

const String *RenderVert::getAttributes() {
    return RenderVertAttributes;
}

const String &getVertCompName( VertexAttribute attrib ) {
    if( attrib > VertexAttribute::Instance3 ) {
        return ErrorCmpName;
    }
    return VertCompName[ static_cast<int>(attrib) ];
}

VertComponent::VertComponent()
: m_attrib( VertexAttribute::InvalidVertexAttr )
, m_format( VertexFormat::InvalidVertexFormat ) {
    // empty
}

VertComponent::VertComponent( VertexAttribute attrib, VertexFormat format )
: m_attrib( attrib )
, m_format( format ) {
    // empty
}

VertComponent::~VertComponent() {
    // empty
}

VertexLayout::VertexLayout()
: m_attributes( nullptr )
, m_components()
, m_offsets()
, m_currentOffset( 0 )
, m_sizeInBytes( 0 ) {
    // empty
}

VertexLayout::~VertexLayout() {
    delete [] m_attributes;
    m_attributes = nullptr;
}

void VertexLayout::clear() {
    if( !m_components.isEmpty() ) {
        for( ui32 i = 0; i < m_components.size(); ++i ) {
            delete m_components[ i ];
        }
        m_components.clear();
    }

    m_offsets.clear();
    m_currentOffset = 0;
}

ui32 VertexLayout::sizeInBytes() {
    if( 0 == m_sizeInBytes ) {
        for( ui32 i = 0; i < m_components.size(); ++i ) {
            const ui32 compSizeInBytes( getVertexFormatSize( m_components[ i ]->m_format ) );
            m_sizeInBytes += compSizeInBytes;
        }
    }

    return m_sizeInBytes;
}

ui32 VertexLayout::numComponents() const {
    return m_components.size();
}

VertexLayout &VertexLayout::add( VertComponent *comp ) {
    if( nullptr == comp ) {
        return *this;
    }
    m_components.add( comp );
    const ui32 offset( getVertexFormatSize( comp->m_format ) );
    m_offsets.add( m_currentOffset );
    m_currentOffset += offset;

    return *this;
}

VertComponent &VertexLayout::getAt( ui32 idx ) const {
    if( idx >= m_components.size() ) {
        return ErrorComp;
    }

    return *m_components[ idx ];
}

const String *VertexLayout::getAttributes() {
    if( m_components.isEmpty() ) {
        return nullptr;
    }
    
    if( nullptr == m_attributes ) {
        const ui32 numAttributes( m_components.size() );
        m_attributes = new String[ numAttributes ];
        for( ui32 i = 0; i < m_components.size(); ++i )  {
            m_attributes[ i ] = VertCompName[ static_cast<int>( m_components[ i ]->m_attrib  ) ];
        }
    }
    
    return m_attributes;
}

BufferData::BufferData()
: m_type( BufferType::EmptyBuffer )
, m_data( nullptr )
, m_size( 0 )
, m_cap( 0 )
, m_access( BufferAccessType::ReadOnly ) {
    // empty
}

BufferData::~BufferData() {
    delete[] m_data;
    m_data = nullptr;
    m_size = 0;
    m_cap = 0;
}

BufferData* BufferData::alloc( BufferType type, ui32 sizeInBytes, BufferAccessType access ) {
    BufferData *buffer( new BufferData );
    buffer->m_size   = sizeInBytes;
    buffer->m_cap    = sizeInBytes;
    buffer->m_access = access;
    buffer->m_type   = type;
    buffer->m_data   = new uc8[ sizeInBytes ];

    return buffer;
}

void BufferData::free( BufferData *data ) {
    if ( nullptr == data ) {
		return;
	}

	delete data;
	data = nullptr;
}

void BufferData::copyFrom( void *data, ui32 size ) {
    if ( nullptr == data ) {
        return;
    }
    if ( size > m_cap ) {
        osre_error( Tag, "Out of buffer error." );
        return;
    }

    m_size = size;
    ::memcpy( m_data, data, size );
}

void BufferData::attach( void *data, ui32 size ) {
    const ui32 newSize( m_size + size );
    if ( newSize < m_cap ) {
        void *ptr = ( (uc8*) m_data ) + m_size;
        ::memcpy( ptr, data, size );
        m_size += size;
        return;
    }
    
    uc8 *newData = new uc8[ newSize ];
    ::memcpy( newData, m_data, m_size );
    ::memcpy( &newData[ m_size ], data, size );
    delete[] m_data;
    m_data = newData;
    m_size += size;
}

BufferType BufferData::getBufferType() const {
    return m_type;
}

BufferAccessType BufferData::getBufferAccessType() const {
    return m_access;
}

PrimitiveGroup::PrimitiveGroup()
: m_primitive( PrimitiveType::LineList )
, m_startIndex( 0 )
, m_numIndices( 0 )
, m_indexType( IndexType::UnsignedShort ) {
    // empty
}

PrimitiveGroup::~PrimitiveGroup() {
    // empty
}

void PrimitiveGroup::init( IndexType indexType, ui32 numPrimitives, PrimitiveType primType, 
        ui32 startIdx ) {
    m_indexType = indexType;
    m_numIndices = numPrimitives;
    m_primitive = primType;
    m_startIndex = startIdx;
}

Texture::Texture()
: m_textureName( "" )
, m_loc()
, m_targetType( TextureTargetType::Texture2D )
, m_size( 0 )
, m_data( nullptr )
, m_width( 0 )
, m_height( 0 )
, m_channels( 0 )
, m_texHandle() {
    // empty
}

Texture::~Texture() {
    delete[] m_data;
    m_data = nullptr;
}

Shader::Shader() {
    // empty
}

Shader::~Shader() {
    // empty
}

Material::Material( const String &name )
: m_name( name )
, m_type( MaterialType::ShaderMaterial )
, m_numTextures( 0 )
, m_textures( nullptr )
, m_shader( nullptr )
, m_numParameters( 0 )
, m_parameters( nullptr ) {
    // empty
}

Material::Material( const String &name, MaterialType type )
: m_name( name )
, m_type( type )
, m_numTextures( 0 )
, m_textures( nullptr )
, m_shader( nullptr )
, m_numParameters( 0 )
, m_parameters( nullptr ) {
    // empty
}

Material::~Material() {
    delete m_shader;
    m_shader = nullptr;

    delete[] m_textures;
    m_textures = nullptr;
}

void Material::createShader(CPPCore::TStaticArray<String, MaxShaderTypes> &shaders) {
    m_shader = new Shader;
    for (ui32 i = 0; i < MaxShaderTypes; ++i) {
        if (!shaders[i].empty()) {
            m_shader->m_src[i] = shaders[i];
        }
    }
}

GeoInstanceData::GeoInstanceData()
: m_data( nullptr ) {
    // empty
}

GeoInstanceData::~GeoInstanceData() {
    delete m_data;
    m_data = nullptr;
}

TransformState::TransformState() 
: m_translate()
, m_scale(1.0f)
, m_rotation() {
    // empty
}

TransformState::~TransformState() {
    // empty
}

void TransformState::setTranslation( f32 x, f32 y, f32 z ) {
    m_translate.x = x;
    m_translate.y = y;
    m_translate.z = z;
}

void TransformState::setScale( f32 sx, f32 sy, f32 sz ) {
    m_scale.x = sx;
    m_scale.y = sy;
    m_scale.z = sz;
}

bool TransformState::operator == ( const TransformState &rhs ) const {
    return ( m_translate == rhs.m_translate && m_scale == rhs.m_scale && m_rotation == rhs.m_rotation );
}

bool TransformState::operator != ( const TransformState &rhs ) const {
    return !( *this == rhs );
}

void TransformState::toMatrix(mat4 &m) const {
    mat4 mvp = mat4( 1.0f );
    mvp *= glm::translate( m, m_translate );
    mvp *= mat4(1.0f);
    mvp *= glm::scale( m, m_scale);
    m *= mvp;
}

TransformMatrixBlock::TransformMatrixBlock()
: m_projection(1.0f)
, m_model(1.0f)
, m_view(1.0f)
, m_mvp(1.0f) {
    init();
}

TransformMatrixBlock::~TransformMatrixBlock() {
    // empty
}

void TransformMatrixBlock::init() {
    m_projection = glm::mat4( 1.0f );
    m_model = glm::mat4( 1.0f );
    m_view = glm::mat4( 1.0f );
    m_mvp = glm::mat4(1.0f);
}

void TransformMatrixBlock::update() {
    m_mvp = m_projection * m_view * m_model;
}

const float *TransformMatrixBlock::getMVP() {
    return glm::value_ptr( m_mvp );
}

Viewport::Viewport() 
: m_x( -1 )
, m_y( -1 )
, m_w( -1 )
, m_h( -1 ) {
	// empty
}

Viewport::Viewport( i32 x, i32 y, i32 w, i32 h )
: m_x( x )
, m_y( y )
, m_w( w )
, m_h( h ) {
	// empty
}

Viewport::~Viewport() {
	// empty
}

bool Viewport::operator == ( const Viewport &rhs ) const {
	return ( m_x == rhs.m_x && m_y == rhs.m_y && m_w == rhs.m_w && m_h == rhs.m_h );
}

bool Viewport::operator != ( const Viewport &rhs ) const {
    return !( *this == rhs );
}

RenderBatch::RenderBatch() 
: m_model( 1.0f )
, m_numGeo( 0 ) 
, m_geoArray( nullptr ) {
    // empty
}

RenderBatch::~RenderBatch() {
    // empty
}

Light::Light()
: m_position(0.0f,0.0f,0.0f,1.0f)
, m_specular( 1.0f,1.0f,1.0f)
, m_diffuse(1.0f,1.0f,1.0f)
, m_ambient(1.0f,1.0f,1.0f)
, m_direction(0.0f, 0.0f, 1.0f, 1.0f)
, m_specularExp( 1.0f)
, m_type( LightType::None ) {
    // empty
}

Light::~Light() {
    // empty
}

} // Namespace RenderBackend
} // Namespace OSRE
