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
#include <osre/Platform/PlatformInterface.h>
#include <src/Engine/Platform/PlatformPluginFactory.h>
#include <osre/Common/EventTriggerer.h>
#include <osre/Properties/Settings.h>
#include <osre/UI/FontManager.h>
#include <osre/UI/UiItemFactory.h>
#ifdef OSRE_WINDOWS
#   include <src/Engine/Platform/win32/Win32RenderContext.h>
#endif // OSRE_WINDOWS
#include "Engine/Platform/sdl2/SDL2Window.h"
#include "Engine/Platform/sdl2/SDL2EventQueue.h"
#include <src/Engine/Platform/sdl2/SDL2RenderContext.h>
#include <src/Engine/Platform/sdl2/SDL2Timer.h>
#include <src/Engine/Platform/sdl2/SDL2SystemInfo.h>

#include <GL/glew.h>
#include <SDL.h>

namespace OSRE {
namespace Platform {

using namespace ::OSRE::Common;
using namespace ::OSRE::Properties;
using namespace ::OSRE::UI;

PlatformInterface *PlatformInterface::s_instance( nullptr );

static const String PlatformPluginName[ static_cast<i32>( PluginType::MaxPlugin ) ] = {
#ifdef OSRE_WINDOWS
    "WindowsPlugin",
#endif // OSRE_WINDOWS
    "SDL2Plugin"
};

static const String Tag = "PlatformInterface";

PlatformInterface::PlatformInterface( const Settings *config )
: AbstractService( "platform/platforminterface" )
, m_config( config )
#ifdef OSRE_WINDOWS
, m_type( PluginType::WindowsPlugin )
#else
, m_type( PluginType::SDL2Plugin )
#endif // OSRE_WINDOWS
, m_rootSurface( nullptr )
, m_oseventHandler( nullptr )
, m_renderContext( nullptr )
, m_timer( nullptr )
, m_dynLoader( nullptr )
, m_systemInfo( nullptr ) {
    // empty
}

PlatformInterface::~PlatformInterface() {
    m_config = nullptr;
}

PlatformInterface *PlatformInterface::create( const Settings *config ) {
    if( nullptr == s_instance ) {
        s_instance = new PlatformInterface( config );
    }

    return s_instance;
}

void PlatformInterface::destroy( ) {
    if( nullptr == s_instance ) {
        return;
    }

    s_instance->close();
    delete s_instance;
    s_instance = nullptr;
}

PlatformInterface *PlatformInterface::getInstance( ) {
    return s_instance;
}

AbstractPlatformEventQueue *PlatformInterface::getPlatformEventHandler() const {
    return m_oseventHandler;
}

AbstractRenderContext *PlatformInterface::getRenderContext() const {
    return m_renderContext;
}

AbstractWindow *PlatformInterface::getRootWindow() const {
    return m_rootSurface;
}

AbstractTimer *PlatformInterface::getTimer() const {
    return m_timer;
}

AbstractDynamicLoader *PlatformInterface::getDynamicLoader() const {
    return m_dynLoader;
}

AbstractSystemInfo *PlatformInterface::getSystemInfo() const {
    return m_systemInfo;
}

const String &PlatformInterface::getDefaultFontName() const {
    if ( nullptr == m_config ) {
        static const String dummy( "none" );
        return dummy;
    }

    static const String font( m_config->get( Settings::DefaultFont ).getString() );
    return font;
}

PluginType PlatformInterface::getOSPluginType() {
#ifdef OSRE_WINDOWS
    return PluginType::WindowsPlugin;
#else 
    return PluginType::SDL2Plugin;
#endif // OSRE_WINDOWS
}

String PlatformInterface::getOSPluginName( PluginType type ) {
    String name( "None" );
    switch( type ) {
#ifdef OSRE_WINDOWS
        case PluginType::WindowsPlugin:
            name = PlatformPluginName[ static_cast<i32>( PluginType::WindowsPlugin ) ];
            break;
#endif // OSRE_WINDOWS

        case PluginType::SDL2Plugin:
            name = PlatformPluginName[ static_cast<i32>( PluginType::SDL2Plugin ) ];
            break;
        default:
            break;
    }

    return name;
}

bool PlatformInterface::onOpen() {
    if( !m_config ) {
        assert( nullptr != m_config );
        osre_debug( Tag, "Invalid pointer to configuration." );
        return false;
    }

    Settings::WorkingModeType appType = 
        static_cast<Settings::WorkingModeType>( m_config->get( Settings::AppType ).getInt() );

    WindowsProperties *props( nullptr );
    bool polls( false );
    if( appType == Settings::GfxApp ) {
        // get the configuration values for the window
        props = new WindowsProperties;
        bool fullscreen = false;
        props->m_x = m_config->get( Settings::WinX ).getInt();
        props->m_y = m_config->get( Settings::WinY ).getInt();
        props->m_width = m_config->get( Settings::WinWidth ).getInt();
        props->m_height = m_config->get( Settings::WinHeight ).getInt();
        props->m_colordepth = m_config->get( Settings::BPP ).getInt();
        props->m_depthbufferdepth = m_config->get( Settings::DepthBufferDepth ).getInt();
        props->m_stencildepth = m_config->get( Settings::StencilBufferDepth ).getInt();
        props->m_fullscreen = fullscreen;
        props->m_resizable = m_config->get( Settings::WindowsResizable ).getBool();
        props->m_childWindow = m_config->get(Settings::ChildWindow).getBool();
        props->m_title = m_config->get( Settings::WindowsTitle ).getString();
        polls = m_config->get( Settings::PollingMode ).getBool();
    }

    String appName = "My OSRE-Application";
    m_type = static_cast<PluginType>( m_config->get( Settings::PlatformPlugin ).getInt( ) );

    PlatformPluginFactory::init( m_type );
    osre_info( Tag, "Platform plugin created for " + PlatformInterface::getOSPluginName( m_type ) );

    m_dynLoader = PlatformPluginFactory::createDynmicLoader( m_type );
    bool result( true );
    if( appType == Settings::GfxApp ) {
        result = setupGfx( props, polls );
    }

    m_systemInfo = PlatformPluginFactory::createSystemInfo( m_type );

    return result;
}

bool PlatformInterface::onClose( ) {
    PlatformPluginFactory::release( m_type );

    delete m_oseventHandler;
    m_oseventHandler = nullptr;

    if( nullptr != m_renderContext ) {
        m_renderContext->destroy();
        delete m_renderContext;
        m_renderContext = nullptr;
    }

    return true;
}

bool PlatformInterface::onUpdate() {
    if (nullptr == m_oseventHandler) {
        return false;
    }

    return m_oseventHandler->update();
}

bool PlatformInterface::setupGfx( WindowsProperties *props, bool polls ) {
    // create the root surface
    m_rootSurface = PlatformPluginFactory::createSurface( m_type, props );
    if( !m_rootSurface->create() ) {
        delete m_rootSurface;
        osre_error( Tag, "Error while creating platform root surface." );

        m_rootSurface = nullptr;
        return false;
    }

    // install the platform event handler
    m_oseventHandler = PlatformPluginFactory::createPlatformEventHandler( m_type, m_rootSurface );
    if( !m_oseventHandler ) {
        osre_error( Tag, "Error while creating platform event handler." );
        m_rootSurface->destroy();
        m_rootSurface = nullptr;
        return false;
    }
    m_oseventHandler->enablePolling( polls );
    m_timer = PlatformPluginFactory::createTimer( m_type );

    // setup the render context
    m_renderContext = PlatformPluginFactory::createRenderContext( m_type );

    UiItemFactory::createInstance( getRootWindow() );
    FontManager::create();
        
    return true;
}

} // Namespace Platform
} // Namespace OSRE
