/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/sockqt.cpp
// Author:      Sean D'Epagnier
// Copyright:   (c) Sean D'Epagnier 2014
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_SOCKETS && defined(__UNIX__)

#include "wx/apptrait.h"
#include "wx/private/fdiomanager.h"

#include <QtCore/QSocketNotifier>

class wxQtFDIOHandler : public QSocketNotifier
{
public:
    wxQtFDIOHandler(wxFDIOHandler *handler, int fd, Type type)
        : QSocketNotifier(fd, type),
          m_handler(handler)
    {
        setEnabled(true);

        connect(this, &wxQtFDIOHandler::activated, this,
                type == QSocketNotifier::Read ?
                &wxQtFDIOHandler::OnInput :
                &wxQtFDIOHandler::OnOutput);
    }

    void OnInput()
    {
        m_handler->OnReadWaiting();
    }


    void OnOutput()
    {
        m_handler->OnWriteWaiting();
    }

private:
    wxFDIOHandler *m_handler;
};

WX_DECLARE_HASH_MAP( int, wxUIntPtr, wxIntegerHash,
                     wxIntegerEqual, wxQtFDIOHandlerHashMap );

class QtFDIOManager : public wxFDIOManager
{
public:
    virtual int AddInput(wxFDIOHandler *handler, int fd, Direction d) wxOVERRIDE
    {
        wxQtFDIOHandlerHashMap &fds = (d == INPUT) ? m_fds_in : m_fds_out;

        wxASSERT(fds.find(fd) == fds.end());

        fds[fd] = (wxUIntPtr)new wxQtFDIOHandler(handler, fd,
                                                 (d == INPUT) ? 
                                                 QSocketNotifier::Read :
                                                 QSocketNotifier::Write);
        return fd;
    }

    virtual void
    RemoveInput(wxFDIOHandler* WXUNUSED(handler), int fd, Direction d) wxOVERRIDE
    {
        wxQtFDIOHandlerHashMap &fds = (d == INPUT) ? m_fds_in : m_fds_out;
        wxQtFDIOHandlerHashMap::const_iterator it = fds.find(fd);

        wxASSERT(it != fds.end());
        delete (wxQtFDIOHandler*)it->second;
        fds.erase(fd);
    }

    wxQtFDIOHandlerHashMap m_fds_in, m_fds_out;
};

wxFDIOManager *wxGUIAppTraits::GetFDIOManager()
{
    static QtFDIOManager s_manager;
    return &s_manager;
}

#endif // wxUSE_SOCKETS && __UNIX__
