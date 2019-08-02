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
#include <iomanip>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "opencv2/core/core.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"


using namespace std;
using namespace cv;

class MyFrame;
class MyApp;
class WxImagePanel;
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

class MyThread : public wxThread
{
public:
    MyThread(MyFrame* draw_rect);
    virtual void *Entry();

private:
    void displaySynchronous(const wxEventType & evtType, const string imagePath, intptr_t cameras_index, bool isDefect) ;
    string make_filename(intptr_t cameras_order);
    void save_imagefiles(Mat &mat);
    bool delete_imageFiles(string fileName);
    bool copy_imageFiles(string fileName, string toFileName);
    void play_cameras();
    void stop_cameras();
    void pause_cameras();
    MyFrame* gui;
};

class MyApp: public wxApp
{
public:
    MyApp(){};
    bool OnInit();

private:
    MyFrame*    frame;    
};
wxDECLARE_APP(MyApp);


class WxImagePanel : public wxPanel
{
    wxImage image;
    wxBitmap resized;
    int w, h;
public:
    WxImagePanel(wxFrame* parent, wxString file, wxBitmapType format);
    void loaderImages(wxString file, wxBitmapType format);
    bool delete_imageFiles(string fileName);
    void paintEvent(wxPaintEvent & evt);
    void paintNow();
    void OnSize(wxSizeEvent& event);
    void render(wxDC& dc);
    // some useful events
    /*
     void mouseMoved(wxMouseEvent& event);
     void mouseDown(wxMouseEvent& event);
     void mouseWheelMoved(wxMouseEvent& event);
     void mouseReleased(wxMouseEvent& event);
     void rightClick(wxMouseEvent& event);
     void mouseLeftWindow(wxMouseEvent& event);
     void keyPressed(wxKeyEvent& event);
     void keyReleased(wxKeyEvent& event);
     */
    DECLARE_EVENT_TABLE()
};

// the main frame class
class MyFrame : public wxFrame
{
public:
    MyFrame();
    ~MyFrame() ;
    bool GetIsCapture() {  return is_capture; }
    void SetIsCapture(bool isCapture) { is_capture = isCapture; }
    void OnThreadUpdate(wxCommandEvent& evt);
    void OnThreadCompletion(wxCommandEvent& evt);
private:
    MyThread*      m_pthread;
    WxImagePanel* drawPanel0;
    WxImagePanel* drawPanel1;
    WxImagePanel* drawPanel2;
    WxImagePanel* drawPanel3;
    bool                    is_capture = true;
    void CreateThread();
    void TestProportions(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSize(wxSizeEvent &event);
    void OnQuit(wxCommandEvent& event);
    void ConnectionDB();
    wxDECLARE_EVENT_TABLE();
};

