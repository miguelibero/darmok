///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "MyProjectBase.h"

///////////////////////////////////////////////////////////////////////////

AppFrame::AppFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	m_mgr.SetManagedWindow(this);
	m_mgr.SetFlags(wxAUI_MGR_DEFAULT);

	m_menubar = new wxMenuBar( 0 );
	m_menuFile = new wxMenu();
	wxMenuItem* m_menuItemNewScene;
	m_menuItemNewScene = new wxMenuItem( m_menuFile, wxID_NewScene, wxString( _("New Scene") ) + wxT('\t') + wxT("Ctrl + N"), wxEmptyString, wxITEM_NORMAL );
	#ifdef __WXMSW__
	m_menuItemNewScene->SetBitmaps( wxNullBitmap );
	#elif (defined( __WXGTK__ ) || defined( __WXOSX__ ))
	m_menuItemNewScene->SetBitmap( wxNullBitmap );
	#endif
	m_menuFile->Append( m_menuItemNewScene );

	m_menubar->Append( m_menuFile, _("File") );

	this->SetMenuBar( m_menubar );

	m_toolBar = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL );
	m_openScene = m_toolBar->AddTool( wxID_OpenScene, _("Open Scene..."), wxArtProvider::GetBitmap( wxASCII_STR(wxART_FILE_OPEN), wxASCII_STR(wxART_MENU) ), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );

	m_saveScene = m_toolBar->AddTool( wxID_SaveScene, _("Save Scene"), wxArtProvider::GetBitmap( wxASCII_STR(wxART_FILE_SAVE), wxASCII_STR(wxART_TOOLBAR) ), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );

	m_toolBar->Realize();
	m_mgr.AddPane( m_toolBar, wxAuiPaneInfo() .Top() .CaptionVisible( false ).CloseButton( false ).PaneBorder( false ).Movable( false ).Dock().Fixed().BottomDockable( false ).TopDockable( false ).LeftDockable( false ).RightDockable( false ).Floatable( false ) );

	m_statusBar = this->CreateStatusBar( 1, wxSTB_SIZEGRIP, wxID_ANY );
	m_sceneTreePanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mgr.AddPane( m_sceneTreePanel, wxAuiPaneInfo() .Name( wxT("sceneTree") ).Left() .Caption( _("Scene Tree") ).PinButton( true ).Dock().Resizable().FloatingSize( wxDefaultSize ).BestSize( wxSize( 200,-1 ) ) );

	m_componentPropertiesPanel = new wxAuiNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE );
	m_mgr.AddPane( m_componentPropertiesPanel, wxAuiPaneInfo() .Name( wxT("componentProperties") ).Right() .Caption( _("Component Properties") ).PinButton( true ).Dock().Resizable().FloatingSize( wxDefaultSize ).BestSize( wxSize( 200,-1 ) ) );



	m_mgr.Update();
	this->Centre( wxBOTH );
}

AppFrame::~AppFrame()
{
	m_mgr.UnInit();

}
