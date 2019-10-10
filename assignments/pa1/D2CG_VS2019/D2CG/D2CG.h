//////////////////////////////////////////////////////////////////////
// D2CG.h
// the file defines 3 classes:
//		D2CGApp				the application class
//		D2CGFrameWindow		the windows class with menu
//		AboutDialog				the about dialog class
#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers


using namespace std;

enum { WINDOW_Triangles=0, WINDOW_FRACTALS=1};

//////////////////////////////////////////////////////////////////////
// D2CG windows class definition
class D2CGFrameWindow : public CFrameWnd
{
public:
	////////////////////////////////
	// D2CG related properties
	int windowType;									// type 0-primitives, 1-fractal

	int wx,wy;


	HGLRC m_hRC;										// rendering context
	HDC m_hDC;											// device context

	//CPalette m_GLPalette;	// Logical Palette

	D2CGFrameWindow(int type, int x, int y);
	~D2CGFrameWindow();


	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	//{{AFX_MSG(CFloodDoc)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
protected:
	// Render the window with respective method
	void RenderScene(void);
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);



public:
	afx_msg void OnEditUndo40017();
	afx_msg void OnEditFractalize();
	afx_msg void OnFileOpen40019();
	afx_msg void OnFileSave40020();
};



//////////////////////////////////////////////////////////////////////
// application class definition
class D2CGApp : public CWinApp
{
public:

	D2CGApp(): CWinApp ("2D Drawing in CG")
	{

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

// DepthDlg dialog

class DepthDlg : public CDialog
{
	DECLARE_DYNAMIC(DepthDlg)

public:
	DepthDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~DepthDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// the depth of recursion specified
	int m_intDepth;
};
