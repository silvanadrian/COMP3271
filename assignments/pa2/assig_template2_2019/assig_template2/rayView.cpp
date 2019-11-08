// rayView.cpp : implementation of the CRayView class
//

#include "stdafx.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include "GL/GLAUX.H"
#include <iomanip>
#include <fstream>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <winbase.h>
#include "ray.h"

#include "Texture.h"
#include "Material.h"
#include "Primitive.h"
#include "LightSource.h"
#include "rayDoc.h"
#include "rayView.h"
#include "PreferencesDialog.h"
#include "JpegFile.h"
#include "BmpFile.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CRayApp theApp;

int boxedge[12][2]={{0,1},{1,2},{2,3},{3,0},
					{0,4},{1,5},{2,6},{3,7},
					{4,5},{5,6},{6,7},{7,4}};
int boxface[6][5]={{0,1,2,3,4},{1,5,6,2,0},{2,6,7,3,1},
					{5,4,7,6,1},{0,3,7,4,1},{1,0,4,5,2}};


/////////////////////////////////////////////////////////////////////////////
// CRayView

IMPLEMENT_DYNCREATE(CRayView, CView)

BEGIN_MESSAGE_MAP(CRayView, CView)
	//{{AFX_MSG_MAP(CRayView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_COMMAND(ID_SELECT, OnSelect)
	ON_COMMAND(ID_NAVIGATE, OnNavigate)
	ON_UPDATE_COMMAND_UI(ID_SELECT, OnUpdateSelect)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATE, OnUpdateNavigate)
	ON_COMMAND(ID_ANTIALIASING, OnAntialiasing)
	ON_UPDATE_COMMAND_UI(ID_ANTIALIASING, OnUpdateAntialiasing)
	ON_COMMAND(ID_MISCMAT, OnMiscmat)
	ON_COMMAND(ID_PREFERENCES, OnPreferences)
	ON_COMMAND(ID_IMPORT_TX, OnImportTx)	
	ON_COMMAND(ID_ZOOM, OnZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM, OnUpdateZoom)
	ON_COMMAND(ID_REMOVETX, OnRemovetx)
	ON_COMMAND(ID_LIGHT_COLOR, OnLightColor)
	//}}AFX_MSG_MAP
//	ON_MESSAGE(WM_USER+1, OnUserNewDoc)
END_MESSAGE_MAP()

// rv = m r, m is 4x4, r is 3x1, rv is 3x1
void MatrixMultVector3(float *m, float *v, float *rv)
{
	rv[0]=m[0]*v[0]+m[4]*v[1]+m[8]*v[2];
	rv[1]=m[1]*v[0]+m[5]*v[1]+m[9]*v[2];
	rv[2]=m[2]*v[0]+m[6]*v[1]+m[10]*v[2];
}

// rv = m r, m is 4x4, r is 4x1, rv is 4x1
void MatrixMultVector4(float *m, float *v, float *rv)
{
	rv[0]=m[0]*v[0]+m[4]*v[1]+m[8]*v[2]+m[12]*v[3];
	rv[1]=m[1]*v[0]+m[5]*v[1]+m[9]*v[2]+m[13]*v[3];
	rv[2]=m[2]*v[0]+m[6]*v[1]+m[10]*v[2]+m[14]*v[3];
	rv[3]=m[3]*v[0]+m[7]*v[1]+m[11]*v[2]+m[15]*v[3];
}


/////////////////////////////////////////////////////////////////////////////
// interface control related variables
V3 rotcenter;
V3 m1,m2;
int scaleDir;


//////////////////////////////////////////////////////////////
// this function is called by the template whenever the left button is pressed
// parameters:
//		pickedid is the id of the object picked
//		x and y are the screen coordinate of the click position
void CRayView::UserLeftButtonDown(int pickedid, int x, int y)
{
	CRayDoc *pDoc=GetDocument();

	if (pickedid<1000 && pickedid>=0){
		m_bRotateObj=TRUE;
		pDoc->SelectedObjectsBackupM();
		rotcenter=pDoc->m_RotateCentre;
	}
	if (pickedid<60000 && pickedid>=10000){	// move object
		int mhit=(pickedid-10000)/12;
		int edgeno=(pickedid-10000)%12;

		CPrimitive *c;

		if ((c=pDoc->ObjectWithIDExists(mhit))!=NULL) m_bMoveObj=TRUE;

		if (m_bMoveObj==TRUE){
			m1=c->m_TBoxCorner[boxedge[edgeno][0]];
			m2=c->m_TBoxCorner[boxedge[edgeno][1]];
			pDoc->SelectedObjectsBackupM();
		}
	}
	if (pickedid<60012 && pickedid>=60000){		// move object
		m_bMoveObj=TRUE;
		int edgeno=pickedid-60000;
		m1=pDoc->m_BigBoxCorner[boxedge[edgeno][0]];
		m2=pDoc->m_BigBoxCorner[boxedge[edgeno][1]];
		pDoc->SelectedObjectsBackupM();
	}
	if (pickedid<60018 && pickedid>=60012){		// resize object
		m_bResize=TRUE;
		int edgeno=pickedid-60012;
		scaleDir=edgeno%3;
		m1=(pDoc->m_BigBoxCorner[boxface[(edgeno+3)%6][0]]+pDoc->m_BigBoxCorner[boxface[(edgeno+3)%6][1]]+pDoc->m_BigBoxCorner[boxface[(edgeno+3)%6][2]]+pDoc->m_BigBoxCorner[boxface[(edgeno+3)%6][3]])/4.0f;
		m2=(pDoc->m_BigBoxCorner[boxface[edgeno][0]]+pDoc->m_BigBoxCorner[boxface[edgeno][1]]+pDoc->m_BigBoxCorner[boxface[edgeno][2]]+pDoc->m_BigBoxCorner[boxface[edgeno][3]])/4.0f;
		pDoc->SelectedObjectsBackupM();
	}
}

//////////////////////////////////////////////////////////////
// this function is called by the template whenever the left button is released
// parameters:
//		x and y are the screen coordinate of the release position
void CRayView::UserLeftButtonUp(int x, int y)
{
	CRayDoc *pDoc=GetDocument();

}

//////////////////////////////////////////////////////////////
// this function is called by the template whenever the right button is pressed
// parameters:
//		pickedid is the id of the object picked
//		x and y are the screen coordinate of the click position
void CRayView::UserRightButtonDown(int pickedid, int x, int y)
{
	CRayDoc *pDoc=GetDocument();

	if (pDoc->m_selectedobjects.size()>0){
		m_bRotateView=TRUE;
		rotcenter=pDoc->m_RotateCentre;
		BackupViewpoint();
	}
}

//////////////////////////////////////////////////////////////
// this function is called by the template whenever the right button is released
// parameters:
//		x and y are the screen coordinate of the release position
void CRayView::UserRightButtonUp(int x, int y)
{
	CRayDoc *pDoc=GetDocument();

}

//////////////////////////////////////////////////////////////
// this function is called by the template whenever the mouse is moved, regardless the click condition
// parameters:
//		x and y are the screen coordinate of the mouse position
void CRayView::UserMouseMove(int x, int y)
{
	CRayDoc *pDoc=GetDocument();

	int i,j;
	// use the already used variables a
	V3 hv,vv,sp,cp,mm,ra,tmpv1,tmpv2,int1,int2, p1, p2;
	float rmag,rmat[16],tmat[16],rsmat[16],tsmat[16],pn1[4],pn2[4];
	float t1,t2,dp1,dp2,dist1,dist2;


	if (m_bRButDown){
		if (m_bRotateView==TRUE){
			RestoreViewpoint();

			hv=(m_viewinfo.lrp-m_viewinfo.llp)/(float)m_nWidth;
			vv=(m_viewinfo.ulp-m_viewinfo.llp)/(float)m_nHeight;

			sp=m_viewinfo.llp+hv*(float)m_nRSx+vv*(float)m_nRSy;

			cp=m_viewinfo.llp+hv*(float)m_nRCx+vv*(float)m_nRCy;

			mm=cp-sp;

			ra=mm.cross(pDoc->m_camera.los);
			ra.normalize();
			rmag=mm.length();
			
			glPushMatrix();
			glLoadIdentity();
			glRotatef(-rmag*1000.0f,ra[0],ra[1],ra[2]);
			glGetFloatv(GL_MODELVIEW_MATRIX,rmat);
			glPopMatrix();

			pn1[0]=m_oldcamera.viewpoint[0]-rotcenter[0];
			pn1[1]=m_oldcamera.viewpoint[1]-rotcenter[1];
			pn1[2]=m_oldcamera.viewpoint[2]-rotcenter[2];
			pn1[3]=1.0;
			MatrixMultVector3(rmat,pn1,pn2);
			pDoc->m_camera.viewpoint[0]=rotcenter[0]+pn2[0];
			pDoc->m_camera.viewpoint[1]=rotcenter[1]+pn2[1];
			pDoc->m_camera.viewpoint[2]=rotcenter[2]+pn2[2];

			MatrixMultVector3(rmat,(float*)m_oldcamera.los,pn2);
			pDoc->m_camera.los[0]=pn2[0];
			pDoc->m_camera.los[1]=pn2[1];
			pDoc->m_camera.los[2]=pn2[2];
			MatrixMultVector3(rmat,(float*)m_oldcamera.huv,pn2);
			pDoc->m_camera.huv[0]=pn2[0];
			pDoc->m_camera.huv[1]=pn2[1];
			pDoc->m_camera.huv[2]=pn2[2];
			pDoc->FindRHV();
			
			////////////////////////////////
			pDoc->SetModifiedFlag(TRUE);
			Invalidate();
		}
	} else if (m_bLButDown){

		if (m_bRotateObj==TRUE){
			/////////////////////////////////////////
			//add your codes here for rotating object

			hv = (m_viewinfo.lrp - m_viewinfo.llp) / (float)m_nWidth;
			vv = (m_viewinfo.ulp - m_viewinfo.llp) / (float)m_nHeight;

			sp = m_viewinfo.llp + hv * (float)m_nSx + vv * (float)m_nSy;
			cp = m_viewinfo.llp + hv * (float)m_nCx + vv * (float)m_nCy;

			mm = cp - sp;

			// calc cross product
			ra = mm.cross(pDoc->m_camera.los);
			ra.normalize();
			rmag = mm.length();

			glPushMatrix();
			glLoadIdentity();
			glRotatef(rmag*1000.0f, ra[0], ra[1], ra[2]);
			glGetFloatv(GL_MODELVIEW_MATRIX, rmat);
			glPopMatrix();

			// init matrix (4x4) with zeros
			for (i = 0;i < 16;i++) tmat[i] = 0.0;

			// set the right values to 1
			tmat[0] = tmat[5] = tmat[10] = tmat[15] = 1.0;

			for (list<LPPRIMITIVE>::iterator it = pDoc->m_selectedobjects.begin(); it != pDoc->m_selectedobjects.end(); it++) {
				(*it)->RestoreM();

				tmat[12] = -(rotcenter[0]);
				tmat[13] = -(rotcenter[1]);
				tmat[14] = -(rotcenter[2]);

				(*it)->MultM(tmat);
				(*it)->MultM(rmat);

				tmat[12] = (rotcenter[0]);
				tmat[13] = (rotcenter[1]);
				tmat[14] = (rotcenter[2]);

				(*it)->MultM(tmat);
			}
			
			////////////////////////////////
			pDoc->GenerateBigBoundingBox();
			pDoc->SetModifiedFlag(TRUE);
			Invalidate();
		}

		if (m_bMoveObj==TRUE){
			if (m_nCx!=m_nSx || m_nCy!=m_nSy){
				////////////////////////////////////////////
				//add your codes here for object translation

				
				hv = (m_viewinfo.lrp - m_viewinfo.llp) / (float)m_nWidth;
				vv = (m_viewinfo.ulp - m_viewinfo.llp) / (float)m_nHeight;


				sp = m_viewinfo.llp + hv * (float)m_nSx + vv * (float)m_nSy;
				cp = m_viewinfo.llp + hv * (float)m_nCx + vv * (float)m_nCy;


				tmpv1 = cp - pDoc->m_camera.viewpoint;
				tmpv1.normalize();


				tmpv2 = sp - pDoc->m_camera.viewpoint;
				tmpv2.normalize();


				mm = m2 - m1;
				mm.normalize();

				// calcualte cross products
				p1 = mm.cross(tmpv1);
				p2 = tmpv1.cross(p1);
				p2.normalize();

				// dot product
				dp2 = p2.dot(pDoc->m_camera.viewpoint);


				t2 = (dp2 - p2[0] * m1[0] - p2[1] * m1[1] - p2[2] * m1[2]) / (p2[0] * mm[0] + p2[1] * mm[1] + p2[2] * mm[2]);
				int2 = m1 + t2 * mm;

				// calcualte cross products
				p2 = mm.cross(tmpv2);
				p1 = tmpv2.cross(p2);
				p1.normalize();

				// calcualte dot product
				dp1 = p1.dot(pDoc->m_camera.viewpoint);


				t1 = (dp1 - p1[0] * m1[0] - p1[1] * m1[1] - p1[2] * m1[2]) / (p1[0] * mm[0] + p1[1] * mm[1] + p1[2] * mm[2]);
				int1 = m1 + t1 * mm;


				for (i = 0;i < 16;i++) tmat[i] = 0.0; // init with zeros matrix of size 16 (4x4)

				// set up translation Matrix with 0 and 1s
				tmat[0] = tmat[5] = tmat[10] = tmat[15] = 1.0;

				// set the values in the trans Matrix
				tmat[12] = int2[0] - int1[0];
				tmat[13] = int2[1] - int1[1];
				tmat[14] = int2[2] - int1[2];

				// go through iterator and restore and multiplicate matrix
				for (list<LPPRIMITIVE>::iterator it = pDoc->m_selectedobjects.begin(); it != pDoc->m_selectedobjects.end(); it++) {
					(*it)->RestoreM();
					(*it)->MultM(tmat);
				}

				////////////////////////////////////////////
				pDoc->GenerateBigBoundingBox();
				pDoc->SetModifiedFlag(TRUE);
				Invalidate();
			} else {
				pDoc->SelectedObjectsRestoreM();
				Invalidate();
			}
			
		}

		if (m_bResize==TRUE){
			if (m_nCx!=m_nSx || m_nCy!=m_nSy){
				////////////////////////////////////////
				//add your codes here for object scaling
				

				hv = (m_viewinfo.lrp - m_viewinfo.llp) / (float)m_nWidth;
				vv = (m_viewinfo.ulp - m_viewinfo.llp) / (float)m_nHeight;

				sp = m_viewinfo.llp + hv * (float)m_nSx + vv * (float)m_nSy;
				cp = m_viewinfo.llp + hv * (float)m_nCx + vv * (float)m_nCy;

				tmpv1 = cp - pDoc->m_camera.viewpoint;
				tmpv1.normalize();

				tmpv2 = sp - pDoc->m_camera.viewpoint;
				tmpv2.normalize();

				mm = m2 - m1;
				dist1 = mm.length();
				mm.normalize();

				p1 = mm.cross(tmpv1);
				// calculate cross prodcuts
				p2 = tmpv1.cross(p1);
				p2.normalize();

				// dot product
				dp2 = p2.dot(pDoc->m_camera.viewpoint);

				t2 = (dp2 - p2[0] * m1[0] - p2[1] * m1[1] - p2[2] * m1[2]) / (p2[0] * mm[0] + p2[1] * mm[1] + p2[2] * mm[2]);

				int2 = m1 + t2 * mm;


				// calculate cross product for p1 and p2
				p2 = mm.cross(tmpv2);
				p1 = tmpv2.cross(p2);
				p1.normalize();

				// calcualte dot product
				dp1 = p1.dot(pDoc->m_camera.viewpoint);

				t1 = (dp1 - p1[0] * m1[0] - p1[1] * m1[1] - p1[2] * m1[2]) / (p1[0] * mm[0] + p1[1] * mm[1] + p1[2] * mm[2]);
				int1 = m1 + t1 * mm;

				tmpv1 = m2 - m1 + int2 - int1;


				dist2 = tmpv1.length();

				// go through iterator and restore and multiplicate matrix
				for (list<LPPRIMITIVE>::iterator it = pDoc->m_selectedobjects.begin(); it != pDoc->m_selectedobjects.end(); it++) {
					(*it)->RestoreM();

					// init with zeros (4x4 matrix)
					for (i = 0;i < 16;i++) tsmat[i] = 0.0;

					// set the right values to 1
					tsmat[0] = tsmat[5] = tsmat[10] = tsmat[15] = 1.0;

					// set the right values
					tsmat[12] = -(m1[0]);
					tsmat[13] = -(m1[1]);
					tsmat[14] = -(m1[2]);

					// intialize rsmat with zeros
					for (i = 0;i < 16;i++) rsmat[i] = 0.0;

					rsmat[0] = rsmat[5] = rsmat[10] = rsmat[15] = 1.0;
					// switch scaleing direction
					switch (scaleDir) {
					case 0:
						// if scaleDir = 0, set 5 element to sValue
						rsmat[5] = dist2 / dist1;
						break;
					case 1:
						// if scaleDir = 1, set 10 element to sValue
						rsmat[10] = dist2 / dist1;
						break;
					case 2:
						// if scaleDir = , set 10 element to sValue
						rsmat[0] = dist2 / dist1;
						break;
					}
					(*it)->MultM(tsmat);
					(*it)->MultM(rsmat);

					tsmat[12] = (m1[0]);
					tsmat[13] = (m1[1]);
					tsmat[14] = (m1[2]);

					(*it)->MultM(tsmat);
				}

				////////////////////////////////////////
				pDoc->GenerateBigBoundingBox();
				pDoc->SetModifiedFlag(TRUE);
				Invalidate();
			} else {
				pDoc->SelectedObjectsRestoreM();
				Invalidate();
			}
		}
	}
}

//////////////////////////////////////////////////////////////
// this function is called by the template in DrawScene. the purpose is to draw the control handles
// parameters:
//		origin  -  the origin of this local coordinate system
//		x_axis, y_axis and z_axis  -  the axis of this local coordinate system. The coordinate system is right-handed.


void CRayView::UserDrawControlHandle(V3 origin, V3 x_axis, V3 y_axis, V3 z_axis)
{
		////////////////////////////////////////
		//add your codes here for drawing a handle.

		float scaling = 0.08; // scaling the handle
		V3 points[5];
		// set up the points with which we will draw the handles + use scaling for the size of the handle
        points[0] = V3(origin.x - scaling * x_axis.x,
                        origin.y - scaling * x_axis.y,
                        origin.z - scaling * x_axis.z);
        points[1] = V3(origin.x + scaling * x_axis.x,
                        origin.y + scaling * x_axis.y,
                        origin.z + scaling * x_axis.z);
        points[2] = V3(origin.x - scaling * y_axis.x, 
					   origin.y - scaling * y_axis.y,
                       origin.z - scaling * y_axis.z);
        points[3] = V3(origin.x + scaling * y_axis.x, 
					   origin.y + scaling * y_axis.y,
					   origin.z + scaling * y_axis.z);
        points[4] = V3(origin.x + 1.5 * scaling * z_axis.x,
                       origin.y + 1.5 * scaling * z_axis.y,
                       origin.z + 1.5 * scaling * z_axis.z);

		// start to draw the lines
		glBegin(GL_LINES);

		glColor3f(255, 255, 0); // make lines yellow as shown in tutorial

		// iterate through the above set up points and draw the lines of it, 
		// so that we end up on the same handle as shown in the tutorial for programming Assignment 2
		for (int i = 1; i <= 4; i++)
		{
			for (int j = i + 1; j <= 4; j++)
			{
				glVertex3f(points[i].x, points[i].y, points[i].z);
				glVertex3f(points[j].x, points[j].y, points[j].z);
			}
			glVertex3f(points[0].x, points[0].y, points[0].z);
			glVertex3f(points[i].x, points[i].y, points[i].z);
		}

		glEnd();


		////////////////////////////////////////

}




//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////// NO CHANGE IS NEEDED BELOW THIS LINE /////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////




CRayView::CRayView()
{
	m_nZoomFactor=4;
	m_bElevate=FALSE;
	m_bMove=FALSE;	
	m_bTwist=FALSE;
	m_bSlide=FALSE;	
	m_bRotateObj=FALSE;
	m_bMoveObj=FALSE;
	m_bRotateView=FALSE;
	m_bMoveLight=FALSE;
	m_bAntialiasing=FALSE;	
	m_bShowTexture=TRUE;	
	m_bBlending=TRUE;

	//FindCorners();
	
	m_bLButDown=FALSE;
	m_bRButDown=FALSE;
	m_nLeft=100;
	m_nTop=100;

	m_nCurrentMode=MODE_NAVIGATE;
	m_bWinSelect=FALSE;

}

CRayView::~CRayView()
{
	delete m_pDC;
}

BOOL CRayView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CRayView drawing

void CRayView::OnDraw(CDC* pDC)
{
	CRayDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CRayView diagnostics

#ifdef _DEBUG
void CRayView::AssertValid() const
{
	CView::AssertValid();
}

void CRayView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CRayDoc* CRayView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRayDoc)));
	return (CRayDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRayView message handlers

BOOL CRayView::bSetupPixelFormat()
{
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
		1,                              // version number
		PFD_DRAW_TO_WINDOW |            // support window
		  PFD_SUPPORT_OPENGL |          // support OpenGL
		  PFD_DOUBLEBUFFER,             // double buffered
		PFD_TYPE_RGBA,                  // RGBA type
		24,                             // 24-bit color depth
		0, 0, 0, 0, 0, 0,               // color bits ignored
		0,                              // no alpha buffer
		0,                              // shift bit ignored
		0,                              // no accumulation buffer
		0, 0, 0, 0,                     // accum bits ignored
		32,                             // 32-bit z-buffer
		0,                              // no stencil buffer
		0,                              // no auxiliary buffer
		PFD_MAIN_PLANE,                 // main layer
		0,                              // reserved
		0, 0, 0                         // layer masks ignored
	};
	int pixelformat;

	if ( (pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0 )
	{
		MessageBox("ChoosePixelFormat failed");
		return FALSE;
	}

	if (SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
	{
		MessageBox("SetPixelFormat failed");
		return FALSE;
	}

	return TRUE;
}

void CRayView::DrawScene()
{
	CRayDoc *pDoc=GetDocument();

	for(list<LPLIGHTSOURCE>::iterator i=pDoc->m_lightsources.begin(); i!=pDoc->m_lightsources.end(); i++) (*i)->Draw();

	for(list<LPPRIMITIVE>::iterator i=pDoc->m_objects.begin(); i!=pDoc->m_objects.end(); i++){
		if (m_bShowTexture==TRUE) (*i)->Draw(1,pDoc->m_selectedobjects);
		else (*i)->Draw(0,pDoc->m_selectedobjects);
	}


	DrawBoundingBox();
}

void CRayView::Init()
{
	m_pDC = new CClientDC(this);

	ASSERT(m_pDC != NULL);

	if (!bSetupPixelFormat())
		return;

	m_hRC = wglCreateContext(m_pDC->GetSafeHdc());
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glLineStipple(2,0xAAAA);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glReadBuffer(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int CRayView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	Init(); // initialize OpenGL
	
	SetTimer(101,0,NULL);

	return 0;
}

BOOL CRayView::OnEraseBkgnd(CDC* pDC) 
{
	//default return added by ide
	//return CView::OnEraseBkgnd(pDC);
	return TRUE;
}

void CRayView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	m_nWidth=cx;
	m_nHeight=cy;

	if (m_nWidth<1) m_nWidth=1;
	if (m_nHeight<1) m_nHeight=1;	
	
	glViewport(0, 0, cx, cy);
/*
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-.5,cx-.5,-.5,cy-.5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
*/
}

void CRayView::FindCorners()
{
	CRayDoc *pDoc=GetDocument();
	ASSERT_VALID(pDoc);
	
	float x,y;
	V3 RHV,HUV,LOS,VP,c;

	VP=pDoc->m_camera.viewpoint;
	LOS=pDoc->m_camera.los;
	RHV=pDoc->m_camera.rhv;
	HUV=pDoc->m_camera.huv;

	y=0.2f*(float)tan(pDoc->m_camera.viewangle*M_PI/360.0f);
	x=y*m_nWidth/m_nHeight;

	c=VP+LOS*.2f;
	m_viewinfo.llp=c-RHV*x-HUV*y;
	m_viewinfo.lrp=c+RHV*x-HUV*y;
	m_viewinfo.ulp=c-RHV*x+HUV*y;
	m_viewinfo.urp=c+RHV*x+HUV*y;
}

void CRayView::DrawSelBox()
{
	CRayDoc *pDoc=GetDocument();

	int i;
	V3 fv[3],tv[6];

	glDisable(GL_LIGHTING);
	glColor3f(.0784f,.3922f,.549f);

	GLboolean bBlend;
	glGetBooleanv(GL_BLEND,&bBlend);
	if (bBlend){
		glLineWidth(1.5);
	} else {
		glLineWidth(1.0);
	}

	for (i=0;i<12;i++){
		glLoadName(60000+i);
		glBegin(GL_LINES);
		glVertex3fv(pDoc->m_BigBoxCorner[boxedge[i][0]]);
		glVertex3fv(pDoc->m_BigBoxCorner[boxedge[i][1]]);
		glEnd();
	}

	glColor3f(.3f,.3f,.5f);
	for (i=0;i<6;i++){
		glLoadName(60012+i);
		tv[0]=(pDoc->m_BigBoxCorner[boxface[i][0]]+pDoc->m_BigBoxCorner[boxface[i][1]]+pDoc->m_BigBoxCorner[boxface[i][2]]+pDoc->m_BigBoxCorner[boxface[i][3]])/4.0f;
		fv[0]=pDoc->m_BigBoxCorner[boxface[i][0]]-pDoc->m_BigBoxCorner[boxface[i][4]];
		fv[1]=pDoc->m_BigBoxCorner[boxface[i][1]]-pDoc->m_BigBoxCorner[boxface[i][0]];
		fv[2]=pDoc->m_BigBoxCorner[boxface[i][3]]-pDoc->m_BigBoxCorner[boxface[i][0]];

		UserDrawControlHandle(tv[0],  fv[2], fv[1], fv[0]);


	}

	glLineWidth(1.0);
	glLoadName(-1);
	glEnable(GL_LIGHTING);
}

void CRayView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRayDoc *pDoc=GetDocument();
	int hitid;
	int edgeno;

	if (nFlags & MK_LBUTTON){
		m_bLButDown=TRUE;
		m_nSx=point.x;
		m_nSy=m_nHeight-1-point.y;
		m_nCx=m_nPx=m_nSx;
		m_nCy=m_nPy=m_nSy;		
		
		switch(m_nCurrentMode){
		case MODE_NAVIGATE:
			//hitid=PickObject(m_nSx,m_nSy);
			//if (hitid<1000 && hitid>=0){		// picked an object
			//	this->m_nCurrentMode=MODE_SELECT;
			//	bool clickagain=false;
			//	for(list<LPPRIMITIVE>::iterator i=pDoc->m_objects.begin(); i!=pDoc->m_objects.end(); i++){
			//		if ((*i)->m_nID==hitid){
			//			if (nFlags & MK_SHIFT) {	// if shift is pressed, select one more object
			//				if ((*i)->IsSelected()==FALSE){
			//					pDoc->SelectOneMore((void*)(*i));
			//				} else {
			//					pDoc->UnSelectOne((void*)(*i));	// if shift is pressed and the picked object is already selected, unselect the object
			//				}
			//			} else {
			//				if ((*i)->IsSelected()==FALSE){	// if no shift, select one only
			//					pDoc->SelectOne((void*)(*i));
			//				} else {
			//					clickagain=TRUE;	
			//				}
			//			}
			//			break;
			//		}
			//	}
			//	if (clickagain==TRUE){		// rotate selected objects
			//		UserLeftButtonDown(hitid, m_nSx, m_nSy);
			//		pDoc->GenerateBigBoundingBox();
			//		pDoc->SetModifiedFlag(TRUE);
			//	}
			//} else{
				if (nFlags & MK_SHIFT){
					m_bSlide=TRUE;
				} else {
					m_bMove=TRUE;
				}
			//}
			m_nDx=0;
			m_nDy=0;
			m_viewinfo.sx1=m_viewinfo.sx2=m_nSx;
			m_viewinfo.sy1=m_viewinfo.sy2=m_nSy;
			Invalidate();
			break;

		case MODE_SELECT:						//select mode
			hitid=PickObject(m_nSx,m_nSy);
			if (hitid!=-1){
				if (hitid<1000 && hitid>=0){		// picked an object
					bool clickagain=false;
					for(list<LPPRIMITIVE>::iterator i=pDoc->m_objects.begin(); i!=pDoc->m_objects.end(); i++){
						if ((*i)->m_nID==hitid){
							if (nFlags & MK_SHIFT) {	// if shift is pressed, select one more object
								if ((*i)->IsSelected()==FALSE){
									pDoc->SelectOneMore((void*)(*i));
								} else {
									pDoc->UnSelectOne((void*)(*i));	// if shift is pressed and the picked object is already selected, unselect the object
								}
							} else {
								if ((*i)->IsSelected()==FALSE){	// if no shift, select one only
									pDoc->SelectOne((void*)(*i));
								} else {
									clickagain=TRUE;	
								}
							}
							break;
						}
					}
					if (clickagain==TRUE){		// rotate selected objects
						UserLeftButtonDown(hitid, m_nSx, m_nSy);
						pDoc->GenerateBigBoundingBox();
						pDoc->SetModifiedFlag(TRUE);
					}
				} else
				if (hitid<10000 && hitid>=1000){	// pick light source
					pDoc->UnSelectAll();
					pDoc->SelectLightWithID(hitid-1000);
				} else
				if (hitid>=60018){		// move light
					m_bMoveLight=TRUE;
					edgeno=(hitid-60018)%3;

					for(list<LPLIGHTSOURCE>::iterator i=pDoc->m_lightsources.begin(); i!=pDoc->m_lightsources.end(); i++){
						if ((*i)->IsSelected()==TRUE){
							rotcenter=(*i)->position;
							switch(edgeno){
							case 0:
								m1=(*i)->position - V3(10,0,0);
								m2=(*i)->position + V3(10,0,0);
								break;
							case 1:
								m1=(*i)->position - V3(0,10,0);
								m2=(*i)->position + V3(0,10,0);
								break;
							case 2:
								m1=(*i)->position - V3(0,0,10);
								m2=(*i)->position + V3(0,0,10);
								break;
							}
						}
					}
				}

				UserLeftButtonDown(hitid, m_nSx, m_nSy);

			} else {
				if (nFlags & MK_SHIFT){
				} else {		// nothing is picked, go to window/group select mode
					pDoc->UnSelectAll();
					
					m_bWinSelect=TRUE;
					m_viewinfo.sx1=m_viewinfo.sx2=m_nSx;
					m_viewinfo.sy1=m_viewinfo.sy2=m_nSy;
				}
			}
			Invalidate();
			break;
		}
	} 
	
	CView::OnLButtonDown(nFlags, point);
}

int CRayView::PickObject(int mx, int my)
{
	CRayDoc *pDoc=GetDocument();

	GLint	viewport[4];
	GLuint	selectBuf[500],*tmpuint,minz,minzid;
	GLint hits;
	int sid=0;	

	glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(500, selectBuf);

	glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(-1);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix((GLdouble) mx, (GLdouble)my, 5,5,viewport);
	gluPerspective(pDoc->m_camera.viewangle,(float)m_nWidth/m_nHeight,.199,1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(pDoc->m_camera.viewpoint[0],pDoc->m_camera.viewpoint[1],pDoc->m_camera.viewpoint[2],
		pDoc->m_camera.viewpoint[0]+pDoc->m_camera.los[0]*10.0,pDoc->m_camera.viewpoint[1]+pDoc->m_camera.los[1]*10.0,pDoc->m_camera.viewpoint[2]+pDoc->m_camera.los[2]*10.0,
		pDoc->m_camera.huv[0],pDoc->m_camera.huv[1],pDoc->m_camera.huv[2]);

	DrawScene();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	hits=glRenderMode(GL_RENDER);
	tmpuint=selectBuf;

	minz=0xFFFFFFFF;
	minzid=1000000;

	if (hits==0) sid=-1;
	else{
		for (int i=0;i<hits-1;i++){
			if (*(tmpuint+1)<=minz){
				minz=*(tmpuint+1);
				minzid=*(tmpuint+3);
			}
			tmpuint+=4;
		}
		if (sid==0){
			if (*(tmpuint+1)<=minz){
				minz=*(tmpuint+1);
				minzid=*(tmpuint+3);
			}
			tmpuint+=3;
			sid=minzid;
		}
	}
	return sid;	
}

void CRayView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRayDoc *pDoc=GetDocument();

	V3 hv,vv,sp,cp,mm,ra,tmpv1,tmpv2,int1,int2, p1, p2;
	float t1,t2,dp1,dp2;

	if (m_bRButDown==TRUE){
		m_nRCx=point.x;
		m_nRCy=m_nHeight-1-point.y;
		if (m_nRCx>=m_nWidth) m_nRCx=m_nWidth-1;
		if (m_nRCx<0) m_nRCx=0;
		if (m_nRCy>=m_nHeight) m_nRCy=m_nHeight-1;
		if (m_nRCy<0) m_nRCy=0;
		if (m_nCx!=m_nPx || m_nCy!=m_nPy){
			if (m_bElevate==TRUE){
				pDoc->ViewpointElevate(m_nRSx,m_nRCx,m_nRSy,m_nRCy);
			}
			if (m_bTwist==TRUE){
				pDoc->ViewpointTwist(m_nRSx,m_nRCx);
			}
		}

		UserMouseMove(m_nRCx, m_nRCy);
		pDoc->GenerateBigBoundingBox();
		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
	}
	if (m_bLButDown==TRUE){
		m_nCx=point.x;
		m_nCy=m_nHeight-1-point.y;
		if (m_nCx>=m_nWidth) m_nCx=m_nWidth-1;
		if (m_nCx<0) m_nCx=0;
		if (m_nCy>=m_nHeight) m_nCy=m_nHeight-1;
		if (m_nCy<0) m_nCy=0;
		m_nDx=m_nCx-m_nPx;
		m_nDy=m_nCy-m_nPy;
		if (m_nCx!=m_nPx || m_nCy!=m_nPy){
			if (m_bMove==TRUE){
				pDoc->ViewpointWalk(m_nSx,m_nCx,m_nSy,m_nCy);
			}

			if (m_bSlide==TRUE){
				pDoc->ViewpointSlide(m_nSx,m_nCx,m_nSy,m_nCy);
			}


			if (m_bMoveLight==TRUE){
				if (m_nCx!=m_nSx || m_nCy!=m_nSy){

					hv=(m_viewinfo.lrp-m_viewinfo.llp)/(float)m_nWidth;
					vv=(m_viewinfo.ulp-m_viewinfo.llp)/(float)m_nHeight;

					sp=m_viewinfo.llp+hv*(float)m_nSx+vv*(float)m_nSy;
					cp=m_viewinfo.llp+hv*(float)m_nCx+vv*(float)m_nCy;

					tmpv1=cp-pDoc->m_camera.viewpoint;					
					tmpv1.normalize();
					
					tmpv2=sp-pDoc->m_camera.viewpoint;
					tmpv2.normalize();
					
					mm=m2-m1;					
					mm.normalize();
					
					p1=mm.cross(tmpv1);
					p2=tmpv1.cross(p1);
					p2.normalize();

					dp2=p2.dot(pDoc->m_camera.viewpoint);

					t2=(dp2-p2[0]*m1[0]-p2[1]*m1[1]-p2[2]*m1[2])/(p2[0]*mm[0]+p2[1]*mm[1]+p2[2]*mm[2]);
					int2=m1+t2*mm;					

					p2=mm.cross(tmpv2);
					p1=tmpv2.cross(p2);
					p1.normalize();

					dp1=p1.dot(pDoc->m_camera.viewpoint);

					t1=(dp1-p1[0]*m1[0]-p1[1]*m1[1]-p1[2]*m1[2])/(p1[0]*mm[0]+p1[1]*mm[1]+p1[2]*mm[2]);
					int1=m1+t1*mm;					

					for(list<LPLIGHTSOURCE>::iterator i=pDoc->m_lightsources.begin(); i!=pDoc->m_lightsources.end(); i++){
						if ((*i)->IsSelected()==TRUE){
							(*i)->position=rotcenter+int2-int1;
						}
					}
					pDoc->SetModifiedFlag(TRUE);
					Invalidate();
				} else {
					for(list<LPLIGHTSOURCE>::iterator i=pDoc->m_lightsources.begin(); i!=pDoc->m_lightsources.end(); i++){
						if ((*i)->IsSelected()==TRUE){
							(*i)->position=rotcenter;
						}

					}
				}
				
			}

			if (m_bWinSelect==TRUE){
				m_viewinfo.sx2=m_nCx;
				m_viewinfo.sy2=m_nCy;
				Invalidate();
			}
			m_nPx=m_nCx;
			m_nPy=m_nCy;

			UserMouseMove(m_nCx, m_nCy);
			pDoc->GenerateBigBoundingBox();
			pDoc->SetModifiedFlag(TRUE);
			Invalidate();
		}
	}

	if (m_nCurrentMode==MODE_ZOOM){
		m_nCx=point.x;
		m_nCy=m_nHeight-1-point.y;
		Invalidate();
	}
	CView::OnMouseMove(nFlags, point);
}

void CRayView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CRayDoc *pDoc=GetDocument();

	if (m_bLButDown==TRUE){
		m_bLButDown=FALSE;
		m_nEx=point.x;
		m_nEy=m_nHeight-1-point.y;

		if (m_nEx==m_nSx || m_nEy==m_nSy){
			int hitid=PickObject(m_nEx,m_nEy);
			if (hitid<1000 && hitid>=0){		// picked an object
				this->m_nCurrentMode=MODE_SELECT;
				bool clickagain=false;
				for(list<LPPRIMITIVE>::iterator i=pDoc->m_objects.begin(); i!=pDoc->m_objects.end(); i++){
					if ((*i)->m_nID==hitid){
						if (nFlags & MK_SHIFT) {	// if shift is pressed, select one more object
							if ((*i)->IsSelected()==FALSE){
								pDoc->SelectOneMore((void*)(*i));
							} else {
								pDoc->UnSelectOne((void*)(*i));	// if shift is pressed and the picked object is already selected, unselect the object
							}
						} else {
							if ((*i)->IsSelected()==FALSE){	// if no shift, select one only
								pDoc->SelectOne((void*)(*i));
							}
						}
						break;
					}
				}
			}
		}


		if (m_bMove==TRUE){
			m_bMove=FALSE;
			pDoc->ViewpointWalk(m_nSx,m_nEx,m_nSy,m_nEy);
		}
		if (m_bSlide==TRUE){
			m_bSlide=FALSE;
			pDoc->ViewpointSlide(m_nSx,m_nEx,m_nSy,m_nEy);
		}

		if (m_bWinSelect==TRUE){
			m_bWinSelect=FALSE;
			m_viewinfo.sx2=m_nEx;
			m_viewinfo.sy2=m_nEy;
			if (m_nSx!=m_nEx || m_nSy!=m_nEy) WindowSelect(m_viewinfo.sx1,m_viewinfo.sy1,m_viewinfo.sx2,m_viewinfo.sy2);
			else {
				m_nCurrentMode=MODE_NAVIGATE;
			}
			Invalidate();

		}
		
		if (m_bRotateObj==TRUE){
			m_bRotateObj=FALSE;
			Invalidate();
		}

		if (m_bResize==TRUE){
			m_bResize=FALSE;
			Invalidate();
		}

		if (m_bMoveObj==TRUE){
			m_bMoveObj=FALSE;
			Invalidate();
		}

		if (m_bMoveLight==TRUE){
			m_bMoveLight=FALSE;
			Invalidate();
		}

		UserLeftButtonUp(m_nEx, m_nEy);
		pDoc->GenerateBigBoundingBox();
		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
	}
	
	CView::OnLButtonUp(nFlags, point);
}

void CRayView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CRayDoc *pDoc=GetDocument();

	if (m_bLButDown==TRUE){	// does not respond with left button is down
		return;
	}

	if (nFlags & MK_RBUTTON){
		m_bRButDown=TRUE;
		m_nSx=m_nRSx=point.x;
		m_nSy=m_nRSy=m_nHeight-1-point.y;
		m_nRCx=m_nRPx=m_nRSx;
		m_nRCy=m_nRPy=m_nRSy;		

		int hitid=PickObject(m_nSx,m_nSy);

		switch(m_nCurrentMode){
		case MODE_NAVIGATE:
			if (nFlags & MK_SHIFT){
				m_bTwist=TRUE;
			} else {
				m_bElevate=TRUE;
			}
			break;
		case MODE_SELECT:
			if (hitid<1000 && hitid>=0){		// picked an object
				UserRightButtonDown(hitid, m_nRSx, m_nRSy);
			} else 
			if (hitid<10000 && hitid<60000){		// picked a user drawn handle
				UserRightButtonDown(hitid, m_nRSx, m_nRSy);
			}
			break;
		}

		pDoc->GenerateBigBoundingBox();
		pDoc->SetModifiedFlag(TRUE);
	}

	CView::OnRButtonDown(nFlags, point);
}

void CRayView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CRayDoc *pDoc=GetDocument();

	if (m_bRButDown==TRUE){
		m_bRButDown=FALSE;
		m_nREx=point.x;
		m_nREy=m_nHeight-1-point.y;

		if (m_bElevate==TRUE){
			m_bElevate=FALSE;
			pDoc->ViewpointElevate(m_nRSx,m_nREx,m_nRSy,m_nREy);
		}
		if (m_bTwist==TRUE){
			m_bTwist=FALSE;
			pDoc->ViewpointTwist(m_nRSx,m_nREx);
		}
		if (m_bRotateView==TRUE){
			m_bRotateView=FALSE;
			Invalidate();
		}

		UserRightButtonUp(m_nREx, m_nREy);
		pDoc->GenerateBigBoundingBox();
		pDoc->SetModifiedFlag(TRUE);

	}
	CView::OnRButtonUp(nFlags, point);
}

void CRayView::BackupViewpoint()
{
	CRayDoc *pDoc=GetDocument();

	m_oldcamera=pDoc->m_camera;
	m_oldviewinfo=m_viewinfo;
}

void CRayView::RestoreViewpoint()
{
	CRayDoc *pDoc=GetDocument();

	pDoc->m_camera=m_oldcamera;
	m_viewinfo=m_oldviewinfo;
}

void CRayView::WindowSelect(int x1,int y1,int x2,int y2)
{
	CRayDoc *pDoc=GetDocument();

	GLint	viewport[4];
	GLuint	selectBuf[500],*tmpuint;
	GLint hits;
	int sid=0;
	int i,mx,my,dx,dy;
		
	if (x1==x2 && y1==y2){
		return;
	}

	mx=(x1+x2)/2;
	my=(y1+y2)/2;
	dx=abs(x1-x2);
	dy=abs(y1-y2);


	glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(500, selectBuf);

	glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(-1);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix((GLdouble) mx, (GLdouble)my, dx,dy,viewport);
	gluPerspective(pDoc->m_camera.viewangle,(float)m_nWidth/m_nHeight,.199,1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(pDoc->m_camera.viewpoint[0],pDoc->m_camera.viewpoint[1],pDoc->m_camera.viewpoint[2],
		pDoc->m_camera.viewpoint[0]+pDoc->m_camera.los[0]*10.0,pDoc->m_camera.viewpoint[1]+pDoc->m_camera.los[1]*10.0,pDoc->m_camera.viewpoint[2]+pDoc->m_camera.los[2]*10.0,
		pDoc->m_camera.huv[0],pDoc->m_camera.huv[1],pDoc->m_camera.huv[2]);

	DrawScene();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	hits=glRenderMode(GL_RENDER);
	tmpuint=selectBuf;

	if (hits==0) sid=-1;
	else{
		for (i=0;i<hits;i++){
			if (*(tmpuint+3)>0 && *(tmpuint+3)<1000){
				for(list<LPPRIMITIVE>::iterator i=pDoc->m_objects.begin(); i!=pDoc->m_objects.end(); i++){
					if ((*i)->m_nID==(int)(*(tmpuint+3))){
						pDoc->SelectOneMore((void*)(*i));
						break;
					}
				}
			}

			tmpuint+=4;
		}
	}	
}

void CRayView::OnTimer(UINT nIDEvent) 
{
	CRayDoc *pDoc=GetDocument();

	if (nIDEvent==101){
		if (m_bMove==TRUE){
			pDoc->ViewpointWalk(m_nSx,m_nCx,m_nSy,m_nCy);
		}
		if (m_bElevate==TRUE){
			pDoc->ViewpointElevate(m_nRSx,m_nRCx,m_nRSy,m_nRCy);
		}
		if (m_bSlide==TRUE){
			pDoc->ViewpointSlide(m_nSx,m_nCx,m_nSy,m_nCy);
		}
		if (m_bTwist==TRUE){
			pDoc->ViewpointTwist(m_nRSx,m_nRCx);
		}
	}
	CView::OnTimer(nIDEvent);
}

void CRayView::OnPaint() 
{
	CRayDoc *pDoc=GetDocument();

	CPaintDC dc(this); // device context for painting
	
	PAINTSTRUCT ps;
	RECT rc;

	// check if the WM_PAINT message is for this window
    BeginPaint(&ps);
	GetClientRect(&rc); 

	if ((ps.rcPaint.left>rc.right) && (ps.rcPaint.right<rc.left) && (ps.rcPaint.top>rc.bottom) && (ps.rcPaint.bottom<rc.top)) return; 
	EndPaint(&ps); 

	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

    glViewport(0, 0, m_nWidth, m_nHeight);

	glClearColor(.0f,.0f,.0f,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(pDoc->m_camera.viewangle,(float)m_nWidth/m_nHeight,.199,500);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	FindCorners();

	gluLookAt(pDoc->m_camera.viewpoint[0],pDoc->m_camera.viewpoint[1],pDoc->m_camera.viewpoint[2],
		pDoc->m_camera.viewpoint[0]+pDoc->m_camera.los[0]*10.0,pDoc->m_camera.viewpoint[1]+pDoc->m_camera.los[1]*10.0,pDoc->m_camera.viewpoint[2]+pDoc->m_camera.los[2]*10.0,
		pDoc->m_camera.huv[0],pDoc->m_camera.huv[1],pDoc->m_camera.huv[2]);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,0);

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	if (m_bBlending) {
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(.5);
	} else {
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
		glLineWidth(1.0);
	}

	float flWhite[4]={1.0,1.0,1.0,1.0};
	
	int i=0;
	for(list<LPLIGHTSOURCE>::iterator ls=pDoc->m_lightsources.begin(); ls!=pDoc->m_lightsources.end(); ls++){
		float flLightPos[4];
		flLightPos[0]=(*ls)->position.x;
		flLightPos[1]=(*ls)->position.y;
		flLightPos[2]=(*ls)->position.z;
		flLightPos[3]=1.0;
		glLightfv(GL_LIGHT0+i,GL_POSITION,flLightPos);

		float flColor[4];
		flColor[0]=(*ls)->color.vec[0];
		flColor[1]=(*ls)->color.vec[1];
		flColor[2]=(*ls)->color.vec[2];
		flColor[3]=1.0;
		glLightfv(GL_LIGHT0+i,GL_DIFFUSE,flColor);
		glLightfv(GL_LIGHT0+i,GL_SPECULAR,flColor);
		glLightfv(GL_LIGHT0+i,GL_AMBIENT,flColor);

		if ((*ls)->m_bON==TRUE){
			glEnable(GL_LIGHT0+i);
		} else {
			glDisable(GL_LIGHT0+i);
		}
		i++;
	}

	if (i<8){
		for (int j=i;j<8;j++){
			glDisable(GL_LIGHT0+j);
		}
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawScene();


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(-.5,m_nWidth-.5,-.5,m_nHeight-.5);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (m_bMove==TRUE || m_bElevate==TRUE || m_bSlide==TRUE || m_bTwist==TRUE){
		glDisable(GL_LIGHTING);
		glColor3f(1.0,1.0,1.0);
		glBegin(GL_LINE_LOOP);
			glVertex2i(m_nSx,m_nSy+2);
			glVertex2i(m_nSx-2,m_nSy-2);
			glVertex2i(m_nSx+2,m_nSy-2);
		glEnd();
		glEnable(GL_LIGHTING);
	}

	if (m_nCurrentMode==MODE_ZOOM && m_bWinSelect==FALSE){
		int nZoomDim=30,i,j;
		unsigned char *pBuf=new unsigned char[nZoomDim*nZoomDim*3];
		unsigned char *pZBuf=new unsigned char[nZoomDim*nZoomDim*3*m_nZoomFactor*m_nZoomFactor];
		
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glPixelStorei(GL_PACK_ALIGNMENT,1);
		glReadPixels(m_nCx-nZoomDim/2,m_nCy-nZoomDim/2,nZoomDim,nZoomDim,GL_RGB,GL_UNSIGNED_BYTE,pBuf);
		
		int nF=nZoomDim*m_nZoomFactor;
		int nF2,nF3;
		
		for (i=0;i<nF;i++){		//non-antialiased resize by m_nZoomFactor times.
			for (j=0;j<nF;j++){
				nF2=i*nF*3+j*3;
				nF3=(i/m_nZoomFactor)*nZoomDim*3+(j/m_nZoomFactor)*3;
				//nF3=nF2/nZoomFactor;
				pZBuf[nF2]=pBuf[nF3];
				pZBuf[nF2+1]=pBuf[nF3+1];
				pZBuf[nF2+2]=pBuf[nF3+2];
			}
		}
		
		int xoff=0,yoff=0;

		if (m_nCx+nZoomDim/2+nF+10>=m_nWidth) xoff=-(nZoomDim+nF+20);
		if (m_nCy-nZoomDim/2-1-nF-10<=0) yoff=(nZoomDim+nF+20);

		glRasterPos2i(m_nCx+nZoomDim/2+10+xoff,m_nCy-nZoomDim/2-nF-10+yoff);
		glDepthMask(GL_FALSE);
		glPixelStorei(GL_UNPACK_ALIGNMENT ,4);
		glDrawPixels(nF,nF,GL_RGB,GL_UNSIGNED_BYTE,pZBuf);
		
		glDisable(GL_LIGHTING);
		glBegin(GL_LINE_LOOP);
			glColor4f(1.0f,1.0f,1.0f,.5);
			glVertex2i(m_nCx+nZoomDim/2-1+10+xoff,m_nCy-nZoomDim/2-1-nF-10+yoff);
			glVertex2i(m_nCx+nZoomDim/2+nF+10+xoff,m_nCy-nZoomDim/2-1-nF-10+yoff);
			glVertex2i(m_nCx+nZoomDim/2+nF+10+xoff,m_nCy-nZoomDim/2-10+yoff);
			glVertex2i(m_nCx+nZoomDim/2-1+10+xoff,m_nCy-nZoomDim/2-10+yoff);
		glEnd();
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINE_LOOP);
			glColor4f(1.0f,1.0f,1.0f,.5);
			glVertex2i(m_nCx-nZoomDim/2,m_nCy-nZoomDim/2);
			glVertex2i(m_nCx+nZoomDim/2,m_nCy-nZoomDim/2);
			glVertex2i(m_nCx+nZoomDim/2,m_nCy+nZoomDim/2);
			glVertex2i(m_nCx-nZoomDim/2,m_nCy+nZoomDim/2);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
		glEnable(GL_LIGHTING);
		glDepthMask(GL_TRUE);

		delete pBuf;
		delete pZBuf;
	}
	
	if (m_bWinSelect==TRUE ){		// window/group select mode
		glDisable(GL_LIGHTING);
		glColor3f(1.0,1.0,1.0);
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINE_LOOP);
			glVertex2i(m_viewinfo.sx1,m_viewinfo.sy1);
			glVertex2i(m_viewinfo.sx1,m_viewinfo.sy2);
			glVertex2i(m_viewinfo.sx2,m_viewinfo.sy2);
			glVertex2i(m_viewinfo.sx2,m_viewinfo.sy1);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
		glEnable(GL_LIGHTING);
	}

	glDisable(GL_BLEND);
	glFlush();
	SwapBuffers(m_hDC);
	
	glFlush();
	//glFinish();
	SwapBuffers(m_pDC->GetSafeHdc());
}

void CRayView::OnSelect() 
{
	CMenu *mmenu;
	int mid;

	if (m_nCurrentMode==MODE_ZOOM) Invalidate();
	m_nCurrentMode=MODE_SELECT;
	mmenu=theApp.m_pMainWnd->GetMenu();
	mmenu=mmenu->GetSubMenu(3);
	mid=mmenu->GetMenuItemID(0);
	mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	mid=mmenu->GetMenuItemID(1);
	mmenu->CheckMenuItem(mid,MF_CHECKED);
	mid=mmenu->GetMenuItemID(2);
	mmenu->CheckMenuItem(mid,MF_UNCHECKED);
}

void CRayView::OnNavigate() 
{
	CMenu *mmenu;
	int mid;

	if (m_nCurrentMode==MODE_ZOOM) Invalidate();
	m_nCurrentMode=MODE_NAVIGATE;
	mmenu=theApp.m_pMainWnd->GetMenu();
	mmenu=mmenu->GetSubMenu(3);
	mid=mmenu->GetMenuItemID(0);
	mmenu->CheckMenuItem(mid,MF_CHECKED);
	mid=mmenu->GetMenuItemID(1);
	mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	mid=mmenu->GetMenuItemID(2);
	mmenu->CheckMenuItem(mid,MF_UNCHECKED);
}

void CRayView::OnUpdateSelect(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nCurrentMode==MODE_SELECT);
	
}

void CRayView::OnUpdateNavigate(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nCurrentMode==MODE_NAVIGATE);
	
}

void CRayView::OnAntialiasing() 
{
	CMenu *mmenu;
	int mid;

	mmenu=theApp.m_pMainWnd->GetMenu();
	mmenu=mmenu->GetSubMenu(3);
	mid=mmenu->GetMenuItemID(3);
	if (m_bAntialiasing==TRUE){
		m_bAntialiasing=FALSE;
		mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	} else {
		m_bAntialiasing=TRUE;
		mmenu->CheckMenuItem(mid,MF_CHECKED);
	}
}

void CRayView::OnUpdateAntialiasing(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bAntialiasing);
}

void CRayView::OnMiscmat() 
{
	CRayDoc *pDoc=GetDocument();
	CMaterial *materialdialog=NULL;

	float data[13];

	pDoc->SelectedObjectsMiscMaterialProp(data);

	materialdialog=new CMaterial(this,data);
	if (materialdialog->DoModal()==IDOK){
		data[0]=(float)materialdialog->m_nAmbient[0]/255;
		data[1]=(float)materialdialog->m_nAmbient[1]/255;
		data[2]=(float)materialdialog->m_nAmbient[2]/255;
		data[3]=(float)materialdialog->m_nDiffuse[0]/255;
		data[4]=(float)materialdialog->m_nDiffuse[1]/255;
		data[5]=(float)materialdialog->m_nDiffuse[2]/255;
		data[6]=(float)materialdialog->m_nSpecular[0]/255;
		data[7]=(float)materialdialog->m_nSpecular[1]/255;
		data[8]=(float)materialdialog->m_nSpecular[2]/255;
		data[9]=(float)materialdialog->m_nShininess;
		data[10]=(float)materialdialog->m_nRefractiveIndex/100;
		data[11]=(float)materialdialog->m_nReflectance/100;
		data[12]=(float)materialdialog->m_nOpacity/100;

		pDoc->MiscMaterialProp(data);

	}
		
	delete materialdialog;
	materialdialog=NULL;
}

void CRayView::OnUserNewDoc()
{
	CRayDoc *pDoc=GetDocument();

	pDoc->m_selectedobjects.clear();	
}

void CRayView::OnPreferences() 
{
	CRayDoc *pDoc=GetDocument();
	CPreferencesDialog *prefdialog=NULL;

	if (prefdialog==NULL){
		prefdialog=new CPreferencesDialog(this,m_bShowTexture,m_nZoomFactor/2,(int)pDoc->m_camera.viewangle,m_bBlending);
		if (prefdialog->DoModal()==IDOK){			
			m_bShowTexture=(prefdialog->m_bShowTexture==TRUE);
			m_nZoomFactor=prefdialog->m_nZoomFactor*2;
			m_bBlending=(prefdialog->m_bBlending==TRUE);
			pDoc->m_camera.viewangle=(float)prefdialog->m_nViewAngle;
		}
		delete prefdialog;
		prefdialog=NULL;
	}
	pDoc->SetModifiedFlag(TRUE);
	Invalidate();
}

void CRayView::OnImportTx() 
{
	CRayDoc *pDoc=GetDocument();
	CFileDialog *fdlg;
	int cnt;
	CTexture *t,*rt;

	cnt=pDoc->m_selectedobjects.size();
	if (cnt==0) return;

	fdlg=new CFileDialog(TRUE,".jpg",NULL,OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,"JPEG Files (*.jpg)|*.jpg|Windows Bitmaps (*.bmp)|*.bmp||",this);

	fdlg->m_ofn.lpstrTitle="Import texture";

	if (fdlg->DoModal()==IDOK){
		if (fdlg->GetPathName().Right(3).CompareNoCase("jpg")==0 || fdlg->GetPathName().Right(3).CompareNoCase("bmp")==0 ){
			rt=NULL;
			cnt=pDoc->m_textures.size();
			for(list<LPTEXTURE>::iterator ti=pDoc->m_textures.begin(); ti!=pDoc->m_textures.end(); ti++){
				if (fdlg->GetPathName().CompareNoCase((*ti)->m_strFilename)==0){
					rt=(*ti);
					break;
				}
			}

			if (rt==NULL){
				t=new CTexture(fdlg->GetPathName());
				if (t->Read(fdlg->GetPathName())==1){
					AfxMessageBox("Cannot open texture file",MB_OK | MB_ICONEXCLAMATION);
					return;
				} else {
					pDoc->m_textures.push_back(t);
					rt=t;
				}
			}

			cnt=pDoc->m_selectedobjects.size();
			for(list<LPPRIMITIVE>::iterator oi=pDoc->m_selectedobjects.begin(); oi!=pDoc->m_selectedobjects.end(); oi++){
				(*oi)->m_pTX=rt;
			}
			pDoc->SetModifiedFlag(TRUE);
			Invalidate();
		}
	}
	delete fdlg;
}

void CRayView::OnZoom() 
{
	CMenu *mmenu;
	int mid;

	m_nCurrentMode=MODE_ZOOM;

	mmenu=theApp.m_pMainWnd->GetMenu();
	mmenu=mmenu->GetSubMenu(3);
	mid=mmenu->GetMenuItemID(0);
	mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	mid=mmenu->GetMenuItemID(1);
	mmenu->CheckMenuItem(mid,MF_UNCHECKED);
	mid=mmenu->GetMenuItemID(2);
	mmenu->CheckMenuItem(mid,MF_CHECKED);
}

void CRayView::OnUpdateZoom(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nCurrentMode==MODE_ZOOM);
}

void CRayView::OnRemovetx() 
{
	CRayDoc *pDoc=GetDocument();
	int cnt;

	cnt=pDoc->m_selectedobjects.size();
	if (cnt==0) return;

	for(list<LPPRIMITIVE>::iterator oi=pDoc->m_selectedobjects.begin(); oi!=pDoc->m_selectedobjects.end(); oi++){
		(*oi)->m_pTX=NULL;
	}
	pDoc->SetModifiedFlag(TRUE);
	Invalidate();
}


void CRayView::OnLightColor() 
{
	float r,g,b;
	CRayDoc *pDoc=GetDocument();

	CColorDialog *cdlg;
	COLORREF color;

	pDoc->SelectedLightColor(&r,&g,&b);

	cdlg=new CColorDialog(RGB((int)(r*255),(int)(g*255),(int)(b*255)),CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN,this);
	
	if (cdlg->DoModal()==IDOK){
		color=cdlg->GetColor();
		pDoc->LightColor((float)GetRValue(color)/255,(float)GetGValue(color)/255,(float)GetBValue(color)/255);
	}
	delete cdlg;
}


void CRayView::DrawBoundingBox()
{
	CRayDoc *pDoc=GetDocument();

	if (pDoc->m_selectedobjects.size()>0) DrawSelBox();

	for(list<LPPRIMITIVE>::iterator oi=pDoc->m_selectedobjects.begin(); oi!=pDoc->m_selectedobjects.end(); oi++){
		if ((*oi)->IsSelected()){
			(*oi)->DrawSelBox();
		}
	}

	if (m_bMoveObj==TRUE || m_bResize==TRUE || m_bMoveLight==TRUE){
		V3 mray, mr1,mr2;
		glDisable(GL_LIGHTING);

		mray=m2-m1;

		mr1=m1-mray*10000.0f;
		mr2=m1+mray*10000.0f;
		
		glColor3f(1.0f,1.0f,1.0f);
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINES);
			glVertex3fv(mr1);
			glVertex3fv(mr2);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
		glEnable(GL_LIGHTING);
	}
}

// Given the screen coordinates, return the coordinates in world coordinate system
V3 CRayView::ProjectScreenPoint(int x, int y)
{
	V3 hv = (m_viewinfo.lrp-m_viewinfo.llp)/(float)m_nWidth;
	V3 vv = (m_viewinfo.ulp-m_viewinfo.llp)/(float)m_nHeight;

	V3 ret = m_viewinfo.llp+hv*(float)x+vv*(float)y;

	return ret;
}
