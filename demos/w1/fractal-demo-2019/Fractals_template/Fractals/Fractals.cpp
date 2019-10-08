//////////////////////////////////////////////////////////////////////
// Fractals.cpp

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <fstream>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>
#include "resource.h"
#include "Code.h"
#include "Fractals.h"

#ifdef _DEBUG //for debug builds only
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;


// declare a fractals application object, everything starts here.....
FractalsApp Fractals;

//////////////////////////////////////////////////////////////////////
// application initialization, the constructor is defined in
// the Fractal.h file
BOOL FractalsApp::InitInstance()
{
	CFrameWnd *MFrame=new FractalsFrameWindow(SET_MANDELBROT,50,50);
	m_pMainWnd=MFrame;
	MFrame->SetIcon (LoadIcon (IDI_MAIN),TRUE);

	CFrameWnd *JFrame=new FractalsFrameWindow(SET_JULIA,335,50);
	JFrame->SetIcon (LoadIcon (IDI_MAIN),TRUE);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// macro that matchs events to funcitons
BEGIN_MESSAGE_MAP(FractalsFrameWindow, CFrameWnd)
	//{{AFX_MSG_MAP(FractalsFrameWindow)
	ON_COMMAND(ID_OPEN,CmFileOpen)
	ON_COMMAND(ID_SAVE,CmFileSave)
	ON_COMMAND(ID_EXIT,CmExit)
	ON_COMMAND(ID_ABOUT,CmAbout)
	ON_COMMAND(ID_COLORMAP,CmColorMap)
	ON_COMMAND(ID_KEY_COLOR_MAP,CmKeyColorMap)
	ON_COMMAND(ID_INFORMATION,CmInformation)
	ON_COMMAND(ID_ZOOMOUT,CmZoomOut)
	ON_COMMAND(ID_ZOOMTOP,CmZoomTop)
	ON_COMMAND(ID_ASPECT_RATIO,CmAspectRatio)
	ON_COMMAND(ID_REMOVE_KEYCOLOR,CmRemoveKeyColor)
	ON_COMMAND(ID_CLEAR_COLOR,CmClearColor)
	ON_COMMAND(ID_STD_COLOR,CmStdColor)
	ON_COMMAND(ID_IMPORT_COLOR,CmImpColor)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// fractal window constructor, data initialization
FractalsFrameWindow::FractalsFrameWindow(int type,int x,int y)
{
	unsigned int i;
	BUTDOWN=FALSE;
	SHOWINFO=FALSE;
	showcolormap=0;
	wleft=x;
	wtop=y;
	fractalstype=type;
	zoomlevel=0;
	if (type==SET_MANDELBROT){			// mandelbort initial range
		cleft[0]=-2.25;//-2.5;
		cright[0]=0.75;//1.5;
		cbottom[0]=-1.5;//-2.0;
		ctop[0]=1.5;//2.0;
	} else {				// julia initial range
		cleft[0]=-2.0;
		cright[0]=2.0;
		cbottom[0]=-2.0;
		ctop[0]=2.0;
	}
	map=NULL;
	lmap=NULL;

	aboutdialog=NULL;

	for (i=0;i<256;i++){	// initial color map
		colormap[i]=0xFF000000;
		keycolormap[i]=0xFF000000;
		keycolormapmask[i]=0;
	}
	
	keycolormap[7]=0xFF424015;
	keycolormapmask[7]=1;
	keycolormap[71]=0xFFFFFFFF;
	keycolormapmask[71]=1;
	keycolormap[119]=0xFF0000A8;
	keycolormapmask[119]=1;
	keycolormap[167]=0xFF6F001C;
	keycolormapmask[167]=1;
	keycolormap[215]=0xFF79FDF3;
	keycolormapmask[215]=1;

	GenerateColormap();
	
	colormap[0]=0xFF000000;

	selectedcolor=0;
	selectedkeycolor=0;

	if (!Create(NULL,type==SET_MANDELBROT?"Mandelbrot Set":"Julia Set",WS_VISIBLE| WS_OVERLAPPEDWINDOW))
		return;
}

//////////////////////////////////////////////////////////////////////
// close the other window when one is destroyed
FractalsFrameWindow::~FractalsFrameWindow()
{
}

//////////////////////////////////////////////////////////////////////
// initialization of the windows settings required
int FractalsFrameWindow::PreCreateWindow(CREATESTRUCT& cs)
{
	// set window styles required for OpenGL before window is created
	cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CS_OWNDC);
	cs.x=wleft;
	cs.y=wtop;
	cs.cx=282;
	cs.cy=320;
	return CFrameWnd::PreCreateWindow(cs);
}

//////////////////////////////////////////////////////////////////////
// create event, initialization
int FractalsFrameWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

	wglMakeCurrent(NULL,NULL);


	// add the pull down menu
	CMenu *MainMenu;
	MainMenu= new CMenu;
	MainMenu->LoadMenu(IDM_MAINMENU);
	SetMenu(MainMenu);
	MainMenu->Detach();
	delete MainMenu;
	
	return 0;
}


//////////////////////////////////////////////////////////////////////
// Render a XOR rectangle, for rubber banding
void FractalsFrameWindow::XORRentangle(int x1, int y1,int x2, int y2)
{	
	unsigned long white=0xFFFFFFFF;

	// here we use OpenGL blending function to simulate XOR operation
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR,GL_ONE_MINUS_SRC_COLOR);

	glColor4ubv((unsigned char*)&white);
	glBegin(GL_LINES);
		glVertex2i(x1,y1);glVertex2i(x1,y2);
		glVertex2i(x1,y2);glVertex2i(x2,y2);
		glVertex2i(x2,y2);glVertex2i(x2,y1);
		glVertex2i(x2,y1);glVertex2i(x1,y1);
	glEnd();
	glDisable(GL_BLEND);
}

//////////////////////////////////////////////////////////////////
// refresh the windows
void FractalsFrameWindow::RenderScene(BOOL SHOWINFO,					// whether show extra information on screen
				   unsigned char scolor,			// selected color index in color map
				   unsigned char ssourcecolor,		// selected color index in source color map
				   int showcolormap,				// what to display 0-fractals, 1-colormap, 2-source color map
				   int type,						// fractals types 0-mandelbort, 1-julia
				   double left,						// left,right,bottom,top mark the domain in the complex plane
				   double right,					//
				   double bottom,					//
				   double top,						//
				   double ca,						// ca,cb are the coeff. for the complex number c used in julia set
				   double cb,						//
				   int w,							// window width
				   int h,							// window height
				   unsigned char *map,				// colorindex buffer
				   unsigned long *lmap,				// rgb buffer
				   unsigned char **newmap,			// new map pointer return to caller
				   unsigned long **newlmap,			// new rgb buffer return to caller
				   unsigned long *colormap,			// color map
				   unsigned long *keycolormap,		// source color map
				   unsigned char *keycolormapmask)	// source color influence
{
	int i,j;
	float hs,vs,k1,k2,m1,m2;

	CString cstr;

	switch(showcolormap){
	case 2:	//key color map

		glClear(GL_COLOR_BUFFER_BIT);

		hs=(float)w/16.0f;
		vs=(float)h/16.0f;
	
		k1=.0;
		k2=vs;
		glListBase(2000);
		for (i=0;i<16;i++){
			m1=.0;
			m2=hs;
			for(j=0;j<16;j++){
				if (keycolormapmask[i*16+j]>0){
					glColor4ubv((GLubyte *)&keycolormap[i*16+j]);
					glBegin(GL_QUADS);
						glVertex2i((int)m1,(int)k1);
						glVertex2i((int)m2,(int)k1);
						glVertex2i((int)m2,(int)k2);
						glVertex2i((int)m1,(int)k2);
					glEnd();
				} else {
					glColor3d(.5,.5,.5);
					glBegin(GL_TRIANGLES);
						glVertex2i((int)m1,(int)k1);
						glVertex2i((int)m2,(int)k2);
						glVertex2i((int)m1,(int)k2);
					glEnd();
					glColor3d(.8,.8,.8);
					glBegin(GL_TRIANGLES);
						glVertex2i((int)m1,(int)k1);
						glVertex2i((int)m2,(int)k1);
						glVertex2i((int)m2,(int)k2);
					glEnd();
				}
				if (w>400 && h>300){
					glColor3d(1.0,1.0,1.0);
					glRasterPos2i((int)m1+3,(int)k2-10);
					cstr.Format("%d    ",i*16+j);
					glCallLists(5,GL_UNSIGNED_BYTE,cstr);
				}
				m1+=hs;
				m2+=hs;
			}
			k1+=vs;
			k2+=vs;
		}

		glColor3d(1.0,1.0,1.0);
		glBegin(GL_LINE_LOOP);
			glVertex2f(hs*(ssourcecolor%16),vs*(ssourcecolor/16));
			glVertex2f(hs*(ssourcecolor%16+1),vs*(ssourcecolor/16));
			glVertex2f(hs*(ssourcecolor%16+1),vs*(ssourcecolor/16+1));
			glVertex2f(hs*(ssourcecolor%16),vs*(ssourcecolor/16+1));
		glEnd();


		*newmap=NULL;
		*newlmap=NULL;
		break;
	case 1:	// color map
		glClear(GL_COLOR_BUFFER_BIT);

		hs=(float)w/16.0f;
		vs=(float)h/16.0f;

		k1=.0;
		k2=vs;
		glListBase(2000);
		for (i=0;i<16;i++){
			m1=.0;
			m2=hs;
			for(j=0;j<16;j++){
				glColor4ubv((GLubyte *)&colormap[i*16+j]);
				glBegin(GL_QUADS);
					glVertex2i((int)m1,(int)k1);
					glVertex2i((int)m2,(int)k1);
					glVertex2i((int)m2,(int)k2);
					glVertex2i((int)m1,(int)k2);
				glEnd();

				if (w>400 && h>300){
					glColor3d(1.0,1.0,1.0);
					glRasterPos2i((int)m1+3,(int)k2-10);
					cstr.Format("%d    ",i*16+j);
					glCallLists(5,GL_UNSIGNED_BYTE,cstr);
				}
				m1+=hs;
				m2+=hs;
			}
			k1+=vs;
			k2+=vs;
		}

		glColor3d(1.0,1.0,1.0);
		glBegin(GL_LINE_LOOP);
			glVertex2f(hs*(scolor%16),vs*(scolor/16));
			glVertex2f(hs*(scolor%16+1),vs*(scolor/16));
			glVertex2f(hs*(scolor%16+1),vs*(scolor/16+1));
			glVertex2f(hs*(scolor%16),vs*(scolor/16+1));
		glEnd();


		*newmap=NULL;
		*newlmap=NULL;
		break;
	case 0:	// draw fractals
		if (map==NULL){				// if the stored map is invalid, recalculate the fractals
			glColor3d(.0,.0,.0);	// else paste the stored map onto window
			glBegin(GL_POINTS);
			for (i=0;i<=h;i++)
				for (j=i%2;j<w;j+=2){
					glVertex2i(j,i);
				}
			glEnd();
			
			glColor3d(1.0,1.0,1.0);
			glRasterPos2i(5,h-17);
			glListBase(1000);
			glCallLists(14,GL_UNSIGNED_BYTE,"Calculating...");
			glFlush();
			map=(unsigned char*)malloc(w*h*sizeof(unsigned char));
			lmap=(unsigned long*)malloc(w*h*sizeof(unsigned long));
			if (map==NULL) exit(1);
			if (lmap==NULL) exit(1);

			if (type==SET_MANDELBROT){
				Mandelbrot(left,right,bottom,top,w,h,map);
			} else {
				Julia(left,right,bottom,top,ca,cb,w,h,map);
			}

			for(i=0;i<h;i++){
				for(j=0;j<w;j++){
					lmap[i*w+j]=colormap[map[i*w+j]];
				}
			}
			*newmap=map;
			*newlmap=lmap;
		} else {
			*newmap=NULL;
			*newlmap=NULL;
		}

		// paste stored buffer lmap onto window
		glRasterPos2i(0,0);
		glPixelStorei(GL_UNPACK_ALIGNMENT ,4);
		glDrawPixels(w,h,GL_RGBA,GL_UNSIGNED_BYTE,lmap);

		if (SHOWINFO==TRUE){		// display extra information
			glColor4d(.0,.0,.0,.5);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBegin(GL_QUADS);
				glVertex2i(3,3);
				glVertex2i(265,3);
				glVertex2i(265,60);
				glVertex2i(3,60);
			glEnd();
			glDisable(GL_BLEND);
			glColor3d(1.0,1.0,1.0);
			glBegin(GL_LINE_LOOP);
				glVertex2i(3,3);
				glVertex2i(265,3);
				glVertex2i(265,60);
				glVertex2i(3,60);
			glEnd();
			glListBase(2000);
			glRasterPos2i(7,48);
			cstr.Format("X=[%.17f,%.17f]",left,right);
			glCallLists((int)cstr.GetLength(),GL_UNSIGNED_BYTE,cstr);
			glRasterPos2i(7,38);
			cstr.Format("Y=[%.17f,%.17f]",bottom,top);
			glCallLists((int)cstr.GetLength(),GL_UNSIGNED_BYTE,cstr);
			glRasterPos2i(7,28);
			if (type==SET_MANDELBROT)
			{
				cstr.Format("Zoom:  %.0fx  %.0fx",3.0f/(right-left),3.0f/(top-bottom));
			} 
			else
			{
				cstr.Format("Zoom:  %.0fx  %.0fx",4.0f/(right-left),4.0f/(top-bottom));
			}
			glCallLists((int)cstr.GetLength(),GL_UNSIGNED_BYTE,cstr);
			glRasterPos2i(7,18);
			cstr.Format("Viewport dimension: %d x %d",w,h);
			glCallLists((int)cstr.GetLength(),GL_UNSIGNED_BYTE,cstr);
			glRasterPos2i(7,8);
			if (type == SET_JULIA)
			{
				cstr.Format("c= %.17f + %.17fi",ca,cb);
				glCallLists((int)cstr.GetLength(),GL_UNSIGNED_BYTE,cstr);
			}
		}
		break;
	}
	// Flush drawing commands
	glFlush();

}

//////////////////////////////////////////////////////////////////////
// open fractals files
// (*.mfr)- mandelbrot set, (*.jfr)- julia set
void FractalsFrameWindow::CmFileOpen()
{
	CFileDialog *fdlg;
	char c[100];
	unsigned long a;
	unsigned long r,g,b,idx;
	int tmpint;
	int i;

	if (fractalstype==SET_MANDELBROT){
		fdlg=new CFileDialog(TRUE,".mfr",NULL,OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,"Mandelbrot Set (*.mfr)|*.mfr||",this);
	} else {
		fdlg=new CFileDialog(TRUE,".jfr",NULL,OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,"Julia Set (*.jfr)|*.jfr||",this);
	}

	if (fdlg->DoModal()==IDOK){
		if (fdlg->GetPathName().IsEmpty()==TRUE){
			MessageBox("Empty string.","Information",MB_OK | MB_ICONINFORMATION);
			return;
		}
//		ifstream in=ifstream(fdlg->GetPathName());
		ifstream in;
		in.open(fdlg->GetPathName(),ios_base::in);

		in.getline(c,100);
		in >> tmpint;
		if (tmpint!=fractalstype){
			MessageBox("Unmatched fractals file, open abort.","Error",MB_OK | MB_ICONERROR);
			in.close();
			return;
		}
		in.getline(c,100);
		in.getline(c,100);
		in >> cleft[zoomlevel] >> cright[zoomlevel] >> cbottom[zoomlevel] >>ctop[zoomlevel];
		in.getline(c,100);
		in.getline(c,100);
		for (i=0;i<256;i++){
			in >> idx >> r >> g >> b;
			if ((unsigned int)i!=idx){
				MessageBox("Fractals file error, open abort.","Error",MB_OK | MB_ICONERROR);
				in.close();
				return;
			}
			colormap[i]=r | g <<8 | b <<16 | 0xFF000000;
			in.getline(c,100);

		}
		in.getline(c,100);
		for (i=0;i<256;i++){
			in >> idx >> r >> g >> b;
			if ((unsigned int)i!=idx){
				MessageBox("Fractals file error, open abort.","Error",MB_OK | MB_ICONERROR);
				in.close();
				return;
			}
			keycolormap[i]=r | g <<8 | b <<16 | 0xFF000000;
			in.getline(c,100);

		}
		in.getline(c,100);
		for (i=0;i<256;i++){
			in >> idx >> a ;
			if ((unsigned int)i!=idx){
				MessageBox("Fractals file error, open abort.","Error",MB_OK | MB_ICONERROR);
				in.close();
				return;
			}
			keycolormapmask[i]=(unsigned char)a;
			in.getline(c,100);
		}
		if (fractalstype==SET_JULIA){
			in.getline(c,100);
			in >> ca >> cb;
		}
		in.close();
		if (map!=NULL){
			free(map);
			free(lmap);
			map=NULL;
			lmap=NULL;
		}
		wglMakeCurrent(m_hDC,m_hRC);
		unsigned char *newmap;
		unsigned long *newlmap;
		RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
		if (newmap!=NULL){
			map=newmap;
			lmap=newlmap;
		}
		wglMakeCurrent(m_hDC,NULL);

	}
	delete fdlg;

}
	
//////////////////////////////////////////////////////////////////////
// save fractals type, colormaps, and viewports etc in file
// (*.mfr)- mandelbort set, (*.jfr)- julia set
void FractalsFrameWindow::CmFileSave()
{
	CFileDialog *fdlg;
	int i;

	if (fractalstype==SET_MANDELBROT){
		fdlg=new CFileDialog(FALSE,".mfr",NULL,OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,"Mandelbrot Set (*.mfr)|*.mfr||",this);
	} else {
		fdlg=new CFileDialog(FALSE,".jfr",NULL,OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,"Julia Set (*.jfr)|*.jfr||",this);
	}

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
		out << "FractalsType" << endl;
		out << fractalstype << endl;

		out << "Domain" << endl;
		out << cleft[zoomlevel] << " ";
		out << cright[zoomlevel] << " ";
		out << cbottom[zoomlevel] << " ";
		out << ctop[zoomlevel] << endl;

		out << "Colormap 256" << endl;
		for (i=0;i<256;i++){
			out << i << " ";
			out << ((unsigned long)colormap[i] & 0xFF) << " ";
			out << (((unsigned long)colormap[i] & 0xFF00) >> 8) << " ";
			out << (((unsigned long)colormap[i] & 0xFF0000) >> 16) << endl;
		}
		out << "Keycolormap 256" << endl;
		for (i=0;i<256;i++){
			out << i << " ";
			out << ((unsigned long)keycolormap[i] & 0xFF) << " ";
			out << (((unsigned long)keycolormap[i] & 0xFF00) >> 8) << " ";
			out << (((unsigned long)keycolormap[i] & 0xFF0000) >> 16) << endl;
		}
		out << "Keycolormapmask 256" << endl;
		for (i=0;i<256;i++){
			out << i << " ";
			out << ((unsigned long)keycolormapmask[i]) << endl;
		}

		out << "ComplexNumberC" << endl;
		out << ca << " ";
		out << cb << endl;

		out.close();

		MessageBox("Save successfully complete.","Information",MB_OK | MB_ICONINFORMATION);

	}
	delete fdlg;
}

//////////////////////////////////////////////////////////////////////
// bye bye
void FractalsFrameWindow::CmExit()
{
	PostMessage(WM_CLOSE);
}

//////////////////////////////////////////////////////////////////////
// show the about dialogbox, hi...hi...
void FractalsFrameWindow::CmAbout()
{
	if (aboutdialog==NULL){
		aboutdialog=new AboutDialog(this);
		aboutdialog->DoModal();
		delete aboutdialog;
		aboutdialog=NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// resize event, call GLResize() to redefine the viewport
void FractalsFrameWindow::OnSize(UINT nType, int cx, int cy) 
{
	width=cx;
	height=cy;

	CFrameWnd::OnSize(nType, cx, cy);

	VERIFY(wglMakeCurrent(m_hDC,m_hRC));

	// Prevent a divide by zero
	if(cy == 0)
		cy = 1;

	// Set Viewport to window dimensions
    glViewport(0, 0, cx, cy);

	glMatrixMode(GL_PROJECTION);
	// Reset coordinate system, 1 unit = 1 pixel
	glLoadIdentity();

	//gluOrtho2D(-.5,cx-.5,-.5,cy-.5);
	gluOrtho2D(0,cx,0,cy);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	VERIFY(wglMakeCurrent(NULL,NULL));

	if (map!=NULL){
		free(map);
		free(lmap);
		map=NULL;
		lmap=NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// called when window receives WM_PAINT, render scene
void FractalsFrameWindow::OnPaint()
{
	PAINTSTRUCT ps;
	RECT rc;

	// check if the WM_PAINT message is for this window
    BeginPaint(&ps);
	GetClientRect(&rc); 

	if ((ps.rcPaint.left>rc.right) && (ps.rcPaint.right<rc.left) && (ps.rcPaint.top>rc.bottom) && (ps.rcPaint.bottom<rc.top)) return; 
	EndPaint(&ps); 

	// Make the rendering context current
	VERIFY(wglMakeCurrent(m_hDC,m_hRC));

	// Call our external OpenGL code
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	// Allow other rendering contexts to co-exist
	VERIFY(wglMakeCurrent(m_hDC,NULL));



}

//////////////////////////////////////////////////////////////////////
// override to keep the background from being erased everytime
// the window is repainted
BOOL FractalsFrameWindow::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// mouse down event, used for selecting color in colormaps and
// starting rubber banding in zooming
void FractalsFrameWindow::OnLButtonDown( UINT nFlags, CPoint point )
{
	unsigned char *newmap;
	unsigned long *newlmap;

	if (nFlags & MK_LBUTTON){
		BUTDOWN=TRUE;
		sx=point.x;
		sy=height-1-point.y;
		switch(showcolormap){
		case 2:
			selectedkeycolor=(sy*16/height)*16+sx*16/width;
			wglMakeCurrent(m_hDC,m_hRC);
			RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
			if (newmap!=NULL){
				map=newmap;
				lmap=newlmap;
			}
			wglMakeCurrent(m_hDC,NULL);
			break;

		case 1:
			selectedcolor=(sy*16/height)*16+sx*16/width;

			wglMakeCurrent(m_hDC,m_hRC);
			RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
			if (newmap!=NULL){
				map=newmap;
				lmap=newlmap;
			}
			wglMakeCurrent(m_hDC,NULL);
			break;

		case 0:
			wglMakeCurrent(m_hDC,m_hRC);
			XORRentangle(sx,sy,sx,sy);
			wglMakeCurrent(m_hDC,NULL);
			px=sx;
			py=sy;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// mouse move event, used in zooming and rubber banding
void FractalsFrameWindow::OnMouseMove( UINT nFlags, CPoint point )
{

	if (BUTDOWN==TRUE){
		cx=point.x;
		cy=height-1-point.y;
		switch(showcolormap){

		case 0:
			if (cx>=width) cx=width-1;
			if (cx<0) cx=0;
			if (cy>=height) cy=height-1;
			if (cy<0) cy=0;
			if (cx!=px || cy!=py){
				wglMakeCurrent(m_hDC,m_hRC);
				XORRentangle(sx,sy,px,py);
				XORRentangle(sx,sy,cx,cy);
				wglMakeCurrent(m_hDC,NULL);
				px=cx;
				py=cy;
			}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// mouse up event, used for zoom in
void FractalsFrameWindow::OnLButtonUp( UINT nFlags, CPoint point )
{
	unsigned char *newmap;
	unsigned long *newlmap;

	if (BUTDOWN==TRUE){
		BUTDOWN=FALSE;
		ex=point.x;
		ey=height-1-point.y;
		switch(showcolormap){
		case 0:
			if (ex!=sx || ey!=sy){
				if ((zoomlevel<49) && (ex!=sx && ey!=sy)){
					free(map);
					free(lmap);
					map=NULL;
					lmap=NULL;

					double left,right,top,bottom;
					left=cleft[zoomlevel];
					right=cright[zoomlevel];
					top=ctop[zoomlevel];
					bottom=cbottom[zoomlevel];
					cleft[zoomlevel+1]=(double)min(ex,sx)*(right-left)/width+left;
					cright[zoomlevel+1]=(double)max(ex,sx)*(right-left)/width+left;
					cbottom[zoomlevel+1]=(double)min(ey,sy)*(top-bottom)/height+bottom;
					ctop[zoomlevel+1]=(double)max(ey,sy)*(top-bottom)/height+bottom;
					zoomlevel++;
				}
				wglMakeCurrent(m_hDC,m_hRC);
				RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
				if (newmap!=NULL){
					map=newmap;
					lmap=newlmap;
				}
				wglMakeCurrent(m_hDC,NULL);
			} else {
				if (fractalstype==SET_MANDELBROT){
					ca=(double)ex*(cright[zoomlevel]-cleft[zoomlevel])/width+cleft[zoomlevel];
					cb=(double)ey*(ctop[zoomlevel]-cbottom[zoomlevel])/height+cbottom[zoomlevel];
					//if (fractalstype==SET_MANDELBROT) {
						FractalsFrameWindow *wind=(FractalsFrameWindow*)FindWindow(NULL,"Julia Set");
						wind->Redraw();

					//}
				}
			}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// double click event, for color select
void FractalsFrameWindow::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	CColorDialog *cdlg;
	COLORREF color;
	unsigned char *newmap;
	unsigned long *newlmap;

	ex=point.x;
	ey=height-1-point.y;
	switch(showcolormap){
	case 2:
		selectedkeycolor=(sy*16/height)*16+sx*16/width;

		cdlg=new CColorDialog(RGB(keycolormap[selectedkeycolor] & 0xFF,(keycolormap[selectedkeycolor] & 0xFF00)>>8,(keycolormap[selectedkeycolor] & 0xFF0000)>>16),CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN,this);

		if (cdlg->DoModal()==IDOK){
			color=cdlg->GetColor();
			keycolormap[selectedkeycolor]=(unsigned long)GetRValue(color) | (unsigned long)GetGValue(color)<<8 | (unsigned long)GetBValue(color)<<16 | 0xFF000000;
		

			if (showcolormap!=2){
				return;
			}

			keycolormapmask[selectedkeycolor]=1;

			GenerateColormap();

			if (map!=NULL){
				for(int i=0;i<height;i++){
					for(int j=0;j<width;j++){
						lmap[i*width+j]=colormap[map[i*width+j]];
					}
				}
			}

			wglMakeCurrent(m_hDC,m_hRC);
			RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
			if (newmap!=NULL){
				map=newmap;
				lmap=newlmap;
			}
			wglMakeCurrent(m_hDC,NULL);
		}
		delete cdlg;
		break;

	case 1:
		selectedcolor=(sy*16/height)*16+sx*16/width;

		cdlg=new CColorDialog(RGB(colormap[selectedcolor] & 0xFF,(colormap[selectedcolor] & 0xFF00)>>8,(colormap[selectedcolor] & 0xFF0000)>>16),CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN,this);

		if (cdlg->DoModal()==IDOK){
			color=cdlg->GetColor();
			colormap[selectedcolor]=(unsigned long)GetRValue(color) | (unsigned long)GetGValue(color)<<8 | (unsigned long)GetBValue(color)<<16 | 0xFF000000;

			if (map!=NULL){
				for (int i=0;i<height;i++){
					for(int j=0;j<width;j++){
						if (map[i*width+j]==selectedcolor){
							lmap[i*width+j]=colormap[selectedcolor];;
						}
					}
				}
			}

			wglMakeCurrent(m_hDC,m_hRC);
			RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
			if (newmap!=NULL){
				map=newmap;
				lmap=newlmap;
			}
			wglMakeCurrent(m_hDC,NULL);
		}
		delete cdlg;
		break;
	}
}

//////////////////////////////////////////////////////////////////////
// toggle whether to show the color map
void FractalsFrameWindow::CmColorMap()
{
	if (showcolormap==1){
		showcolormap=0;
	} else {
		showcolormap=1;
	}

	UINT mid;

	CMenu *mmenu;

	mmenu=GetMenu();
	mmenu=mmenu->GetSubMenu(1);
	mid=mmenu->GetMenuItemID(1);
	mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	mid=mmenu->GetMenuItemID(0);
	if (showcolormap==1){
		mmenu->CheckMenuItem(mid,MF_CHECKED);
	} else {
		mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	}

	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);	

}

//////////////////////////////////////////////////////////////////////
// toggle whether to show the key color map
void FractalsFrameWindow::CmKeyColorMap()
{
	if (showcolormap==2){
		showcolormap=0;
	} else {
		showcolormap=2;
	}

	UINT mid;

	CMenu *mmenu;

	mmenu=GetMenu();
	mmenu=mmenu->GetSubMenu(1);
	mid=mmenu->GetMenuItemID(0);
	mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	mid=mmenu->GetMenuItemID(1);
	if (showcolormap==2){
		mmenu->CheckMenuItem(mid,MF_CHECKED);
	} else {
		mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	}

	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);	

}

//////////////////////////////////////////////////////////////////////
// toggle the extra information display
void FractalsFrameWindow::CmInformation()
{
	if (SHOWINFO==TRUE) SHOWINFO=FALSE;
	else SHOWINFO=TRUE;
	int mid;

	CMenu *mmenu;

	mmenu=GetMenu();
	mmenu=mmenu->GetSubMenu(1);
	mid=mmenu->GetMenuItemID(2);
	if (SHOWINFO==TRUE){
		mmenu->CheckMenuItem(mid,MF_CHECKED);
	} else {
		mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	}

	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);	

}

//////////////////////////////////////////////////////////////////////
// zoom out one level
void FractalsFrameWindow::CmZoomOut()
{
	if (zoomlevel==0) return;
	zoomlevel--;
	if (map!=NULL){
		free(map);
		free(lmap);
		map=NULL;
		lmap=NULL;
	}
	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);
}

//////////////////////////////////////////////////////////////////////
// zoom out to the top level
void FractalsFrameWindow::CmZoomTop()
{
	zoomlevel=0;
	if (fractalstype==SET_MANDELBROT){
		cleft[0]=-2.5;
		cright[0]=1.5;
		cbottom[0]=-2.0;
		ctop[0]=2.0;
	} else {
		cleft[0]=-2.0;
		cright[0]=2.0;
		cbottom[0]=-2.0;
		ctop[0]=2.0;
	}
	if (map!=NULL){
		free(map);
		free(lmap);
		map=NULL;
		lmap=NULL;
	}
	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);
}

void FractalsFrameWindow::CmAspectRatio()
{
	double mid,half,wratio,dratio;

	if (height==0) height=1;
	if (width==0) width=1;

	wratio=(double)height/width;
	dratio=(ctop[zoomlevel]-cbottom[zoomlevel])/(cright[zoomlevel]-cleft[zoomlevel]);

	if (wratio>dratio){
		mid=(ctop[zoomlevel]+cbottom[zoomlevel])/2.0;
		half=(ctop[zoomlevel]-cbottom[zoomlevel])*wratio/dratio/2.0;
		ctop[zoomlevel]=mid+half;
		cbottom[zoomlevel]=mid-half;
	} else {
		mid=(cright[zoomlevel]+cleft[zoomlevel])/2.0;
		half=(cright[zoomlevel]-cleft[zoomlevel])*dratio/wratio/2.0;
		cright[zoomlevel]=mid+half;
		cleft[zoomlevel]=mid-half;
	}
	if (map!=NULL){
		free(map);
		free(lmap);
		map=NULL;
		lmap=NULL;
	}
	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);
}

//////////////////////////////////////////////////////////////////////
// remove the selected key color
void FractalsFrameWindow::CmRemoveKeyColor()
{
	keycolormapmask[selectedkeycolor]=0;

	GenerateColormap();

	if (map!=NULL){
		for(int i=0;i<height;i++){
			for(int j=0;j<width;j++){
				lmap[i*width+j]=colormap[map[i*width+j]];
			}
		}
	}

	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);
}

//////////////////////////////////////////////////////////////////////
// set the colormap to all black
void FractalsFrameWindow::CmClearColor()
{
	int i;

	for (i=0;i<256;i++){	// empty color map
		colormap[i]=0xFF000000;
		keycolormap[i]=0xFF000000;
		keycolormapmask[i]=0;
	}
	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);	
}

//////////////////////////////////////////////////////////////////////
// reset to the default colormap
void FractalsFrameWindow::CmStdColor()
{
	unsigned long i;

	for (i=0;i<256;i++){	// initial color map
		colormap[i]=0xFF000000;
		keycolormap[i]=0xFF000000;
		keycolormapmask[i]=0;
	}
	
	keycolormap[7]=0xFF424015;
	keycolormapmask[7]=1;
	keycolormap[71]=0xFFFFFFFF;
	keycolormapmask[71]=1;
	keycolormap[119]=0xFF0000A8;
	keycolormapmask[119]=1;
	keycolormap[167]=0xFF6F001C;
	keycolormapmask[167]=1;
	keycolormap[215]=0xFF79FDF3;
	keycolormapmask[215]=1;

	GenerateColormap();
	
	colormap[0]=0xFF000000;

	if (map!=NULL){
		if (map!=NULL){
			for(int i=0;i<height;i++){
				for(int j=0;j<width;j++){
					lmap[i*width+j]=colormap[map[i*width+j]];
				}
			}
		}
	}
	if (showcolormap==1) CmColorMap();
	if (showcolormap==2) CmKeyColorMap();
	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);	
}

//////////////////////////////////////////////////////////////////////
// import colormap from other files
// support files:
//		windows bitmap (*.bmp)
//		mandelbort set fractals file (*.mfr)
//		julia set fractals file (*.jfr)
void FractalsFrameWindow::CmImpColor()
{
	CFileDialog *fdlg;
	char c[100];
	unsigned long idx,r,g,b;
	int i;
	unsigned long a;

	fdlg=new CFileDialog(TRUE,".bmp",NULL,OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,"Windows Bitmaps (*.bmp)|*.bmp|Mandelbrot Set (*.mfr)|*.mfr|Julia Set (*.jfr)|*.jfr||",this);

	if (fdlg->DoModal()==IDOK){
		if (fdlg->GetPathName().Right(3)=="bmp"){
			AUX_RGBImageRec* DIBImage;
			DIBImage=auxDIBImageLoadA(fdlg->GetPathName());
			if( 0 == DIBImage ){
				MessageBox("Cannot open BMP file","Error",MB_OK | MB_ICONERROR);
				return;
			}
			if (DIBImage->sizeX<256){
				MessageBox("The width of the BMP image is less than 256.","Error",MB_OK | MB_ICONERROR);
				return;
			}
			
			for (int i=0;i<256;i++){
				colormap[i]=0xFF000000 | (long)DIBImage->data[2+i*3]<<16 | (long)DIBImage->data[1+i*3]<<8 | (long)DIBImage->data[0+i*3];
			}
			if (map!=NULL){
				for(int i=0;i<height;i++){
					for(int j=0;j<width;j++){
						lmap[i*width+j]=colormap[map[i*width+j]];
					}
				}
			}

			if (showcolormap==1) CmColorMap();
			if (showcolormap==2) CmKeyColorMap();
			wglMakeCurrent(m_hDC,m_hRC);
			unsigned char *newmap;
			unsigned long *newlmap;
			RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
			if (newmap!=NULL){
				map=newmap;
				lmap=newlmap;
			}
			wglMakeCurrent(m_hDC,NULL);
		}
		if (fdlg->GetPathName().Right(3)=="mfr" || fdlg->GetPathName().Right(3)=="jfr"){
			
			//ifstream in=ifstream(fdlg->GetPathName());
			ifstream in;
			in.open(fdlg->GetPathName(),ios_base::in);
			in.getline(c,100);
			in.getline(c,100);
			in.getline(c,100);
			in.getline(c,100);
			in.getline(c,100);
			for (i=0;i<256;i++){
				in >> idx >> r >> g >> b;
				if ((unsigned int)i!=idx){
					MessageBox("Fractals file error, open abort.","Error",MB_OK | MB_ICONERROR);
					in.close();
					return;
				}
				colormap[i]=r | g <<8 | b <<16 | 0xFF000000;
				in.getline(c,100);
			}
			in.getline(c,100);
			for (i=0;i<256;i++){
				in >> idx >> r >> g >> b;
				if ((unsigned int)i!=idx){
					MessageBox("Fractals file error, open abort.","Error",MB_OK | MB_ICONERROR);
					in.close();
					return;
				}
				keycolormap[i]=r | g <<8 | b <<16 | 0xFF000000;
				in.getline(c,100);

			}
			in.getline(c,100);
			for (i=0;i<256;i++){
				in >> idx >> a ;
				if ((unsigned int)i!=idx){
					MessageBox("Fractals file error, open abort.","Error",MB_OK | MB_ICONERROR);
					in.close();
					return;
				}
				keycolormapmask[i]=(unsigned char)a;
				in.getline(c,100);
			}
			in.close();

			if (map!=NULL){
				for(int i=0;i<height;i++){
					for(int j=0;j<width;j++){
						lmap[i*width+j]=colormap[map[i*width+j]];
					}
				}
			}
			if (showcolormap==1) CmColorMap();
			if (showcolormap==2) CmKeyColorMap();
			wglMakeCurrent(m_hDC,m_hRC);
			unsigned char *newmap;
			unsigned long *newlmap;
			RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
			if (newmap!=NULL){
				map=newmap;
				lmap=newlmap;
			}
			wglMakeCurrent(m_hDC,NULL);
		}

	}
	delete fdlg;
}

//////////////////////////////////////////////////////////////////////
// force to calculate the fractals again
void FractalsFrameWindow::Redraw()
{
	if (map!=NULL){
		free(map);
		free(lmap);
		map=NULL;
		lmap=NULL;
	}
	wglMakeCurrent(m_hDC,m_hRC);
	unsigned char *newmap;
	unsigned long *newlmap;
	RenderScene(SHOWINFO,selectedcolor,selectedkeycolor,showcolormap,fractalstype,cleft[zoomlevel],cright[zoomlevel],cbottom[zoomlevel],ctop[zoomlevel],ca,cb,width,height,map,lmap,&newmap,&newlmap,colormap,keycolormap,keycolormapmask);
	if (newmap!=NULL){
		map=newmap;
		lmap=newlmap;
	}
	wglMakeCurrent(m_hDC,NULL);
}

//////////////////////////////////////////////////////////////////////
// generate the colormap from key colors
void FractalsFrameWindow::GenerateColormap()
{
	int i,j,k,m;
	float r,g,b,r1,g1,b1,r2,g2,b2,dr,dg,db;

	int sindex,steps;


	for (i=0;i<256;i++){
		if (keycolormapmask[i]>0) break;
	}

	if (i==256){
		for (j=0;j<256;j++){
			colormap[j]=keycolormap[j];
		}
		return;
	}

	sindex=i;

	do {
		k=i;
		for (j=1;j<256;j++){
			if (keycolormapmask[(k+j)%256]>0){
				break;
			}
		}
		// special case when only one key color exists
		// fill the whole colormap the same color
		if (j==256){
			for (j=0;j<256;j++){
				colormap[j]=keycolormap[i];
			}
			return;
		}
		steps=j;
		r1=(float)((unsigned long)keycolormap[k] & 0xFF)/255.0f;
		g1=(float)((unsigned long)((unsigned long)keycolormap[k] & 0xFF00) >> 8)/255.0f;
		b1=(float)((unsigned long)((unsigned long)keycolormap[k] & 0xFF0000) >> 16)/255.0f;
		r2=(float)((unsigned long)keycolormap[(k+j)%256] & 0xFF)/255.0f;
		g2=(float)((unsigned long)((unsigned long)keycolormap[(k+j)%256] & 0xFF00) >> 8)/255.0f;
		b2=(float)((unsigned long)((unsigned long)keycolormap[(k+j)%256] & 0xFF0000) >> 16)/255.0f;

		if (steps>1){
			dr=(r2-r1)/steps;
			dg=(g2-g1)/steps;
			db=(b2-b1)/steps;
			for (m=0;m<steps;m++){
				r=r1+dr*m;
				g=g1+dg*m;
				b=b1+db*m;
				colormap[(i+m)%256]=((unsigned long)(r*255.0f)) | ((unsigned long)((unsigned long)(g*255.0f)))<<8 | ((unsigned long)((unsigned long)(b*255.0f)))<<16 | 0xFF000000;
			}
			i+=steps;
		} else {
			colormap[i]=keycolormap[i];
			i++;
		}

		i%=256;
	} while (i!=sindex);

}


void FractalsFrameWindow::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	if (map) delete map, map=NULL;
	if (lmap) delete lmap, lmap=NULL;


	if (fractalstype==SET_JULIA){
		FractalsFrameWindow *wind=(FractalsFrameWindow*)FindWindow(NULL,"Mandelbrot Set");
		wind->CloseWindow();
		wind->DestroyWindow();
		
	}

	if (fractalstype==SET_MANDELBROT){
		FractalsFrameWindow *wind=(FractalsFrameWindow*)FindWindow(NULL,"Julia Set");
		wind->CloseWindow();
		wind->DestroyWindow();
		
	}
	CFrameWnd::OnClose();
}
