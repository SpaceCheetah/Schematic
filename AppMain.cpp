#include "AppMain.h"
#include "FrameMain.h"
#include <wx/image.h>

wxIMPLEMENT_APP(AppMain);

bool AppMain::OnInit() {
    wxInitAllImageHandlers();
    auto* frame = new FrameMain(); //this is not a memory leak; wxWidgets classes derived from wxWindow delete themselves.
    frame->Show(true);
    return true;
}
