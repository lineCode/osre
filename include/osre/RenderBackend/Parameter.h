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

#include <cppcore/Container/THashMap.h>
#include <cppcore/Container/TArray.h>

#include <glm/glm.hpp>

namespace OSRE {
namespace RenderBackend {

class RenderBackendService;

struct Geometry;
struct GeoInstanceData;
struct Light;

enum class ParameterType {
    PT_None,
    PT_Int,
    PT_IntArray,
    PT_Float,
    PT_FloatArray,
    PT_Float2,
    PT_Float2Array,
    PT_Float3,
    PT_Float3Array,
    PT_Mat4, 
    PT_Mat4Array
};

struct OSRE_EXPORT UniformDataBlob {
    void *m_data;
    ui32  m_size;

    UniformDataBlob();
    ~UniformDataBlob();
    void *getData() const;
    void clear();

    static UniformDataBlob *create( ParameterType type, ui32 arraySize );
};

struct OSRE_EXPORT UniformVar {
    String         m_name;
    ParameterType  m_type;
    ui32           m_numItems;
    UniformDataBlob  m_data;
    UniformVar     *m_next;

    static ui32 getParamDataSize( ParameterType type, ui32 arraySize );
    static UniformVar *create( const String &name, ParameterType type, ui32 arraySize=1 );
    static void destroy( UniformVar *param );

private:
    UniformVar();
    ~UniformVar();
};

class UniformBuffer {
public:
    static UniformBuffer *create( ui32 size ) {
        return new UniformBuffer( size );
    }

    static void destroy( UniformBuffer *buffer ) {
        delete buffer;
    }

    ui32 read( ui32 size, uc8 *data ) {
        if ( nullptr == data || 0 == size ) {
            return 0;
        }
        if ( m_offset + size > m_size ) {
            return 0;
        }
        ::memcpy(data, &m_data[m_offset], size );
        m_offset += size;
        
        return size;
    }

    void write(ui32 size, uc8 *data) {
        if (size + m_offset > m_size) {
            return;
        }

        ::memcpy(&m_data[m_offset], data, size);
        m_offset += size;
    }

    void reset() {
        m_offset = 0;
    }

    bool setPos( ui32 pos ) {
        if ( pos > m_size ) {
            return false;
        }
        m_offset = pos;
        
        return true;
    }

    UniformBuffer( ui32 size) 
    : m_size( size )
    , m_offset( 0 )
    , m_data( nullptr ) {
        m_data = new uc8[size];
    }
    
    ~UniformBuffer() {
        m_size = 0;
        delete[] m_data;
        m_data = nullptr;
    }

    UniformBuffer(const UniformBuffer &) = delete;
    UniformBuffer& operator = (const UniformBuffer &) = delete;

private:
    ui32 m_size;
    ui32 m_offset;
    uc8 *m_data;
};

struct GeometryPackage {
    ui32       m_numInstances;
    ui32       m_numNewGeo;
    Geometry **m_newGeo;

    GeometryPackage()
    : m_numInstances( 0 )
    , m_numNewGeo( 0 )
    , m_newGeo( nullptr ) {
        // empty
    }
};

struct Frame {
    ui32              m_numVars;
    UniformVar      **m_vars;
    ui32              m_numGeoPackages;
    GeometryPackage **m_geoPackages;
    ui32              m_numGeoUpdates;
    Geometry        **m_geoUpdates;
    ui32              m_numLights;
    Light           **m_lights;
    GeoInstanceData   *m_geoInstanceData;
    glm::mat4         m_model;
    glm::mat4         m_view;
    glm::mat4         m_proj;

    Frame() 
    : m_numVars( 0 )
    , m_vars( nullptr )
    , m_numGeoPackages( 0 )
    , m_geoPackages( nullptr )
    , m_numGeoUpdates( 0 )
    , m_geoUpdates( nullptr )
    , m_numLights( 0 )
    , m_lights( nullptr )
    , m_geoInstanceData( nullptr )
    , m_model( 1.0f )
    , m_view( 1.0f )
    , m_proj( 1.0f ) {
        // empty
    }
    
    ~Frame() {
        // empty
    }

    Frame( const Frame & ) = delete;
    Frame( Frame &&) = delete;
    Frame& operator = ( const Frame & ) = delete;
};

} // Namespace RenderBackend
} // Namespace OSRE
