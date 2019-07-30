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
#if !wxUSE_THREADS
    #error "This sample requires thread support!"
#endif 
#include "wx/thread.h"
#include "wx/dynarray.h"
#include "wx/image.h"
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

void  ShowImage( IImage& image, const char* message = NULL)
{
#ifdef PYLON_WIN_BUILD
    Pylon::DisplayImage(1, image);
#endif
    if ( message)
    {
        cout << endl << message << " ";
    }
    ios state(NULL);
    state.copyfmt(cout);
    const uint8_t* pBytes = reinterpret_cast<const uint8_t*>(image.GetBuffer());
    cout << endl << "First six bytes of the image: " << endl;
    for (unsigned int i = 0; i < 6; ++i)
    {
        cout << "0x" << hex << setfill('0') << setw(2) << unsigned(pBytes[i]) << " ";
    }
    cout << endl;
    cout.copyfmt(state);
    cerr << "Press Enter to continue." << endl;
    while( cin.get() != '\n');
}

// ----------------------------------------------------------------------------
// MyCanvas n                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
// ----------------------------------------------------------------------------
IMPLEMENT_CLASS(MyCanvas, wxScrolledWindow)
wxBEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
    EVT_PAINT(MyCanvas::OnPaint)
wxEND_EVENT_TABLE()

MyCanvas::MyCanvas( wxWindow *parent, MyFrame* frame, wxWindowID id, const wxPoint &pos, const wxSize &size )
        : wxScrolledWindow( parent, id,  pos, size, wxSUNKEN_BORDER )
{
    m_nCurrentProgress = 0;
    m_owner = frame;
    wxMemoryDC dc;
    dc.SelectObject( wxNullBitmap );
    ClearShapes();
}

MyCanvas::~MyCanvas()
{
    ClearShapes();
}

void MyCanvas::ClearShapes()
{
    wxList::compatibility_iterator node = m_displayList.GetFirst();
    cout << "the total image face number is : " << node << endl;
    while (node)
    {
        DrawShape* shape = (DrawShape*) node->GetData();
        delete shape;
        node = node->GetNext();
    }
    m_displayList.Clear();
}

void MyCanvas::DrawShapes(wxDC& dc)
{
    wxList::compatibility_iterator node = m_displayList.GetFirst();
    while (node)
    {
        DrawShape* shape = (DrawShape*) node->GetData();
        shape->Draw(dc);
        node = node->GetNext();
    }
}

void MyCanvas::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    cout << "MyCanvas : " << this  << endl;
    // wxPaintDC dc( this );
    wxBufferedPaintDC dc(this);
    PrepareDC( dc );
    DrawShapes(dc);
}

// ----------------------------------------------------------------------------
// DrawShape n                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
// ----------------------------------------------------------------------------
DrawShape::DrawShape(const wxBitmap& bitmap)
{
    m_bitmap = bitmap;
    m_pos.x = 0;
    m_pos.y = 0;
}

bool DrawShape::Draw(wxDC& dc)
{
    cout << "DrawShape is Complete." << endl;
    if (m_bitmap.IsOk())
    {
        wxMemoryDC memDC;
        memDC.SelectObject(m_bitmap);
	    double scaleX = ((double) imagesW)/((double) m_bitmap.GetWidth());
	    double scaleY = ((double) imagesH)/((double) m_bitmap.GetHeight());
        if (scaleX < 1.0 || scaleY < 1.0)
        {
            double scale = wxMin(scaleX, scaleY);
            int newWidth = (int) (scale * m_bitmap.GetWidth());
            int newHeight = (int) (scale * m_bitmap.GetHeight());
            wxImage temp_image = m_bitmap.ConvertToImage();
            temp_image.Rescale(newWidth, newHeight);
            wxBitmap temp_bitmap(temp_image);
            dc.DrawBitmap(temp_bitmap, (imagesW-newWidth) * 0.5, (imagesH-newHeight) * 0.5, true);
        }
        else
        {
            dc.DrawBitmap(m_bitmap, (imagesW-m_bitmap.GetWidth()) * 0.5, (imagesH-m_bitmap.GetHeight()) * 0.5, true);
        }
        return true;
    }
    return false;
}
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
    frame->SetFrame(frame);
    frame->SetIsCapture(false);
    DrawHandle();
    frame->initCameras();
    frame->Show(true); 
    return true;
}

void MyApp::DrawHandle() 
{
    wxString bitmap_name;
    wxImage image;
    cout <<  "00000 ----> MyFrame : " << GetFrame() << endl;
    int i = 1;
    bitmap_name = wxString::Format(wxT("/soft/wxWidgets-3.0.4/samples/cameras/image0%i.png"),  i);
    if (image.LoadFile(bitmap_name, wxBITMAP_TYPE_PNG))
    {
        DrawShape* newShape = new DrawShape(wxBitmap(image));
        GetFrame()->GetCanvas0()->GetDisplayList().Append(newShape);
    }
    i = 2;
    bitmap_name = wxString::Format(wxT("/soft/wxWidgets-3.0.4/samples/cameras/image0%i.png"),  i);
    if (image.LoadFile(bitmap_name, wxBITMAP_TYPE_PNG))
    {
        DrawShape* newShape = new DrawShape(wxBitmap(image));
        GetFrame()->GetCanvas1()->GetDisplayList().Append(newShape);
    }
    i = 3;
    bitmap_name = wxString::Format(wxT("/soft/wxWidgets-3.0.4/samples/cameras/image0%i.png"),  i);
    if (image.LoadFile(bitmap_name, wxBITMAP_TYPE_PNG))
    {
        DrawShape* newShape = new DrawShape(wxBitmap(image));
        GetFrame()->GetCanvas2()->GetDisplayList().Append(newShape);
    }
    i = 4;
    bitmap_name = wxString::Format(wxT("/soft/wxWidgets-3.0.4/samples/cameras/image0%i.png"),  i);
    if (image.LoadFile(bitmap_name, wxBITMAP_TYPE_PNG))
    {
        DrawShape* newShape = new DrawShape(wxBitmap(image));
        GetFrame()->GetCanvas3()->GetDisplayList().Append(newShape);
    }
}

// ----------------------------------------------------------------------------
// CSampleImageEventHandler
// ----------------------------------------------------------------------------
// A user-provided buffer factory.
class CSampleImageEventHandler : public CImageEventHandler
{
public:
    virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
    {
        CPylonImage targetImage;
        MyFrame* frame = new MyFrame();
        intptr_t cameraContextValue = ptrGrabResult->GetCameraContext();
        cout << "Camera " << cameraContextValue << ": " << camera.GetDeviceInfo().GetModelName() << endl;
        uint16_t frameNumber = (uint16_t)ptrGrabResult->GetBlockID();
        cout << "frameNumber : " << frameNumber << endl;
        cout << "GetErrorCode : " << ptrGrabResult->GetErrorCode() << endl;
        cout << "GetErrorDescription : " << ptrGrabResult->GetErrorDescription() << endl;
        cout << "imageGetNumberOfSkippedImages : " << ptrGrabResult->GetNumberOfSkippedImages() << endl;
        cout << "CSampleImageEventHandler::OnImageGrabbed called." << endl;
        cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
        cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
        CImageFormatConverter Format_converter;
        Format_converter.OutputPixelFormat = PixelType_BGR8packed;
        Format_converter.Convert(targetImage, ptrGrabResult);
        string filesName = frame->GetImagesFileName(cameraContextValue);
        Mat image(targetImage.GetWidth(), targetImage.GetHeight(), CV_8UC1, targetImage.GetBuffer(), Mat::AUTO_STEP); 
        imwrite(filesName, image);
        frame->ImageFaceDatas(filesName, cameraContextValue);
        cout << endl;
        cout << endl;
    }
};

// ----------------------------------------------------------------------------
// MyBufferFactory
// ----------------------------------------------------------------------------
// A user-provided buffer factory.
class MyBufferFactory : public IBufferFactory
{
public:
    MyBufferFactory()
        : m_lastBufferContext(5000)
    {
    }

    virtual ~MyBufferFactory()
    {
    }

    // Will be called when the Instant Camera object needs to allocate a buffer.
    // Return the buffer and context data in the output parameters.
    // In case of an error new() will throw an exception
    // which will be forwarded to the caller to indicate an error.
    // Warning: This method can be called by different threads.
    virtual void AllocateBuffer( size_t bufferSize, void** pCreatedBuffer, intptr_t& bufferContext)
    {
        try
        {
            // Allocate buffer for pixel data.
            // If you already have a buffer allocated by your image processing library you can use this instead.
            // In this case you must modify the delete code (see below) accordingly.
            *pCreatedBuffer = new uint8_t[bufferSize];
            // The context information is never changed by the Instant Camera and can be used
            // by the buffer factory to manage the buffers.
            // The context information can be retrieved from a grab result by calling
            // ptrGrabResult->GetBufferContext();
            bufferContext = ++m_lastBufferContext;

            cout << "Created buffer " << bufferContext << ", " << *pCreatedBuffer << endl;
        }
        catch (const std::exception&)
        {
            // In case of an error we must free the memory we may have already allocated.
            if (*pCreatedBuffer != NULL)
            {
                uint8_t* p = reinterpret_cast<uint8_t*>(pCreatedBuffer);
                delete[] p;
                *pCreatedBuffer = NULL;
            }
            // Rethrow exception.
            // AllocateBuffer can also just return with *pCreatedBuffer = NULL to indicate
            // that no buffer is available at the moment.
            throw;
        }
    }

    // Frees a previously allocated buffer.
    // Warning: This method can be called by different threads.
    virtual void FreeBuffer( void* pCreatedBuffer, intptr_t bufferContext)
    {
        uint8_t* p = reinterpret_cast<uint8_t*>(pCreatedBuffer);
        delete [] p;
        cout << "Freed buffer " << bufferContext << ", " << pCreatedBuffer << endl;
    }

    // Destroys the buffer factory.
    // This will be used when you pass the ownership of the buffer factory instance to pylon
    // by defining Cleanup_Delete. pylon will call this function to destroy the instance
    // of the buffer factory. If you don't pass the ownership to pylon (Cleanup_None)
    // this method will be ignored.
    virtual void DestroyBufferFactory()
    {
        delete this;
    }

protected:
    unsigned long m_lastBufferContext;
};

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
    string filesName;
     try
    {
		MyBufferFactory myFactory;
		CTlFactory& tlFactory = CTlFactory::GetInstance();
		DeviceInfoList_t devices;
        Mat openCvImage;
        CGrabResultPtr ptrGrabResult;
        CImageFormatConverter Format_converter;
        CPylonImage targetImage;
        wxImage images;
        string filesName;
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
            cameras.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);
            intptr_t cameraContextValue = ptrGrabResult->GetCameraContext();
            cout << "Camera " <<  cameraContextValue << ": " << cameras[ cameraContextValue ].GetDeviceInfo().GetModelName() << endl;
            cout << "GrabSucceeded: " << ptrGrabResult->GrabSucceeded() << endl;
            cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
            cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
            const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
            cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;    
            Format_converter.Convert(targetImage, ptrGrabResult);
            openCvImage =  Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) targetImage.GetBuffer());
            MatToWxImage(openCvImage, images);
            displaySynchronous(wxEVT_COMMAND_MYTHREAD_UPDATE, images, cameraContextValue);
            /*
            wxBitmap bitmap(images);
            DrawShape* newShape = new DrawShape(wxBitmap(image));
            if (cameraContextValue == 0) 
            {
                gui->GetCanvas0()->GetDisplayList().Append(newShape);
            }
            if (cameraContextValue == 1) 
            {
                gui->GetCanvas1()->GetDisplayList().Append(newShape);
            }
            if (cameraContextValue == 2) 
            {
                gui->GetCanvas2()->GetDisplayList().Append(newShape);
            }
            if (cameraContextValue == 3) 
            {
                gui->GetCanvas3()->GetDisplayList().Append(newShape);
            }
            gui->Refresh(true);
            gui->Update();
             */
            //////////////////////////////////////////////////////////////////////
            filesName = this->make_filename(cameraContextValue);
            //imwrite(filesName, openCvImage);
            cout << "the image file name is : " << filesName << endl;    
            WaitObject::Sleep(150);
        } while (cameras.IsGrabbing());
        cameras.StopGrabbing();
        cameras.Close();
        PylonTerminate();
    }
    catch (const GenericException &e)
    {
        (void)wxMessageBox(e.GetDescription(), wxT("视频设备-错误信息"), wxOK|wxICON_INFORMATION);
        PylonTerminate();
    }
}

void MyThread::displaySynchronous(const wxEventType & evtType, const wxBitmap bitmap, intptr_t cameras_index) 
{
    wxThreadEvent * threadEvt = new wxThreadEvent(evtType);
	THREAD_MSG_TYPE * threadMsg = new THREAD_MSG_TYPE{ wxThread::GetCurrentId(), cameras_index, bitmap };
	threadEvt->SetExtraLong(reinterpret_cast<long>(threadMsg));
	wxQueueEvent(gui, threadEvt);
}

void MyThread::MatToWxImage(Mat &mat, wxImage  &image)
{
    // data dimension
    int w = mat.cols, h = mat.rows;//获取MAT宽度，高度
    int size = w*h*3*sizeof(unsigned char);//为WxImage分配空间数，因为其为24bit，所以*3
    // allocate memory for internal wxImage data
    unsigned char * wxData = (unsigned char*) malloc(size);//分配空间
    // the matrix stores BGR image for conversion
    Mat cvRGBImg = Mat(h, w, CV_8UC3, wxData);//转换存储用MAT
    switch (mat.channels())//根据MAT通道数进行转换
    {
        case 1: // 1-channel case: expand and copy灰度到RGB
        {
            // convert type if source is not an integer matrix
            if (mat.depth() != CV_8U)//不是unsigned char先转化
            {
                cvtColor(convertType(mat, CV_8U, 255,0), cvRGBImg, CV_8U);//把图片从一个颜色空间转换为另一个
            }
            else
            {
                cvtColor(mat, cvRGBImg, CV_8U);
            }
        } 
        break;
        case 3: // 3-channel case: swap R&B channels
        {
            int mapping[] = {0,2,1,1,2,0}; // CV(BGR) to WX(RGB)
            // bgra[0] -> bgr[2], bgra[1] -> bgr[1], bgra[2] -> bgr[0]，即转成RGB，舍弃alpha通道
            mixChannels(&mat, 1, &cvRGBImg, 1, mapping, 3);//一个输入矩阵，一个输出矩阵，maping中三个索引对
        } 
        break;
        default://通道数量不对
        {
            wxLogError(wxT("Cv2WxImage : input image (#channel=%d) should be either 1- or 3-channel"), mat.channels());
        }
  }
  image.Destroy(); // free existing data if there's any
  image = wxImage(w, h, wxData);
}

Mat MyThread::convertType(const Mat& srcImg, int toType, double alpha, double beta)
{
    Mat dstImg;
    srcImg.convertTo(dstImg, toType, alpha, beta);
    return dstImg;
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
// MyFrame
// ----------------------------------------------------------------------------
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(LAYOUT_ABOUT, MyFrame::OnAbout)
    EVT_MENU(LAYOUT_QUIT, MyFrame::OnQuit)
    EVT_MENU(LAYOUT_TEST_PROPORTIONS, MyFrame::TestProportions)
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
    wxFlexGridSizer *sizerFlex;
    wxPanel* p = new wxPanel(this, wxID_ANY);
    wxSizer *sizerCol1 = new wxBoxSizer(wxVERTICAL);
    sizerFlex = new wxFlexGridSizer(2, 2, wxSize(5, 5));
    InitFlexSizer(sizerFlex, p);
    sizerFlex->AddGrowableCol(0, 1);
    sizerFlex->AddGrowableCol(1, 2);
    sizerCol1->Add(sizerFlex, 1, wxALL | wxEXPAND, 10);
    wxGridSizer *sizerTop = new wxGridSizer(1, 0,  20);
    sizerTop->Add(sizerCol1, 1, wxEXPAND);
    p->SetSizer(sizerTop);
    sizerTop->SetSizeHints(this);
}

void MyFrame::initShowCameras() 
{
    bool iscapture = GetIsCapture();
    if (!iscapture) { 
        return ;
    }
    PylonInitialize();
    string filesName;
     try
    {
		MyBufferFactory myFactory;
		CTlFactory& tlFactory = CTlFactory::GetInstance();
		DeviceInfoList_t devices;
		if (tlFactory.EnumerateDevices(devices) == 0)
		{
			throw RUNTIME_EXCEPTION("No camera present.");
			PylonTerminate();
			return;
		}
        CInstantCameraArray cameras(min(devices.size(), c_maxCamerasToUse));
		 for (size_t i = 0; i < cameras.GetSize(); ++i)
		{
            cameras[i].Attach( tlFactory.CreateDevice( devices[i]));
            cameras[i].RegisterConfiguration( new CSoftwareTriggerConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);
            cameras[i].RegisterConfiguration( new CConfigurationEventPrinter, RegistrationMode_Append, Cleanup_Delete);
            cameras[i].RegisterImageEventHandler( new CImageEventPrinter, RegistrationMode_Append, Cleanup_Delete);
            cameras[i].RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete);
            cameras[i].SetBufferFactory(&myFactory, Cleanup_None);
            cameras[i].MaxNumBuffer = 5;
            cameras[i].Open();
        }
        if (cameras[0].CanWaitForFrameTriggerReady() && cameras[1].CanWaitForFrameTriggerReady())
        {
            cameras[0].StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
            cameras[1].StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
            do
            {
                if ( cameras[0].WaitForFrameTriggerReady(500, TimeoutHandling_ThrowException))
                {
                    cameras[0].ExecuteSoftwareTrigger();
                }
                if ( cameras[1].WaitForFrameTriggerReady(500, TimeoutHandling_ThrowException))
                {
                    cameras[1].ExecuteSoftwareTrigger();
                }
                WaitObject::Sleep(200);
            } while ( true);     
        }
        cameras.StopGrabbing();
        cameras.Close();
    }
    catch (const GenericException &e)
    {
        (void)wxMessageBox(e.GetDescription(), wxT("视频设备-错误信息"), wxOK|wxICON_INFORMATION);
    }
    PylonTerminate(); 
    initCameras();
}

void MyFrame::initCameras()
{
    bool iscapture = GetIsCapture();
    if (!iscapture) { 
        return ;
    }
    PylonInitialize();
    string filesName;
     try
    {
		MyBufferFactory myFactory;
		CTlFactory& tlFactory = CTlFactory::GetInstance();
		DeviceInfoList_t devices;
        Mat openCvImage;
        CGrabResultPtr ptrGrabResult;
        CImageFormatConverter Format_converter;
        CPylonImage targetImage;
         string filesName;
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
            cameras.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);
            intptr_t cameraContextValue = ptrGrabResult->GetCameraContext();
            cout << "Camera " <<  cameraContextValue << ": " << cameras[ cameraContextValue ].GetDeviceInfo().GetModelName() << endl;
            cout << "GrabSucceeded: " << ptrGrabResult->GrabSucceeded() << endl;
            cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
            cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
            const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
            cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;    
            Format_converter.Convert(targetImage, ptrGrabResult);
            filesName = frame->GetImagesFileName(cameraContextValue);
            openCvImage =  Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) targetImage.GetBuffer());
            imwrite(filesName, openCvImage);
            cout << "the image file name is : " << filesName << endl;    
            WaitObject::Sleep(150);
            wxImage image(filesName);
            wxBitmap bitmap(image);  
            // GetCanvas0()->SetBitmapLabel(bitmap);
            WaitObject::Sleep(200);
        } while (cameras.IsGrabbing());
        cameras.StopGrabbing();
        cameras.Close();
        PylonTerminate();
    }
    catch (const GenericException &e)
    {
        (void)wxMessageBox(e.GetDescription(), wxT("视频设备-错误信息"), wxOK|wxICON_INFORMATION);
        PylonTerminate();
    }
}

void MyFrame::ImageFaceDatas( string files, intptr_t order)
{
    wxString bitmap_name = wxString::FromUTF8(files.c_str());
    wxImage image;
    cout <<  "MyFrame : " << wxGetApp().GetFrame() << endl;
    if (image.LoadFile(bitmap_name, wxBITMAP_TYPE_PNG))
    {
        DrawShape* newShape = new DrawShape(wxBitmap(image));
        cout << "GetCanvas --- ----- -----> 0  : " << GetCanvas0() << endl;
        if (order == 0) {
            wxGetApp().GetFrame()->GetCanvas0()->GetDisplayList().Append(newShape);
        } else if (order == 1) {
            wxGetApp().GetFrame()->GetCanvas1()->GetDisplayList().Append(newShape);
        } else if (order == 2) {
            wxGetApp().GetFrame()->GetCanvas2()->GetDisplayList().Append(newShape);
        } else if (order == 3) {
            wxGetApp().GetFrame()->GetCanvas3()->GetDisplayList().Append(newShape);
        }
    }
}

string MyFrame::GetImagesFileName(intptr_t Orders) 
{
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    unsigned long micro = curTime.tv_sec * (uint64_t)1000000 + curTime.tv_usec;
    stringstream OrderStr;
    stringstream microTimer;
    stringstream ss; 
    string FileName;
    OrderStr << Orders;
    microTimer << micro;
    ss << "/soft/wxWidgets-3.0.4/gtk-build/samples/cameras/cachedatas/GrabbedImage_" << OrderStr.str() << "_" << microTimer.str() << ".png";
    ss >> FileName;
    return FileName;
}

void MyFrame::InitFlexSizer(wxFlexGridSizer *sizer, wxWindow* parent)
{
    m_canvas_0 = new MyCanvas(parent, this, wxID_ANY,  wxPoint(0,0), wxSize(imagesW, imagesH));
    m_canvas_1 = new MyCanvas(parent, this, wxID_ANY,  wxPoint(0,0), wxSize(imagesW, imagesH));
    m_canvas_2 = new MyCanvas(parent, this,  wxID_ANY,  wxPoint(0,0), wxSize(imagesW, imagesH));
    m_canvas_3 = new MyCanvas(parent, this,  wxID_ANY,  wxPoint(0,0), wxSize(imagesW, imagesH));
    sizer->Add(m_canvas_0, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    sizer->Add(m_canvas_1, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    sizer->Add(m_canvas_2, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    sizer->Add(m_canvas_3, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    m_pthread = new MyThread(this);
    if ( m_pthread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError(wxT("Can't create thread!"));
        delete m_pthread;
        m_pthread = NULL;
    }
}

void MyFrame::OnThreadUpdate(wxCommandEvent& evt) 
{
	wxThreadEvent* tEvt = (wxThreadEvent*)&evt;
	THREAD_MSG_TYPE * threadMsg = (THREAD_MSG_TYPE *)tEvt->GetExtraLong();
	wxString msg = wxString::Format(wxT("线程Id:%x，发来消息：%x\r\n"), threadMsg->threadId, threadMsg->cameras_index);
	cout << msg << endl;
    delete threadMsg;
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
        SetIsCapture(true);
        if (m_pthread)
        {
            m_pthread->Run();
        }
}

void MyFrame::OnThreadCompletion(wxCommandEvent& evt) {
	wxThreadEvent* tEvt = (wxThreadEvent*)&evt;
	THREAD_MSG_TYPE * threadMsg = (THREAD_MSG_TYPE *)tEvt->GetExtraLong();
	wxString msg = wxString::Format(wxT("线程Id:%x，发来消息：%s\r\n"), threadMsg->threadId, threadMsg->cameras_index);
	wxPrintf(msg, msg.Len());
	delete threadMsg;
	m_pthread = nullptr;
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
