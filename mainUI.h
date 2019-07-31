/////////////////////////////////////////////////////////////////////////////
// Name:        mainUI.h
// Purpose:     Layout mainUI
// Author:      qiubo2@lenovo.com
// Modified by:
// Created:     2019.07.11
// Copyright:   (c) qiubo2@lenovo.com
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/time.h>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <sstream>
#include <stdint.h>
#include "opencv2/core/core.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"


using namespace std;
using namespace cv;

class DrawShape;
class MyFrame;
class MyCanvas;
class MyApp;

// controls and menu constants
enum
{
    LAYOUT_TEST_PROPORTIONS = 101,
    LAYOUT_QUIT = wxID_EXIT,
    LAYOUT_ABOUT = wxID_ABOUT
};
//声明一个新的事件种类用于我们的MyThread类
wxDECLARE_EVENT(wxEVT_COMMAND_MYTHREAD_COMPLETED, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_MYTHREAD_UPDATE, wxThreadEvent);

 
// 为线程事件传输数据，你可以定义成其他的结构
typedef struct THREAD_MSG
{
	long unsigned int    threadId;
    intptr_t                        cameras_index;
	wxBitmap                   bitmap;
}  THREAD_MSG_TYPE;

class MyThread : public wxThread
{
public:
    MyThread(MyFrame* draw_rect);
    virtual void *Entry();

private:
    void displaySynchronous(const wxEventType & evtType, const wxBitmap bitmap, intptr_t cameras_index);
    string make_filename(intptr_t cameras_order);
    void save_imagefiles(Mat &mat);
    void play_cameras();
    void stop_cameras();
    void pause_cameras();
    MyFrame* gui;
};

class MyApp: public wxApp
{
public:
    MyApp(){};
    void DrawHandle();
    bool OnInit();
    MyFrame* GetFrame() const { return frame; }
    void SetFrame(MyFrame* frames) { frame = frames; }
private:
    MyFrame*    frame;    
};
wxDECLARE_APP(MyApp);

class DrawShape: public wxObject
{
public:
    DrawShape(const wxBitmap& bitmap);
    ~DrawShape(){};
    bool Draw(wxDC& dc);
    wxBitmap& GetBitmap() const { return (wxBitmap&) m_bitmap; }
    void SetBitmap(const wxBitmap& bitmap) { m_bitmap = bitmap; }
    wxPoint GetPosition() const { return m_pos; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }
protected:
    wxPoint        m_pos;
    wxBitmap    m_bitmap;  
};

class MyCanvas: public wxScrolledWindow
{
public:
    MyCanvas( wxWindow *parent, MyFrame* frame, wxWindowID, const wxPoint &pos, const wxSize &size );
    ~MyCanvas();
    void OnPaint( wxPaintEvent &event );
    void DrawShapes(wxDC& dc);
    void ClearShapes();
    wxList& GetDisplayList() { return m_displayList; }
private:
    wxList          m_displayList; // A list of DragShapes
    MyFrame*  m_owner;
    int m_nCurrentProgress;
    wxDECLARE_ABSTRACT_CLASS(MyCanvas);
    wxDECLARE_EVENT_TABLE();
};

// the main frame class
class MyFrame : public wxFrame
{
public:
    MyFrame();
    ~MyFrame() ;
    void initCameras();
    MyCanvas* GetCanvas0() const { return m_canvas_0; }
    void SetCanvas0(MyCanvas* canvas) { m_canvas_0 = canvas; }
    MyCanvas* GetCanvas1() const { return m_canvas_1; }
    void SetCanvas1(MyCanvas* canvas) { m_canvas_1 = canvas; }
    MyCanvas* GetCanvas2() const { return m_canvas_2; }
    void SetCanvas2(MyCanvas* canvas) { m_canvas_2 = canvas; }
    MyCanvas* GetCanvas3() const { return m_canvas_3; }
    void SetCanvas3(MyCanvas* canvas) { m_canvas_3 = canvas; }
    MyFrame* GetFrame() const { return frame; }
    void SetFrame(MyFrame* frames) { frame = frames; }
    bool GetIsCapture() {  return is_capture; }
    void SetIsCapture(bool isCapture) { is_capture = isCapture; }
    void ImageFaceDatas(string files, intptr_t order = 0);
    string GetImagesFileName(intptr_t Orders);
    void OnThreadUpdate(wxCommandEvent& evt);
    void OnThreadCompletion(wxCommandEvent& evt);
private:
   MyCanvas*       m_canvas_0;
   MyCanvas*       m_canvas_1;
   MyCanvas*       m_canvas_2;
   MyCanvas*       m_canvas_3;
   MyFrame*        frame;
   MyThread*      m_pthread;
   bool                    is_capture = true;
    void TestProportions(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void InitFlexSizer(wxFlexGridSizer *sizer, wxWindow* parent);
    void ConnectionDB();
    wxDECLARE_EVENT_TABLE();
};

