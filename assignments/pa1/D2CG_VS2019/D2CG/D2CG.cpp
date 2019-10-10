//////////////////////////////////////////////////////////////////////
// D2CG.cpp

#include "stdafx.h"

#include "D2CG.h"

#include "Code.h"


#ifdef _DEBUG //for debug builds only
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;



// declare a D2CG application object, everything starts here.....
D2CGApp d2cg;

// global aboutdialog to prevent 2 such windows created at the same time
AboutDialog *aboutdialog;


//the number of pixels corresponding to a unit length in world coordinate
const double pixelRatio=300;


//////////////////////////////////////////////////////////////////////
// application initialization, the constructor is defined in
// the Fractal.h file
BOOL D2CGApp::InitInstance()
{
	CFrameWnd *MFrame=new D2CGFrameWindow(WINDOW_Triangles,500,500);
	m_pMainWnd=MFrame;
	MFrame->SetIcon (LoadIcon (IDI_MAIN),TRUE);

	CFrameWnd *JFrame=new D2CGFrameWindow(WINDOW_FRACTALS,500,500);
	JFrame->SetIcon (LoadIcon (IDI_MAIN),TRUE);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// macro that matchs events to funcitons
BEGIN_MESSAGE_MAP(D2CGFrameWindow, CFrameWnd)
	//{{AFX_MSG_MAP(D2CGFrameWindow)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_EDIT_UNDO40017, &D2CGFrameWindow::OnEditUndo40017)
	ON_COMMAND(ID_EDIT_FRACTALIZE, &D2CGFrameWindow::OnEditFractalize)
	ON_COMMAND(ID_FILE_OPEN40019, &D2CGFrameWindow::OnFileOpen40019)
	ON_COMMAND(ID_FILE_SAVE40020, &D2CGFrameWindow::OnFileSave40020)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// fractal window constructor, data initialization
D2CGFrameWindow::D2CGFrameWindow(int type,int x,int y)
{

	windowType=type;
	wx=x;
	wy=y;

	aboutdialog=NULL;

	if (!Create(NULL,type==WINDOW_Triangles?"Triangles":"Fractal",WS_VISIBLE|WS_OVERLAPPEDWINDOW))
		return;
}

//////////////////////////////////////////////////////////////////////
// close the other window when one is destroyed
D2CGFrameWindow::~D2CGFrameWindow()
{
}

//////////////////////////////////////////////////////////////////////
// initialization of the windows settings required
int D2CGFrameWindow::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CS_OWNDC);

	// set window styles required for OpenGL before window is created
	cs.cx=wx;
	cs.cy=wy;
	if(windowType==WINDOW_FRACTALS)
	{	cs.x=500;
		cs.y=100;
	}
	else
	{
		cs.x=10;
		cs.y=100;
	}

	return CFrameWnd::PreCreateWindow(cs);
}


//////////////////////////////////////////////////////////////////////
// create event, initialization
int D2CGFrameWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct))
		return -1;
	
	/////// setup OpenGL
	int nPixelFormat;					// pixel format index
	m_hDC = ::GetDC(m_hWnd);			// get the device context

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this structure
		1,								// version of this structure	
		PFD_DRAW_TO_WINDOW |			// draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL,				// support OpenGL calls in window
		PFD_TYPE_RGBA,					// RGBA Color mode
		24,								// want 24bit color 
		0,0,0,0,0,0,					// not used to select mode
		0,0,							// not used to select mode
		0,0,0,0,0,						// not used to select mode
		0,								// size of depth buffer, here is 0, not used
		0,								// size of stencil buffer
		0,								// not used to select mode
		PFD_MAIN_PLANE,					// draw in main plane
		0,								// not used to select mode
		0,0,0 };						// not used to select mode

	// choose a pixel format that best matches that described in pfd
	nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);

	// set the pixel format for the device context
	VERIFY(SetPixelFormat(m_hDC, nPixelFormat, &pfd));

	// create the rendering context
	m_hRC = wglCreateContext(m_hDC);

	// make the rendering context current, perform initialization, then deselect it
	VERIFY(wglMakeCurrent(m_hDC,m_hRC));

	// setup OpenGL fonts
	HFONT hFont;
	LOGFONT logfont;

	logfont.lfHeight = 20;
	logfont.lfWidth = 0;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = FW_BOLD;
	logfont.lfItalic = FALSE;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = NONANTIALIASED_QUALITY;
	logfont.lfPitchAndFamily = DEFAULT_PITCH;
	strcpy_s(logfont.lfFaceName, 32, "Arial");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	hFont = CreateFontIndirect(&logfont);
	SelectObject (m_hDC, hFont); 
	wglUseFontBitmaps(m_hDC, 0, 255, 1000); 
	DeleteObject(hFont);


	logfont.lfHeight = 12;
	logfont.lfWidth = 0;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = FW_BOLD;
	logfont.lfItalic = FALSE;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = NONANTIALIASED_QUALITY;
	logfont.lfPitchAndFamily = DEFAULT_PITCH;
	strcpy_s(logfont.lfFaceName, 32, "Arial");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	hFont = CreateFontIndirect(&logfont);
	SelectObject (m_hDC, hFont); 
	wglUseFontBitmaps(m_hDC, 0, 255, 2000); 
	DeleteObject(hFont);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	wglMakeCurrent(m_hDC,NULL);


	if(windowType==WINDOW_Triangles){
	// add the pull down menu
	CMenu *MainMenu;
	MainMenu= new CMenu;
	MainMenu->LoadMenu(IDR_MENU2);
	SetMenu(MainMenu);
	MainMenu->Detach();
	delete MainMenu;

	}
	return 0;
}



//////////////////////////////////////////////////////////////////////
// resize event, call GLResize() to redefine the viewport
void D2CGFrameWindow::OnSize(UINT nType, int cx, int cy) 
{

	CFrameWnd::OnSize(nType, cx, cy);

	wglMakeCurrent(m_hDC,m_hRC);

	//set the window of viewing
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D((-cx)/2.0/pixelRatio,(cx)/2.0/pixelRatio,(-cy)/2.0/pixelRatio,(cy)/2.0/pixelRatio);

	//set modelview matrix to be I
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//set viewport to fill the whole window
	glViewport(0,0,cx,cy);

	RenderScene();

	wglMakeCurrent(m_hDC,NULL);
}

//////////////////////////////////////////////////////////////////////
// called when window receives WM_PAINT, render scene
bool stop_debug=false;
void D2CGFrameWindow::OnPaint()
{

	PAINTSTRUCT ps;
	RECT rc;

	CPaintDC dc(this);


	// Make the rendering context current
	VERIFY(wglMakeCurrent(m_hDC,m_hRC));

	// Call our external OpenGL code
	RenderScene();

	// Allow other rendering contexts to co-exist
	VERIFY(wglMakeCurrent(m_hDC,NULL));

}

//////////////////////////////////////////////////////////////////////
// override to keep the background from being erased everytime
// the window is repainted
BOOL D2CGFrameWindow::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}


void D2CGFrameWindow::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	if (windowType==WINDOW_Triangles){
		D2CGFrameWindow *wind=(D2CGFrameWindow*)FindWindow(NULL,"Fractal");
		wind->CloseWindow();
		wind->DestroyWindow();
		
	}

	if (windowType==WINDOW_FRACTALS){
		D2CGFrameWindow *wind=(D2CGFrameWindow*)FindWindow(NULL,"Triangles");
		wind->CloseWindow();
		wind->DestroyWindow();
		
	}
	CFrameWnd::OnClose();
}


// Render the window with respective method
void D2CGFrameWindow::RenderScene(void)
{

	glClear(GL_COLOR_BUFFER_BIT);	//clear color buffer


	if(windowType==WINDOW_FRACTALS){
		ConstructiveFractals();
	}
	else
	{
		for(int i=0;i<triangles.size();i++)
		{
			triangles[i].color[0]=color_array[i%11][0];
			triangles[i].color[1]=color_array[i%11][1];
			triangles[i].color[2]=color_array[i%11][2];
		}
		DrawTriangles();
	}
	

	glFlush();
}

void D2CGFrameWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if(windowType==WINDOW_Triangles){
	RECT rect;
	this->GetClientRect(&rect);
	GLdouble xv = (point.x-(rect.right-rect.left)/2)/pixelRatio;
	GLdouble yv = -(point.y-(rect.bottom-rect.top)/2)/pixelRatio;

	MouseInteraction(xv,yv);

	if(point_count==0)
	{
		D2CGFrameWindow *wind=(D2CGFrameWindow*)FindWindow(NULL,"Fractal");
		wind->RedrawWindow();
	}

	}

	CFrameWnd::OnLButtonUp(nFlags, point);
	this->RedrawWindow();
}


void D2CGFrameWindow::OnEditUndo40017()
{
	// TODO: Add your command handler code here
	if(point_count>0)
	{
		point_count--;
	}
	else
	{if(triangles.size()>0)
		{
			triangles.pop_back();
		}
		D2CGFrameWindow *wind=(D2CGFrameWindow*)FindWindow(NULL,"Fractal");
		wind->RedrawWindow();
	}
	this->RedrawWindow();
}


void D2CGFrameWindow::OnEditFractalize()
{

	//specify the depth of recursion
	DepthDlg *depthDialog;
	depthDialog=new DepthDlg(this);

	depthDialog->m_intDepth = recursion_depth;

	

	if(depthDialog->DoModal()==IDOK)
	{
	recursion_depth=depthDialog->m_intDepth;

	}
	delete depthDialog;

	
	//IFS
	D2CGFrameWindow *wind=(D2CGFrameWindow*)FindWindow(NULL,"Fractal");
	wind->RedrawWindow();

}

void D2CGFrameWindow::OnFileOpen40019()
{
	
	CFileDialog *fdlg;
	char c[100];
	int triangleNumber;
	int i,j;
	fdlg=new CFileDialog(TRUE,".fra",NULL,OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,"Fractal (*.fra)|*.fra||",this);

	if (fdlg->DoModal()==IDOK){
		if (fdlg->GetPathName().IsEmpty()==TRUE){
			MessageBox("Empty string.","Information",MB_OK | MB_ICONINFORMATION);
			return;
		}
		triangles.clear();
		ifstream in;
		in.open(fdlg->GetPathName(),ios_base::in);
		
		//in.getline(c,100);
		in >> triangleNumber;


		for (i=0;i<triangleNumber;i++){

			Triangle newTriangle;
	//		in.getline(c,100);
			for(j=0;j<3;j++){
				in >>newTriangle.vertices[j][0]>>newTriangle.vertices[j][1];
			}

			//compute the affine matrix for this transformation
			if(triangles.size()>0)
			{
				AffineMatricesCalculation(triangles[0].vertices,newTriangle.vertices,newTriangle.matrix);
			}

			triangles.push_back(newTriangle);			
		}
		
		in.close();
	}
	delete fdlg;
	this->RedrawWindow();
	
	D2CGFrameWindow *wind=(D2CGFrameWindow*)FindWindow(NULL,"Fractal");
	wind->RedrawWindow();
}


void D2CGFrameWindow::OnFileSave40020()
{
	CFileDialog *fdlg;
	int i,j;

	fdlg=new CFileDialog(FALSE,".fra",NULL,OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,"Fractal (*.fra)|*.fra||",this);

	if (fdlg->DoModal()==IDOK){
		if (fdlg->GetPathName().IsEmpty()==TRUE){
			MessageBox("Empty string.","Information",MB_OK | MB_ICONINFORMATION);
			return;
		}
		//ofstream out=ofstream(fdlg->GetPathName());
		ofstream out;
		out.open(fdlg->GetPathName(),ios_base::out);
		out.precision(30);
		//out << setprecision(30);
		int triangleNumber=triangles.size();
		out << triangleNumber << endl;
		for (i=0;i<triangleNumber;i++){
			for(j=0;j<3;j++){
				out << triangles[i].vertices[j][0] << " ";
				out << triangles[i].vertices[j][1] << " ";			
			}
			out <<endl;
		}		
		out.close();

		MessageBox("Save successfully complete.");

	}
	delete fdlg;
}

// DepthDlg dialog

IMPLEMENT_DYNAMIC(DepthDlg, CDialog)

DepthDlg::DepthDlg(CWnd* pParent /*=NULL*/)
	: CDialog(DepthDlg::IDD, pParent)
	, m_intDepth(0)
{

}

DepthDlg::~DepthDlg()
{
}

void DepthDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_intDepth);
	DDV_MinMaxInt(pDX, m_intDepth, 0, 100);
}


BEGIN_MESSAGE_MAP(DepthDlg, CDialog)
END_MESSAGE_MAP()
