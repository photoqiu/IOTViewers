/////////////////////////////////////////////////////////////////////////////
// Name:        layout.cpp
// Purpose:     Layout sample
// Author:      Julian Smart
// Modified by: Robin Dunn, Vadim Zeitlin
// Created:     04/01/98
// Copyright:   (c) 1998 Julian Smart
//                  2005 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include "wx/sizer.h"
#include "wx/gbsizer.h"
#include "wx/statline.h"
#include "wx/notebook.h"
#include "wx/spinctrl.h"
#include "wx/wrapsizer.h"
#include "wx/generic/stattextg.h"
#include "wx/dcbuffer.h"
#include "wx/file.h"
#include "wx/filename.h"
#include "wx/metafile.h"
#include "wx/graphics.h"
#include "wx/mstream.h"
#include "wx/wfstream.h"
#include "wx/quantize.h"
#include "wx/scopedptr.h"
#include "wx/stopwatch.h"
#include "wx/versioninfo.h"
#include <wx/bitmap.h>

#include "wx/thread.h"
#include "wx/dynarray.h"
#include "wx/image.h"
#if !wxUSE_THREADS
    #error "This sample requires thread support!"
#endif 
// #include <mysql/mysql.h>
// Include files to use the pylon API.
#include  "pylon/PylonIncludes.h"
#include "mainUI.h"
#if wxUSE_CLIPBOARD
    #include "wx/dataobj.h"
    #include "wx/clipbrd.h"
#endif // wxUSE_CLIPBOARD
#if defined(__WXMSW__)
    #ifdef wxHAVE_RAW_BITMAP
    #include "wx/rawbmp.h"
    #endif
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
    #define wxHAVE_RAW_BITMAP
    #include "wx/rawbmp.h"
#endif
#if wxUSE_ZLIB && wxUSE_STREAMS
    #include "wx/zstream.h"
#endif

#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "./sample.xpm"
#endif
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
#include "ConfigurationEventPrinter.h"
#include "ImageEventPrinter.h"


#define GUITHREAD_NUM_UPDATES       50

using namespace Pylon;
using namespace GenApi;
using namespace std;

static const uint32_t c_countOfImagesToGrab =  500000;
static const size_t c_maxCamerasToUse = 8;
static const int imagesW  =  480;
static const int imagesH  =  320;
wxDEFINE_EVENT(wxEVT_COMMAND_MYTHREAD_COMPLETED, wxThreadEvent);//定义事件种类
wxDEFINE_EVENT(wxEVT_COMMAND_MYTHREAD_UPDATE, wxThreadEvent);

// ----------------------------------------------------------------------------
// MyApp n                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
// ----------------------------------------------------------------------------
wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() ) {
        return false;
    }
    ::wxInitAllImageHandlers();
    frame = new MyFrame();
    frame->SetIsCapture(false);
    frame->Show(true); 
    return true;
}

MyThread::MyThread(MyFrame* draw_rect):wxThread()
{
    gui = draw_rect;
}

void *MyThread::Entry()
{
    play_cameras();
}

void MyThread::play_cameras()
{
    PylonInitialize();
     try
    {
		CTlFactory& tlFactory = CTlFactory::GetInstance();
		DeviceInfoList_t devices;
        Mat openCvImage;
        CGrabResultPtr ptrGrabResult;
        CImageFormatConverter Format_converter;
        CPylonImage targetImage;
        string filesName;
        wxImage images;
        Format_converter.OutputPixelFormat = PixelType_BGR8packed;
		if (tlFactory.EnumerateDevices(devices) == 0)
		{
			throw RUNTIME_EXCEPTION("No camera present.");
			PylonTerminate();
			return;
		}
        CInstantCameraArray cameras(min(devices.size(), c_maxCamerasToUse));
		cout << "Using Total Device Number is : " << cameras.GetSize() << endl;
        for (size_t i = 0; i < cameras.GetSize(); ++i)
		{
            cameras[i].Attach(tlFactory.CreateDevice(devices[i]));
           cout << "Using device " << cameras[ i ].GetDeviceInfo().GetModelName() << endl;
        }
        cameras.StartGrabbing();
        do 
        {
            if ( TestDestroy() ) 
            {
                cameras.StopGrabbing();
                cameras.Close();
                PylonTerminate();
                return ;
            }
            cameras.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);
            intptr_t cameraContextValue = ptrGrabResult->GetCameraContext();
            cout << "Camera " <<  cameraContextValue << ": " << cameras[ cameraContextValue ].GetDeviceInfo().GetModelName() << endl;
            cout << "GrabSucceeded: " << ptrGrabResult->GrabSucceeded() << endl;
            cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
            cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
            const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
            cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;
            Format_converter.Convert(targetImage, ptrGrabResult);
            filesName = make_filename(cameraContextValue);
            openCvImage =  Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) targetImage.GetBuffer());
            imwrite(filesName, openCvImage);
            WaitObject::Sleep(50);
            bool isDefect = false;
            displaySynchronous(wxEVT_COMMAND_MYTHREAD_UPDATE, filesName, cameraContextValue, isDefect);
            WaitObject::Sleep(10);
            //////////////////////////////////////////////////////////////////////
            cout << "the image file name is : " << filesName << endl;
        } while (cameras.IsGrabbing());
        cameras.StopGrabbing();
        cameras.Close();
        PylonTerminate();
    }
    catch (const GenericException &e)
    {
        (void)wxMessageBox(e.GetDescription(), wxT("视频设备-错误信息"), wxOK|wxICON_INFORMATION);
        play_cameras();
        PylonTerminate();
    }
}

void MyThread::displaySynchronous(const wxEventType & evtType, const string imagePath, intptr_t cameras_index, bool isDefect) 
{
    if ( TestDestroy() ) 
    {
        delete_imageFiles(imagePath);
        return ;
    }
    wxThreadEvent * threadEvt = new wxThreadEvent(evtType);
    wxString datas(imagePath.c_str());
	unsigned long dataIndex = cameras_index;
    int camerasIndex = 0;
    if (cameras_index == 1) 
    {
        camerasIndex = 1;
    } 
    else if (cameras_index == 2) 
    {
        camerasIndex = 2;
    }
    else if (cameras_index == 3) 
    {   
        camerasIndex = 3;
    }
    threadEvt->SetInt ( camerasIndex );
    threadEvt->SetString ( datas );
    wxQueueEvent(gui, threadEvt->Clone());
}

bool MyThread::delete_imageFiles(string fileName)
{
    const char* savePath = (char*)fileName.c_str();
    if((access(savePath, F_OK)) != -1)   
    {   
        if(remove(savePath)==0)
        {
            return true;
        }
        else
        {
            cout<<"删除失败"<<endl;
            return false;
        }
    }   
    else  
    {   
        return false;
    }
}

bool MyThread::copy_imageFiles(string fileName, string toFileName)
{
    const char* fileNameFrom = (char*)fileName.c_str();
    const char* fileNameTo = (char*)toFileName.c_str();
    if((access(fileNameFrom, F_OK)) != -1)   
    {
        ifstream in (fileNameFrom);
        ofstream out (fileNameTo);
        out << in.rdbuf();
        out.close();
        in.close();
        return true;
    } 
    else 
    {

        return false;
    }
}

string MyThread::make_filename(intptr_t cameras_order) 
{
     timeval curTime;
    gettimeofday(&curTime, NULL);
    srand((unsigned)time(NULL)); 
    int a = 33;
    int b = 1000;
    int milli = curTime.tv_usec / 1000;
    unsigned long micro = curTime.tv_sec * (uint64_t)1000000 + curTime.tv_usec;
    stringstream OrderStr;
    stringstream microTimer;
    stringstream randStr;
    stringstream ss; 
    string FileName;
    int getRand = a + (int)b * rand() / RAND_MAX;
    OrderStr << cameras_order;
    microTimer << micro;
    randStr << getRand;
    srand((unsigned)time(NULL));
    ss << "/soft/wxWidgets-3.0.4/gtk-build/samples/cameras/cachedatas/GrabbedImage_" << OrderStr.str() << "_" << randStr.str() << "_" << microTimer.str() << ".png";
    ss >> FileName;
    return FileName;
}

// ----------------------------------------------------------------------------
// WxImagePanel
// ----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WxImagePanel, wxPanel)
    // some useful events
    /*
    EVT_MOTION(wxImagePanel::mouseMoved)
    EVT_LEFT_DOWN(wxImagePanel::mouseDown)
    EVT_LEFT_UP(wxImagePanel::mouseReleased)
    EVT_RIGHT_DOWN(wxImagePanel::rightClick)
    EVT_LEAVE_WINDOW(wxImagePanel::mouseLeftWindow)
    EVT_KEY_DOWN(wxImagePanel::keyPressed)
    EVT_KEY_UP(wxImagePanel::keyReleased)
    EVT_MOUSEWHEEL(wxImagePanel::mouseWheelMoved)
    */
    // catch paint events
    EVT_PAINT(WxImagePanel::paintEvent)
    //Size event
    EVT_SIZE(WxImagePanel::OnSize)
END_EVENT_TABLE()

WxImagePanel::WxImagePanel(wxFrame* parent, wxString file, wxBitmapType format) : wxPanel(parent)
{
    image.LoadFile(file, format);
    w = -1;
    h = -1;
}

void WxImagePanel::loaderImages(wxString file, wxBitmapType format)
{
    image.LoadFile(file, format);
    w = -1;
    h = -1;
    string imageFiles = string ( file.mb_str() );
    delete_imageFiles(imageFiles);
}

bool WxImagePanel::delete_imageFiles(string fileName)
{
    const char* savePath = (char*)fileName.c_str();
    if((access(savePath, F_OK)) != -1)   
    {   
        if(remove(savePath)==0)
        {
            return true;
        }
        else
        {
            cout<<"删除失败"<<endl;
            return false;
        }
    }   
    else  
    {   
        return false;
    }
}

void WxImagePanel::paintEvent(wxPaintEvent & evt)
{
    wxPaintDC dc(this);
    render(dc);
}

void WxImagePanel::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
    wxClientDC dc(this);
    render(dc);
}

void WxImagePanel::render(wxDC&  dc)
{
    int neww, newh;
    dc.GetSize( &neww, &newh );
    if( neww != w || newh != h )
    {
        resized = wxBitmap( image.Scale( neww, newh /*, wxIMAGE_QUALITY_HIGH*/ ) );
        w = neww;
        h = newh;
        dc.DrawBitmap( resized, 0, 0, false );
    } else {
        dc.DrawBitmap( resized, 0, 0, false );
    }
}

void WxImagePanel::OnSize(wxSizeEvent& event){
    Refresh();
    event.Skip();
}

// ----------------------------------------------------------------------------
// MyFrame
// ----------------------------------------------------------------------------
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(LAYOUT_ABOUT, MyFrame::OnAbout)
    EVT_MENU(LAYOUT_QUIT, MyFrame::OnQuit)
    EVT_MENU(LAYOUT_TEST_PROPORTIONS, MyFrame::TestProportions)
    EVT_SIZE(MyFrame::OnSize)
    EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_MYTHREAD_UPDATE, MyFrame::OnThreadUpdate)
	EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_MYTHREAD_COMPLETED, MyFrame::OnThreadCompletion)
wxEND_EVENT_TABLE()

MyFrame::~MyFrame()  
{
    m_pthread->Delete();
}

MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, wxT("衡钢钢管缺陷检测--视觉识别"), wxPoint(20,20), wxSize(470,360))
{
    SetIcon(wxICON(sample));
    wxMenuBar *menu_bar = new wxMenuBar;
    wxMenu *file_menu = new wxMenu;
    int i = 1;
    file_menu->Append(LAYOUT_TEST_PROPORTIONS, wxT("&开始采集数据...\tF1"));
    file_menu->AppendSeparator();
    file_menu->Append(LAYOUT_QUIT, wxT("退&出"), wxT("退出程序"));
    menu_bar->Append(file_menu, wxT("&功能"));
    wxMenu *help_menu = new wxMenu;
    help_menu->Append(LAYOUT_ABOUT, wxT("关于&软件"), wxT("关于软件..."));
    menu_bar->Append(help_menu, wxT("&说明"));
    SetMenuBar(menu_bar); 
#if wxUSE_STATUSBAR
    CreateStatusBar(2);
    SetStatusText("衡钢OnThreadCompletion视觉识别");
#endif 
////头部菜单栏----结束。OnThreadCompletion
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxGridSizer *sizerTop = new wxGridSizer(2, 2, 1, 1);
    wxString bitmap_name = wxString::Format(wxT("/soft/wxWidgets-3.0.4/samples/cameras/image0%i.png"),  i);
    drawPanel0 = new WxImagePanel(this, bitmap_name, wxBITMAP_TYPE_PNG);
    i = 2;
    bitmap_name = wxString::Format(wxT("/soft/wxWidgets-3.0.4/samples/cameras/image0%i.png"),  i);
    drawPanel1 = new WxImagePanel(this, bitmap_name, wxBITMAP_TYPE_PNG);
    i = 3;
    bitmap_name = wxString::Format(wxT("/soft/wxWidgets-3.0.4/samples/cameras/image0%i.png"),  i);
    drawPanel2 = new WxImagePanel(this, bitmap_name, wxBITMAP_TYPE_PNG);
    i = 4;
    bitmap_name = wxString::Format(wxT("/soft/wxWidgets-3.0.4/samples/cameras/image0%i.png"),  i);
    drawPanel3 = new WxImagePanel(this, bitmap_name, wxBITMAP_TYPE_PNG);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    sizerTop->Add(drawPanel0, 0, wxEXPAND);
    sizerTop->Add(drawPanel1, 0, wxEXPAND);
    sizerTop->Add(drawPanel2, 0, wxEXPAND);
    sizerTop->Add(drawPanel3, 0, wxEXPAND);
    sizer->Add(sizerTop, 1, wxEXPAND);
    SetSizer(sizer);
    SetMinSize(wxSize(470,360));
    Centre();
    CreateThread();
}

void MyFrame::CreateThread()
{
    m_pthread = new MyThread(this);
    if ( m_pthread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError(wxT("Can't create thread!"));
        delete m_pthread;
        m_pthread = NULL;
    }
    SetIsCapture(true);
    m_pthread->Run();
    m_pthread->Pause();
}

void MyFrame::OnSize(wxSizeEvent &event)
{
    event.Skip();
    wxSize currentRect = this->GetClientSize();
    cout << "the frame is resize(), w:" << currentRect.x <<  "h:" << currentRect.y << endl;
}

void MyFrame::OnThreadUpdate(wxCommandEvent& evt) 
{
	wxThreadEvent* tEvt = (wxThreadEvent*)&evt;
    wxString imagePath = tEvt->GetString();
    int camerasIndex = tEvt->GetInt();
    if (camerasIndex == 0) 
    {
        drawPanel0->loaderImages(imagePath, wxBITMAP_TYPE_PNG);
    }
    else if (camerasIndex == 1) 
    {
        drawPanel1->loaderImages(imagePath, wxBITMAP_TYPE_PNG);
    }
    else if (camerasIndex == 2) 
    {
        drawPanel2->loaderImages(imagePath, wxBITMAP_TYPE_PNG);
    }
    else if (camerasIndex == 3)
    {
        drawPanel3->loaderImages(imagePath, wxBITMAP_TYPE_PNG);
    }
    Refresh();
    wxString msg = wxString::Format(wxT("获取的进度的内容：%x, 发来消息：%s\r\n"), tEvt->GetInt(), tEvt->GetString());
	cout << "发来消息：" << msg << endl;
}

void MyFrame::ConnectionDB() 
{
    /* MYSQL mysql;
    mysql_init( &mysql );
    mysql_real_connect( &mysql,
        "192.168.16.114",   //要访问数据库的IP地址
        "root",         //用户名
        "root",         //密码
        "test",         //要访问的数据
        3306,           //该数据库的端口
        NULL,           //一般为NULL
        0           //一般为0
    );
    //插入
    string sql = "insert into student value(1, 'jp', 24, 'gzjd')";
    mysql_query( &mysql, sql.c_str() );
    mysql_close( &mysql ); */
}

void MyFrame::TestProportions(wxCommandEvent& WXUNUSED(event))
{
        if (m_pthread)
        {
            if (m_pthread->IsRunning()) 
            {
                SetIsCapture(true);
                m_pthread->Pause();
            } 
            else if (m_pthread->IsPaused()) 
            {
                SetIsCapture(false);
                m_pthread->Resume();
            }
        }
}

void MyFrame::OnThreadCompletion(wxCommandEvent& evt) {
	cout << "the thread is complete!" << endl;
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
    (void)wxMessageBox(wxT("衡钢视觉识别项目-钢管缺陷检测项目\n"),
            wxT("湖南衡阳钢铁厂"), wxOK|wxICON_INFORMATION);
}
