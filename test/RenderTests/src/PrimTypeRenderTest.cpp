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
#include "AbstractRenderTest.h"
#include "RenderTestUtils.h"

#include <osre/Common/Logger.h>
#include <osre/RenderBackend/RenderBackendService.h>
#include <osre/RenderBackend/RenderCommon.h>
#include <osre/RenderBackend/Geometry.h>
#include <osre/Scene/GeometryBuilder.h>
#include <osre/Scene/MaterialBuilder.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace OSRE {
namespace RenderTest {

using namespace ::OSRE::RenderBackend;

class PrimTypeRenderTest : public AbstractRenderTest {
    static const ui32 NumPoints = 3;
    static const ui32 PtNumIndices = 3;

    TransformMatrixBlock m_transformMatrix;

public:
    PrimTypeRenderTest()
        : AbstractRenderTest( "rendertest/PrimTypeRenderTest" ) {
        // empty
    }

    virtual ~PrimTypeRenderTest() {
        // empty
    }

    bool onCreate( RenderBackendService *rb ) override {
        rb->sendEvent( &OnAttachViewEvent, nullptr );

        // colors
        glm::vec3 col[ NumPoints ];
        col[ 0 ] = glm::vec3( 1, 0, 0 );
        col[ 1 ] = glm::vec3( 0, 1, 0 );
        col[ 2 ] = glm::vec3( 0, 0, 1 );

        // point coordinates
        glm::vec3 points[ NumPoints ];
        points[ 0 ] = glm::vec3( -0.5, -0.5, 0 );
        points[ 1 ] = glm::vec3(  0,    0.5, 0 );
        points[ 2 ] = glm::vec3(  0.5, -0.5, 0 );
        
        GLushort pt_indices[ PtNumIndices ];
        pt_indices[ 0 ] = 0;
        pt_indices[ 1 ] = 1;
        pt_indices[ 2 ] = 2;

        // line segment coordinates
        glm::vec3 pos[ NumPoints ];
        pos[ 0 ] = glm::vec3( -1, -1, 0 );
        pos[ 1 ] = glm::vec3(  0,  1, 0 );
        pos[ 2 ] = glm::vec3(  1, -1, 0 );

        static const ui32 NumIndices = 6;
        GLushort indices[ NumIndices ];
        indices[ 0 ] = 0;
        indices[ 1 ] = 1;

        indices[ 2 ] = 1;
        indices[ 3 ] = 2;

        indices[ 4 ] = 2;
        indices[ 5 ] = 0;

        static ui32 NumGeo( 2 );

        Geometry *ptGeo = Scene::GeometryBuilder::allocPoints( VertexType::ColorVertex, BufferAccessType::ReadOnly, NumPoints, points, col );
        rb->attachGeo( ptGeo, 0 );
        Geometry *lineGeo = Scene::GeometryBuilder::allocEmptyGeometry( VertexType::ColorVertex, 1 );
        lineGeo->m_vb = Scene::GeometryBuilder::allocVertices( VertexType::ColorVertex, 3, pos, col, nullptr, BufferAccessType::ReadOnly );
        lineGeo->m_indextype = IndexType::UnsignedShort;
        ui32 size = sizeof( GLushort ) * NumIndices;
        lineGeo->m_ib = BufferData::alloc( BufferType::IndexBuffer, size, BufferAccessType::ReadOnly );
        lineGeo->m_ib->copyFrom( indices, size );

        // setup primitives
        lineGeo->m_numPrimGroups = 1;
        lineGeo->m_pPrimGroups = new PrimitiveGroup[ lineGeo->m_numPrimGroups ];
        lineGeo->m_pPrimGroups[ 0 ].init( IndexType::UnsignedShort, 2*3, PrimitiveType::LineList, 0 );
        
        // setup material
        Material *mat = Scene::MaterialBuilder::createBuildinMaterial( VertexType::ColorVertex );
        lineGeo->m_material = mat;
        rb->attachGeo( lineGeo, 0 );

        m_transformMatrix.m_model = glm::rotate( m_transformMatrix.m_model, 0.0f, glm::vec3( 1, 1, 0 ) );
        m_transformMatrix.m_model = glm::scale( m_transformMatrix.m_model, glm::vec3( .5, .5, .5 ) );
        rb->setMatrix(MatrixType::Model, m_transformMatrix.m_model);

        return true;
    }
};

ATTACH_RENDERTEST( PrimTypeRenderTest )

} // Namespace RenderTest
} // Namespace OSRE
