#include <osre/RenderBackend/RenderBackendService.h>
#include <osre/RenderBackend/RenderCommon.h>
#include <osre/Properties/ConfigurationMap.h>
#include <osre/Threading/SystemTask.h>

#include "OGLRenderer/OGLRenderEventHandler.h"

namespace OSRE {
namespace RenderBackend {

using namespace ::OSRE::Common;
using namespace ::OSRE::Threading;
using namespace ::OSRE::Properties;

//-------------------------------------------------------------------------------------------------
RenderBackendService::RenderBackendService()
: AbstractService( "renderbackend/renderbackendserver" )
, m_RenderTaskPtr()
, m_pConfigMap( nullptr )
, m_ownConfig( false ) {
    // empty
}

//-------------------------------------------------------------------------------------------------
RenderBackendService::~RenderBackendService() {
    if( m_ownConfig ) {
        delete m_pConfigMap;
        m_pConfigMap = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
bool RenderBackendService::onOpen() {
    if (!m_pConfigMap) {
        m_pConfigMap = new Properties::ConfigurationMap;
        m_ownConfig = true;
    }

    if( !m_RenderTaskPtr.isValid() ) {
        m_RenderTaskPtr.init( SystemTask::create( "render_task" ) );
    }

    m_RenderTaskPtr->start( nullptr );
    m_RenderTaskPtr->attachEventHandler( new OGLRenderEventHandler );

    return true;
}

//-------------------------------------------------------------------------------------------------
bool RenderBackendService::onClose() {
    if( !m_RenderTaskPtr.isValid() ) {
        return false;
    }

    if ( m_RenderTaskPtr->isRunning() ) {
        m_RenderTaskPtr->detachEventHandler();        
        m_RenderTaskPtr->stop();
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
bool RenderBackendService::onUpdate( d32 timediff ) {
    if ( !m_RenderTaskPtr.isValid() ) {
        return false;
    }
    
    // synchronizing event with render back-end
    bool result( m_RenderTaskPtr->sendEvent( &OnRenderFrameEvent, nullptr ) );
    m_RenderTaskPtr->await();

    return result;
}

//-------------------------------------------------------------------------------------------------
void RenderBackendService::setConfig( const ConfigurationMap *pConfiguration ) {
    m_pConfigMap = pConfiguration;
}

//-------------------------------------------------------------------------------------------------
const Properties::ConfigurationMap *RenderBackendService::getConfig() const {
    return m_pConfigMap;
}

//-------------------------------------------------------------------------------------------------
void RenderBackendService::sendEvent( const Event *pEvent, const EventData *pEventData ) {
    if ( !m_RenderTaskPtr.isValid() ) {
        return;
    }

    m_RenderTaskPtr->sendEvent( pEvent, pEventData );
}

//-------------------------------------------------------------------------------------------------

} // Namespace RenderBackend
} // Namespace OSRE