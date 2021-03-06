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
#include "OGLRenderEventHandler.h"
#include "OGLRenderBackend.h"
#include "OGLShader.h"
#include "OGLCommon.h"
#include "RenderCmdBuffer.h"

#include <osre/Common/Logger.h>
#include <osre/Platform/PlatformInterface.h>
#include <osre/Platform/AbstractWindow.h>
#include <osre/Platform/AbstractRenderContext.h>
#include <osre/Profiling/PerformanceCounterRegistry.h>
#include <osre/RenderBackend/RenderCommon.h>
#include <osre/RenderBackend/Geometry.h>
#include <osre/RenderBackend/HWBufferManager.h>
#include <osre/Debugging/osre_debugging.h>
#include <osre/IO/Uri.h>
#include <osre/Assets/AssetRegistry.h>

#include <cppcore/Container/TArray.h>

namespace OSRE {
namespace RenderBackend {

using namespace ::OSRE::Common;
using namespace ::OSRE::Platform;
using namespace ::CPPCore;

static const String Tag = "OGLRendeEventHandler";

static bool setupTextures( Material *mat, OGLRenderBackend *rb, TArray<OGLTexture*> &textures ) {
    OSRE_ASSERT( nullptr != mat );
	OSRE_ASSERT( nullptr != rb );

    if ( nullptr == mat ) {
        osre_debug( Tag, "Material is nullptr." );
        return false;
    }

    if ( nullptr == rb ) {
        osre_debug( Tag, "Renderbackend is nullptr." );
        return false;
    }

    const ui32 numTextures( mat->m_numTextures );
    if( 0 == numTextures ) {
        return true;
    }

    for( ui32 i = 0; i < numTextures; ++i ) {
        Texture *tex( mat->m_textures[ i ] );
        if( !tex->m_textureName.empty() ) {
            String root = Assets::AssetRegistry::getPath( "media" );
            String path = Assets::AssetRegistry::resolvePathFromUri( tex->m_loc );
            
            IO::Uri loc( tex->m_loc );
            loc.setPath( path );

            OGLTexture *oglTexture = rb->createTextureFromFile( tex->m_textureName, loc );
            if( nullptr != oglTexture ) {
                textures.add( oglTexture );
            }
        }
    }

    return true;
}

static void setupLights( ui32 numLights, Light **lights, OGLRenderBackend *rb, OGLRenderEventHandler *eh) {
    OSRE_ASSERT(nullptr != eh);
    OSRE_ASSERT(nullptr != rb);
    OSRE_ASSERT(nullptr != lights);

    for ( ui32 i = 0; i < numLights; ++i ) {
        Light *currentLight( lights[ i ] );
    }
}

static void setConstantBuffers(const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &proj, 
        OGLRenderBackend *rb, OGLRenderEventHandler *eh) {
    OSRE_ASSERT(nullptr != eh);
    OSRE_ASSERT(nullptr != rb);

    eh->getRenderCmdBuffer()->setMatrixes(model, view, proj);
}

static SetMaterialStageCmdData *setupMaterial( Material *material, OGLRenderBackend *rb, OGLRenderEventHandler *eh ) {
	OSRE_ASSERT( nullptr != eh );
	OSRE_ASSERT( nullptr != material );
	OSRE_ASSERT( nullptr != rb );

    SetMaterialStageCmdData *matData = new SetMaterialStageCmdData;
    switch( material->m_type ) {
        case MaterialType::ShaderMaterial: {
                TArray<OGLTexture*> textures;
                setupTextures( material, rb, textures );
                OGLRenderCmd *renderMatCmd = OGLRenderCmdAllocator::alloc( OGLRenderCmdType::SetMaterialCmd, nullptr );
                if( !textures.isEmpty() ) {
                    matData->m_textures = textures;
                }

                OGLShader *shader = rb->createShader( "mat", material->m_shader );
                if ( nullptr != shader ) {
                    matData->m_shader = shader;
                    for( ui32 i = 0; i < material->m_shader->m_attributes.size(); i++ ) {
                        const String &attribute = material->m_shader->m_attributes[ i ];
                        //if ( shader->hasAttribute( attribute ) ) {
                            shader->addAttribute( attribute );
                        //}
                    }

                    for( ui32 i = 0; i < material->m_shader->m_parameters.size(); i++ ) {
                        shader->addUniform( material->m_shader->m_parameters[ i ] );
                    }

                    // for setting up all buffer objects
                    eh->setActiveShader( shader );
                }
                renderMatCmd->m_data = matData;
                eh->enqueueRenderCmd( renderMatCmd );
            }
            break;

        default:
            break;
    }

    return matData;
}

static void setupParameter( UniformVar *param, OGLRenderBackend *rb, OGLRenderEventHandler *ev ) {
	OSRE_ASSERT( nullptr != param );
	OSRE_ASSERT( nullptr != rb );
	OSRE_ASSERT( nullptr != ev );

    if( !param ) {
        return;
    }

    ::CPPCore::TArray<OGLParameter*> paramArray;
    OGLParameter *oglParam = rb->getParameter( param->m_name );
    if ( nullptr == oglParam ) {
        oglParam = rb->createParameter( param->m_name, param->m_type, &param->m_data, param->m_numItems );
    } else {
        ::memcpy( oglParam->m_data->getData(), param->m_data.getData(), param->m_data.m_size );
    }

    paramArray.add( oglParam );
    ev->setParameter( paramArray );
}


static OGLVertexArray *setupBufferForGeoBundle( GeometryPackage *geoPackage, OGLRenderBackend *rb, OGLShader *oglShader ) {
    OSRE_ASSERT( nullptr != geoPackage );
    OSRE_ASSERT( nullptr != rb );
    OSRE_ASSERT( nullptr != oglShader );

    rb->useShader( oglShader );
    OGLVertexArray *vertexArray = rb->createVertexArray();
    rb->bindVertexArray( vertexArray );

    // Get the batch size
    ui32 lenVB( 0 ), lenIB( 0 );
    for ( ui32 i = 0; i < geoPackage->m_numNewGeo; ++i ) {
        lenVB += geoPackage->m_newGeo[ i ]->m_vb->m_size;
        lenIB += geoPackage->m_newGeo[ i ]->m_ib->m_size;
    }
    if ( 0 == lenVB ) {
        return vertexArray;
    }

    // Build one batch
    ui32 offsetVD( 0 ), offsetID( 0 );
    TArray<uc8> vertexData( lenVB ), indexData( lenIB );
    BufferType vbType( geoPackage->m_newGeo[ 0 ]->m_vb->m_type );
    BufferAccessType vbAccessType(geoPackage->m_newGeo[ 0 ]->m_vb->m_access);
    BufferType ibType( geoPackage->m_newGeo[ 0 ]->m_ib->m_type );
    BufferAccessType ibAccessType(geoPackage->m_newGeo[ 0 ]->m_ib->m_access);
    VertexType vertexType(geoPackage->m_newGeo[ 0 ]->m_vertextype);
    for ( ui32 i = 0; i < geoPackage->m_numNewGeo; ++i ) {
        Geometry *geo( geoPackage->m_newGeo[ i ] );
        OSRE_ASSERT( vertexType == geo->m_vertextype );
        OSRE_ASSERT( geo->m_vb->m_type == vbType );
        OSRE_ASSERT( geo->m_vb->m_access == vbAccessType);
        OSRE_ASSERT( geo->m_ib->m_type == ibType );
        OSRE_ASSERT( geo->m_ib->m_access == ibAccessType);

        ::memcpy(&vertexData[offsetVD], geo->m_vb->m_data, geo->m_vb->m_size);
        offsetVD += geo->m_vb->m_size;
        ::memcpy( &indexData[ offsetID ], geo->m_ib->m_data, geo->m_ib->m_size );
        offsetID += geo->m_ib->m_size;
    }

    // create vertex buffer and  and pass triangle vertex to buffer object
    OGLBuffer *vb = rb->createBuffer(vbType);
    //vb->m_geoId = geo->m_id;
    rb->bindBuffer(vb);
    rb->copyDataToBuffer(vb, &vertexData[0], vertexData.size(), vbAccessType );

    // enable vertex attribute arrays
    TArray<OGLVertexAttribute*> attributes;
    rb->createVertexCompArray(vertexType, oglShader, attributes);
    const ui32 stride = Geometry::getVertexSize(vertexType);
    rb->bindVertexLayout(vertexArray, oglShader, stride, attributes);
    rb->releaseVertexCompArray(attributes);

    // create index buffer and pass indices to element array buffer
    OGLBuffer *ib = rb->createBuffer(ibType);
    //ib->m_geoId = geo->m_id;
    rb->bindBuffer(ib);
    rb->copyDataToBuffer(ib, &indexData[0], indexData.size(), ibAccessType );

    rb->unbindVertexArray();

    return vertexArray;
}

static OGLVertexArray *setupBuffers( Geometry *geo, OGLRenderBackend *rb, OGLShader *oglShader ) {
	OSRE_ASSERT( nullptr != geo );
	OSRE_ASSERT( nullptr != rb );
	OSRE_ASSERT( nullptr != oglShader );

    rb->useShader( oglShader );

    OGLVertexArray *vertexArray = rb->createVertexArray();
    rb->bindVertexArray( vertexArray );
    BufferData *vertices = geo->m_vb;
	if ( nullptr == vertices ) {
		osre_debug( Tag, "No vertex buffer data for setting up data." );
		return nullptr;
    }

    BufferData *indices = geo->m_ib;
    if ( nullptr == indices ) {
        osre_debug( Tag, "No index buffer data for setting up data." );
        return nullptr;
    }

    // create vertex buffer and  and pass triangle vertex to buffer object
    OGLBuffer *vb = rb->createBuffer( vertices->m_type );
    vb->m_geoId = geo->m_id;
    rb->bindBuffer( vb );
    rb->copyDataToBuffer( vb, vertices->m_data, vertices->m_size, vertices->m_access );

    // enable vertex attribute arrays
    TArray<OGLVertexAttribute*> attributes;
    rb->createVertexCompArray( geo->m_vertextype, oglShader, attributes );
    const ui32 stride = Geometry::getVertexSize( geo->m_vertextype );
    rb->bindVertexLayout( vertexArray, oglShader, stride, attributes );
    rb->releaseVertexCompArray( attributes );

    // create index buffer and pass indices to element array buffer
    OGLBuffer *ib = rb->createBuffer( indices->m_type );
    ib->m_geoId = geo->m_id;
    rb->bindBuffer( ib );
    rb->copyDataToBuffer( ib, indices->m_data, indices->m_size, indices->m_access );

    rb->unbindVertexArray();

    return vertexArray;
}

static void setupPrimDrawCmd( bool useLocalMatrix, const glm::mat4 &model, const TArray<ui32> &primGroups, OGLRenderBackend *rb, 
        OGLRenderEventHandler *eh, OGLVertexArray *va ) {
	OSRE_ASSERT( nullptr != rb );
	OSRE_ASSERT( nullptr != eh );

    if( primGroups.isEmpty() ) {
        return;
    }

	OGLRenderCmd *renderCmd = OGLRenderCmdAllocator::alloc( OGLRenderCmdType::DrawPrimitivesCmd, nullptr );
    DrawPrimitivesCmdData *data = new DrawPrimitivesCmdData;
    if ( useLocalMatrix ) {
        data->m_model = model;
        data->m_localMatrix = useLocalMatrix;
    }
    data->m_vertexArray = va;
    data->m_primitives.reserve( primGroups.size() );
    for( ui32 i = 0; i < primGroups.size(); ++i ) {
        data->m_primitives.add( primGroups[ i ] );
    }
    renderCmd->m_data = static_cast<void*>( data );
    
    eh->enqueueRenderCmd( renderCmd );
}

static void setupInstancedDrawCmd( const TArray<ui32> &ids, Frame *currentFrame, 
        OGLRenderBackend *rb, OGLRenderEventHandler *eh, OGLVertexArray *va ) {
	OSRE_ASSERT( nullptr != currentFrame );
	OSRE_ASSERT( nullptr != rb );
	OSRE_ASSERT( nullptr != eh );

    if( ids.isEmpty() ) {
        return;
    }

    GeoInstanceData *instData( currentFrame->m_geoInstanceData );
	OGLRenderCmd *renderCmd = OGLRenderCmdAllocator::alloc( OGLRenderCmdType::DrawPrimitivesInstancesCmd, nullptr );
    if( nullptr != instData ) {
        if( nullptr != instData->m_data ) {
            OGLBuffer *instanceDataBuffer = rb->createBuffer( BufferType::InstanceBuffer );
            rb->bindBuffer( instanceDataBuffer );
            rb->copyDataToBuffer( instanceDataBuffer, instData->m_data->m_data, instData->m_data->m_size, instData->m_data->m_access );
        }
    }
    
    for ( ui32 i=0; i<currentFrame->m_numGeoPackages; i++ ) {
        GeometryPackage *currentGeoPackage( currentFrame->m_geoPackages[ i ] );
        if ( nullptr == currentGeoPackage ) {
            continue;
        }
        DrawInstancePrimitivesCmdData *data = new DrawInstancePrimitivesCmdData;
        data->m_vertexArray = va;
        data->m_numInstances = currentGeoPackage->m_numInstances;
        data->m_primitives.reserve( ids.size() );
        for( ui32 j = 0; j < ids.size(); ++j ) {
            data->m_primitives.add( ids[ j ] );
        }
        renderCmd->m_data = static_cast< void* >( data );
        eh->enqueueRenderCmd( renderCmd );
    }
}

OGLRenderEventHandler::OGLRenderEventHandler( )
: AbstractEventHandler()
, m_isRunning ( true )
, m_oglBackend( nullptr )
, m_renderCmdBuffer( nullptr )
, m_renderCtx( nullptr )
, m_vertexArray( nullptr )
, m_hwBufferManager( nullptr ) {
    // empty
}
        
OGLRenderEventHandler::~OGLRenderEventHandler( ) {
    delete m_hwBufferManager;
    m_hwBufferManager = nullptr;
}

bool OGLRenderEventHandler::onEvent( const Event &ev, const EventData *data ) {
    bool result( false );
    if ( !m_isRunning ) {
        return true;
    }

    if ( OnAttachEventHandlerEvent == ev ) {
        result = onAttached( data );
    } else if ( OnDetatachEventHandlerEvent == ev ) {
        result = onDetached( data );
    } else if ( OnCreateRendererEvent == ev ) {
        result = onCreateRenderer( data );
    } else if ( OnDestroyRendererEvent == ev ) {
        result = onDestroyRenderer( data );
    } else if( OnAttachViewEvent == ev ) {
        result = onAttachView( data );
    } else if ( OnDetachViewEvent == ev ) {
        result = onDetachView( data );
    } else if ( OnRenderFrameEvent == ev ) {
        result = onRenderFrame( data );
    } else if ( OnCommitFrameEvent == ev ) {
        result = onCommitNexFrame( data );
    } else if ( OnClearSceneEvent == ev ) {
        result = onClearGeo( data );
    } else if ( OnShutdownRequest == ev ) {
        result = onShutdownRequest( data );
    } else if ( OnResizeEvent == ev ) {
        result = onResizeRenderTarget( data );
    }
 
    return result;
}

void OGLRenderEventHandler::setActiveShader( OGLShader *oglShader ) {
    m_renderCmdBuffer->setActiveShader( oglShader );
}

void OGLRenderEventHandler::enqueueRenderCmd( OGLRenderCmd *oglRenderCmd ) {
    m_renderCmdBuffer->enqueueRenderCmd( "pass0", oglRenderCmd );
}

void OGLRenderEventHandler::setParameter( const ::CPPCore::TArray<OGLParameter*> &paramArray ) {
    m_renderCmdBuffer->setParameter( paramArray );
}

bool OGLRenderEventHandler::onAttached( const EventData* ) {
    if( nullptr != m_oglBackend ) {
        return false;
    }

    m_oglBackend = new OGLRenderBackend;
    m_oglBackend->setTimer( PlatformInterface::getInstance()->getTimer() );
    m_hwBufferManager = new HWBufferManager;

    return true;
}

bool OGLRenderEventHandler::onDetached( const EventData* ) {
    if( m_renderCmdBuffer ) {
        osre_error( Tag, "Renderer not destroyed." );
        delete m_renderCmdBuffer;
        m_renderCmdBuffer = nullptr;
    }

    delete m_oglBackend;
    m_oglBackend = nullptr;

    return true;
}

bool OGLRenderEventHandler::onCreateRenderer( const EventData *eventData ) {
	OSRE_ASSERT( nullptr != m_oglBackend );
	
	CreateRendererEventData *createRendererEvData = ( CreateRendererEventData* ) eventData;
    AbstractWindow *activeSurface = createRendererEvData->m_activeSurface;
    if( nullptr == activeSurface ) {
        osre_debug( Tag, "No active surface, pointer is nullptr." );
        return false;
    }

    if( !activeSurface->create() ) {
        osre_debug( Tag, "Cannot create active surface." );
        return false;
    }

    bool result( false );
    m_renderCtx = PlatformInterface::getInstance()->getRenderContext();

    if( nullptr !=  m_renderCtx ) {
        result = m_renderCtx->create( activeSurface );
        if( !result ) {
            osre_debug( Tag, "Cannot create render context." );
            return false;
        }
    }

    m_oglBackend->setRenderContext( m_renderCtx );
    if ( !m_renderCtx->isActive() ) {
        osre_debug( Tag, "Error while activafing render context." );
        return false;
    }

    Rect2ui rect = activeSurface->getWindowsRect();
    m_oglBackend->setViewport( rect.m_x1, rect.m_y1, rect.m_width, rect.m_height );

    const String defaultFont( PlatformInterface::getInstance()->getDefaultFontName() );
    IO::Uri fontUri( "file://assets/Textures/Fonts/" + defaultFont );
    String root = Assets::AssetRegistry::getPath( "media" );
    String path = Assets::AssetRegistry::resolvePathFromUri( fontUri );
    fontUri.setPath( path );
    m_oglBackend->createFont( fontUri );
    m_renderCmdBuffer = new RenderCmdBuffer( m_oglBackend, m_renderCtx, createRendererEvData->m_pipeline );

    bool ok( Profiling::PerformanceCounterRegistry::create() );
    if ( !ok ) {
        osre_error( Tag, "Error while destroying performance counters." );
        return false;
    }

    Profiling::PerformanceCounterRegistry::registerCounter( "fps" );

    return true;
}

bool OGLRenderEventHandler::onDestroyRenderer( const Common::EventData * ) {
	OSRE_ASSERT( nullptr != m_oglBackend );
	
	if ( nullptr != m_renderCtx ) {
        return false;
    }

    bool ok( Profiling::PerformanceCounterRegistry::destroy() );
    if ( !ok ) {
        osre_error( Tag, "Error while destroying performance counters." );
    }

    m_renderCtx->destroy();
    delete m_renderCtx;
    m_renderCtx = nullptr;

    delete m_renderCmdBuffer;
    m_renderCmdBuffer = nullptr;

    return true;
}

bool OGLRenderEventHandler::onAttachView( const EventData * ) {
	OSRE_ASSERT( nullptr != m_oglBackend );
	
	return true;
}

bool OGLRenderEventHandler::onDetachView( const EventData * ) {
    OSRE_ASSERT( nullptr != m_oglBackend );

    return true;
}

bool OGLRenderEventHandler::onClearGeo( const EventData * ) {
	OSRE_ASSERT( nullptr != m_oglBackend );
	
	m_oglBackend->releaseAllBuffers();
    m_oglBackend->releaseAllShaders();
    m_oglBackend->releaseAllTextures();
    m_oglBackend->releaseAllParameters();
    m_renderCmdBuffer->clear();

    return true;
}

bool OGLRenderEventHandler::onRenderFrame( const EventData *eventData ) {
	OSRE_ASSERT( nullptr != m_oglBackend );

    if ( nullptr == m_renderCtx ) {
		osre_debug( Tag, "Render context is nullptr." );
        return false;
    }

    OSRE_ASSERT(nullptr != m_renderCmdBuffer);
    m_renderCmdBuffer->onPreRenderFrame();
    m_renderCmdBuffer->onRenderFrame( eventData );
    m_renderCmdBuffer->onPostRenderFrame();

    return true;
}

bool OGLRenderEventHandler::onCommitNexFrame( const Common::EventData *eventData ) {
    OSRE_ASSERT( nullptr != m_oglBackend );

    CommitFrameEventData *frameToCommitData = ( CommitFrameEventData* ) eventData;
    if ( nullptr == frameToCommitData ) {
        return false;
    }
    
    Frame *frame = frameToCommitData->m_frame;
    setConstantBuffers( frame->m_model, frame->m_view, frame->m_proj, m_oglBackend, this );

    for ( ui32 geoPackageIdx = 0; geoPackageIdx<frame->m_numGeoPackages; geoPackageIdx++ ) {
        GeometryPackage *currentGeoPackage( frame->m_geoPackages[ geoPackageIdx ] );
        if ( nullptr == currentGeoPackage ) {
            continue;
        }

        /*
        SetMaterialStageCmdData *data(nullptr);
        CPPCore::TArray<ui32> primGroups;
        for (ui32 geoIdx = 0; geoIdx < currentGeoPackage->m_numNewGeo; ++geoIdx) {
            Geometry *geo = currentGeoPackage->m_newGeo[geoIdx];
            if (nullptr == geo) {
                osre_debug(Tag, "Geometry-pointer is a nullptr.");
                return false;
            }

            // register primitive groups to render
            for (ui32 i = 0; i < geo->m_numPrimGroups; ++i) {
                const ui32 primIdx(m_oglBackend->addPrimitiveGroup(&geo->m_pPrimGroups[i]));
                primGroups.add(primIdx);
            }
            // create the default material
            data = setupMaterial(geo->m_material, m_oglBackend, this);
        }
        m_vertexArray = setupBufferForGeoBundle(currentGeoPackage, m_oglBackend, m_renderCmdBuffer->getActiveShader());
        data->m_vertexArray = m_vertexArray;


        // setup the draw calls
        if (0 == currentGeoPackage->m_numInstances) {
            setupPrimDrawCmd(primGroups, m_oglBackend, this, m_vertexArray);
        } else {
            setupInstancedDrawCmd(primGroups, frame, m_oglBackend, this, m_vertexArray);
        }
        */
        
        CPPCore::TArray<ui32> primGroups;
        for (ui32 geoIdx = 0; geoIdx < currentGeoPackage->m_numNewGeo; ++geoIdx) {
            Geometry *geo = currentGeoPackage->m_newGeo[geoIdx];
            if (nullptr == geo) {
                osre_debug(Tag, "Geometry-pointer is a nullptr.");
                return false;
            }

            // register primitive groups to render
            for (ui32 i = 0; i < geo->m_numPrimGroups; ++i) {
                const ui32 primIdx( m_oglBackend->addPrimitiveGroup( &geo->m_pPrimGroups[ i ]) );
                primGroups.add( primIdx );
            }

            // create the default material
            SetMaterialStageCmdData *data = setupMaterial(geo->m_material, m_oglBackend, this);

            // setup vertex array, vertex and index buffers
            m_vertexArray = setupBuffers(geo, m_oglBackend, m_renderCmdBuffer->getActiveShader());
            if (nullptr == m_vertexArray) {
                osre_debug(Tag, "Vertex-Array-pointer is a nullptr.");
                return false;
            }
            data->m_vertexArray = m_vertexArray;

            if (frame->m_numLights > 0) {
                setupLights(frame->m_numLights, frame->m_lights, m_oglBackend, this);
            }

            // setup the draw calls
            if (0 == currentGeoPackage->m_numInstances) {
                setupPrimDrawCmd( geo->m_localMatrix, geo->m_model, primGroups, m_oglBackend, this, m_vertexArray);
            } else {
                setupInstancedDrawCmd(primGroups, frame, m_oglBackend, this, m_vertexArray);
            }
        }
        primGroups.resize( 0 );
    }

    // setup global parameter
    if( frame->m_numVars > 0 ) {
        for( ui32 i = 0; i < frame->m_numVars; i++ ) {
            setupParameter( frame->m_vars[ i ], m_oglBackend, this );
        }
    }

    if ( nullptr != frame->m_geoPackages ) {
        delete[] frame->m_geoPackages;
        frame->m_geoPackages = nullptr;
        frame->m_numGeoPackages = 0;
    }

    for ( ui32 i=0; i<frame->m_numGeoUpdates; ++i ) {
        Geometry *geo = frame->m_geoUpdates[ i ];
        if ( nullptr == geo ) {
            osre_debug(Tag, "Geometry-update-pointer is a nullptr.");
            return false;
        }

        OGLBuffer *buffer( m_oglBackend->getBufferById( geo->m_id ) );
        if (nullptr != buffer) {
            m_oglBackend->bindBuffer(buffer);
            m_oglBackend->copyDataToBuffer(buffer, geo->m_vb->m_data, geo->m_vb->m_size, geo->m_vb->m_access);
            m_oglBackend->unbindBuffer(buffer);
        }
    }

    delete[] frame->m_geoUpdates;
    frame->m_geoUpdates = nullptr;
    frame->m_numGeoUpdates = 0;

    m_oglBackend->useShader( nullptr );

    return true;
}

bool OGLRenderEventHandler::onShutdownRequest( const Common::EventData *eventData ) {
    OSRE_ASSERT( nullptr != eventData );

    m_isRunning = false;

    return true;
}

bool OGLRenderEventHandler::onResizeRenderTarget( const Common::EventData *eventData ) {
    OSRE_ASSERT( nullptr != eventData );

    ResizeEventData *data = ( ResizeEventData* ) eventData;
    const ui32 x( data->m_x );
    const ui32 y( data->m_y );
    const ui32 w( data->m_w );
    const ui32 h( data->m_h );
    m_oglBackend->setViewport( x, y, w, h );

    return true;
}

} // Namespace RenderBackend
} // Namespace OSRE
