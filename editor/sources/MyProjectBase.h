///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/toolbar.h>
#include <wx/statusbr.h>
#include <wx/panel.h>
#include <wx/aui/auibook.h>
#include <wx/frame.h>
#include <wx/aui/aui.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class AppFrame
///////////////////////////////////////////////////////////////////////////////
class AppFrame : public wxFrame
{
	private:

	protected:
		enum
		{
			wxID_NewScene = 6000,
			wxID_OpenScene,
			wxID_SaveScene,
		};

		wxMenuBar* m_menubar;
		wxMenu* m_menuFile;
		wxToolBar* m_toolBar;
		wxToolBarToolBase* m_openScene;
		wxToolBarToolBase* m_saveScene;
		wxStatusBar* m_statusBar;
		wxPanel* m_sceneTreePanel;
		wxAuiNotebook* m_componentPropertiesPanel;

	public:

		AppFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("darmok editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 810,503 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		wxAuiManager m_mgr;

		~AppFrame();

};

