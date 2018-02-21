#include <osre/App/AppBase.h>
#include <osre/Common/Logger.h>
#include <osre/Common/ArgumentParser.h>
#include <osre/Properties/Settings.h>
#include <osre/Assets/AssetRegistry.h>
#include <osre/Scene/World.h>
#include <osre/IO/Uri.h>
#include <osre/Assets/AssetDataArchive.h>
#include <osre/Platform/PlatformOperations.h>
#include <osre/Platform/PlatformInterface.h>
#include <osre/Platform/AbstractSurface.h>
#include <osre/UI/Panel.h>
#include <osre/UI/ButtonBase.h>
#include <osre/UI/Screen.h>
#include <osre/RenderBackend/RenderBackendService.h>

using namespace ::OSRE;
using namespace ::OSRE::Common;
using namespace ::OSRE::Scene;
using namespace ::OSRE::Platform;
using namespace ::OSRE::Assets;
using IO::Uri;

static const String SupportedArgs = "help:api:gen_project:asset_path";
static const String Descs         = "Shows the help:The render API:Generates a template project:Path to media";
static const String Tag = "osre_ed";

static const ui32 ButtonWidth = 196;
static const ui32 ButtonHeight = 20;

namespace Details {
    static const String Version = "0.1";

    static String createWindowsTitle( const String &project ) {
        String title = project + " | OSRE_Ed Version " + Version;
        return title;
    }
}

class EditorApp : public App::AppBase {
    World             *m_world;
    AssetDataArchive  *m_project;
    IO::Uri            m_projecUri;
    UI::Screen        *m_screen;
    UI::Panel         *m_panel;
    RenderBackend::TransformMatrixBlock m_transformMatrix;

public:
    EditorApp( int argc, char *argv[] )
    : AppBase( argc, argv, SupportedArgs, Descs )
    , m_world( nullptr )
    , m_project( nullptr )
    , m_projecUri()
    , m_screen( nullptr )
    , m_panel( nullptr ) {
        // empty
    }

    virtual ~EditorApp() {
        // empty
    }

    bool openWorld( const String &projectName ) {
        m_project = new AssetDataArchive( 1 );
        m_project->setName( projectName );

        return true;
    }

    bool closeWorld() {
        if ( nullptr == m_project ) {
            return true;
        }

        return true;
    }

    bool loadWorld( const IO::Uri &loc ) {
        if ( loc.isEmpty() ) {
            PlatformOperations::getFileOpenDialog( AssetDataArchive::getExtension().c_str(), m_projecUri );
        } else {
            m_projecUri = loc;
        }

        const String &name = m_projecUri.getResource();
        if ( name.empty() ) {
            return false;
        }

        m_project->setName( name );

        return true;
    }

    bool saveWorld( const IO::Uri &loc ) {
        if ( !loc.isValid() ) {
            PlatformOperations::getFileOpenDialog( AssetDataArchive::getExtension().c_str(), m_projecUri );
        } else {
            m_projecUri = loc;
        }

        return false;
    }

protected:
    void newProjectCallback( ui32 id, void *data ) {
        const String name = "NewProject";
        openWorld( name );
    }

    void openProjectCallback( ui32 id, void *data ) {
        IO::Uri loc;
        PlatformOperations::getFileOpenDialog( "All\0 *.*\0", loc );
        loadWorld( loc );
    }

    void closeProjectCallback( ui32 id, void *data ) {
        PlatformOperations::DlgResults results;
        PlatformOperations::getDialog("Close project", 
                "Do you really want to save the project?", 
                PlatformOperations::DlgButton_YesNo, results );
        if (PlatformOperations::DlgButtonRes_Yes == results) {
            IO::Uri loc;
            PlatformOperations::getFileSaveDialog("All\0 *.*\0", loc);
            saveWorld(loc);
        }
    }

    void quitEd( ui32 id, void *data ) {
        // todo
    }

    void setupProjectPanel() {
        // Create panel
        m_panel = new UI::Panel("project_panel_id", UI::UiFlags::Resizable, m_screen);
        m_panel->setRect(10, 10, 200, 600);

        // Create new project
        UI::ButtonBase *new_proj(UI::ButtonBase::createBaseButton("new_proj_id", "New Project", m_panel));
        new_proj->setRect(12, 12, ButtonWidth, ButtonHeight);
        new_proj->registerCallback(UI::ButtonBase::ButtonPressed, UI::UIFunctor::Make(this, &EditorApp::newProjectCallback));

        // Open existing project
        UI::ButtonBase *open_proj(UI::ButtonBase::createBaseButton("open_proj_id", "Open Project", m_panel));
        open_proj->setRect(12, 34, ButtonWidth, ButtonHeight);
        open_proj->registerCallback(UI::ButtonBase::ButtonPressed, UI::UIFunctor::Make(this, &EditorApp::openProjectCallback));

        // Close opened project
        UI::ButtonBase *close_proj(UI::ButtonBase::createBaseButton("close_proj_id", "Close project", m_panel));
        close_proj->setRect(12, 56, ButtonWidth, ButtonHeight);
        close_proj->registerCallback(UI::ButtonBase::ButtonPressed, UI::UIFunctor::Make(this, &EditorApp::closeProjectCallback));

        // Quit button
        UI::ButtonBase *quit_ed(UI::ButtonBase::createBaseButton("quit_id", "Quit", m_panel));
        quit_ed->setRect(12, 578, ButtonWidth, ButtonHeight);
        quit_ed->registerCallback(UI::ButtonBase::ButtonPressed, UI::UIFunctor::Make(this, &EditorApp::quitEd));
    }

    void setupUI() {
        m_screen = AppBase::createScreen("OSRE-Editor");
        setupProjectPanel();
    }

    virtual bool onCreate( Properties::Settings *settings = nullptr ) {
        const ArgumentParser &argParser = getArgumentParser();
        if ( argParser.hasArgument( "help" ) ) {
            String help( argParser.showHelp() ); 
            osre_info( Tag, help );
            return false;
        }

        Properties::Settings *baseSettings( AppBase::getSettings() );
        if ( nullptr == baseSettings ) {
            return false;
        }

        baseSettings->setString( Properties::Settings::WindowsTitle, "osre_ed!" );
        if ( !AppBase::onCreate( baseSettings ) ) {
            return false;
        }

#ifdef OSRE_WINDOWS
        Assets::AssetRegistry::registerAssetPath( "assets", "../../media" );
#else
        Assets::AssetRegistry::registerAssetPath( "assets", "../media" );
#endif // OSRE_WINDOWS

        setupUI();

        m_transformMatrix.m_model = glm::rotate(m_transformMatrix.m_model, 0.0f, glm::vec3(1, 1, 0));
        m_transformMatrix.update();
        AppBase::getRenderBackendService()->setMatrix("MVP", m_transformMatrix.m_mvp);

        return true;
    }

    bool onLoadWorld() {
        return false;
    }

    bool onSaveWorld() {
        return false;
    }

    bool onImportStage() {
        return false;
    }


    void setProjectTitle( const String &project ) {
        String newWindowsTitle = Details::createWindowsTitle( project );
        PlatformInterface::getInstance()->getRootSurface()->setWindowsTitle( newWindowsTitle );
    }
};

OSRE_MAIN( EditorApp )

