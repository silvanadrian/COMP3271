//////////////////////////////////////////////////////////////////////
// Fractals.h
// the file defines 3 classes:
//		FractalsApp				the application class
//		FractalsFrameWindow		the windows class with menu
//		AboutDialog				the about dialog class
#pragma once

enum { SET_MANDELBROT=0, SET_JULIA=1};

static double ca,cb;									// complex number c=a+bi, for julia set

//////////////////////////////////////////////////////////////////////
// fractals windows class definition
class FractalsFrameWindow : public CFrameWnd
{
public:
	////////////////////////////////
	// fractals related properties
	int fractalstype;									// type 0-mandelbrot, 1-julia
	int zoomlevel;										// current zoom level
	double cleft[50],cright[50],ctop[50],cbottom[50];	// 50 levels of zoom, storing ranges
	unsigned char *map;									// stored screen in color index for faster repaint
	unsigned long *lmap;								// stored screen in rgb format
	unsigned long colormap[256];						// color map
	unsigned long keycolormap[256];						// key color map, for adjusting color map
	unsigned char keycolormapmask[256];					// key color map entry influrences

	////////////////////////////////
	// interface related properties
	int wleft,wtop,width,height;						// windows position and size
	unsigned char selectedcolor;						// selected color in colormap
	unsigned char selectedkeycolor;						// selected color in source color map
	BOOL SHOWINFO;										// whether to show extra information
	BOOL BUTDOWN;										// mouse button down
	int showcolormap;									// what to display 0-fractals,1-color map,2-keycolor map
	int sx,sy,ex,ey,cx,cy,px,py;						// mouse positions s=start,e=end,c=current,p=previous

	HGLRC m_hRC;										// rendering context
	HDC m_hDC;											// device context

	//CPalette m_GLPalette;	// Logical Palette

	FractalsFrameWindow(int type, int x, int y);
	~FractalsFrameWindow();
	
	void GenerateColormap();
	void XORRentangle(int x1, int y1,int x2, int y2);

	// render the scene
	void RenderScene(BOOL SHOWINFO,unsigned char scolor,unsigned char skeycolor,BOOL showcolormap,int type,double left,double right,double bottom,double top,double a,double b,int w,int h,unsigned char *map,unsigned long *lmap,unsigned char **newmap,unsigned long **newlmap,unsigned long *colormap,unsigned long *keycolormap,unsigned char *keycolormapmask);

	
	void Redraw();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	//{{AFX_MSG(CFloodDoc)
	afx_msg void CmFileOpen();
	afx_msg void CmFileSave();
	afx_msg void CmExit();
	afx_msg void CmAbout();
	afx_msg void CmColorMap();
	afx_msg void CmKeyColorMap();
	afx_msg void CmInformation();
	afx_msg void CmZoomOut();
	afx_msg void CmZoomTop();
	afx_msg void CmAspectRatio();
	afx_msg void CmRemoveKeyColor();
	afx_msg void CmClearColor();
	afx_msg void CmStdColor();
	afx_msg void CmImpColor();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
};

//////////////////////////////////////////////////////////////////////
// application class definition
class FractalsApp : public CWinApp
{
public:

	FractalsApp(): CWinApp ("Fractals")
	{
		ca=.0;		// inistial complex number c for julia set is 0
		cb=.0;
	}
	
	BOOL InitInstance();

};


//////////////////////////////////////////////////////////////////////
// about dialog class definition
class AboutDialog : public CDialog
{
public:
	AboutDialog(CWnd *parent) : CDialog(IDD_ABOUT,parent)
	{}
};

// global aboutdialog to prevent 2 such windows created at the same time
AboutDialog *aboutdialog;
