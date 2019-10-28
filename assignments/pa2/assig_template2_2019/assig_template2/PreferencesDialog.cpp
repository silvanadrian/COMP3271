// PreferencesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ray.h"
#include "PreferencesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog dialog


CPreferencesDialog::CPreferencesDialog(CWnd* pParent /*=NULL*/,BOOL inshowtx,int inzoomfactor,int inviewangle,BOOL inblending)
	: CDialog(CPreferencesDialog::IDD, pParent)
{
	m_bShowTexture = inshowtx;	
	m_nZoomFactor = inzoomfactor;
	m_nViewAngle = inviewangle;
	m_bBlending = inblending;	
}


void CPreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);	
	DDX_Check(pDX, IDC_SHOWTX, m_bShowTexture);
	DDX_Check(pDX, IDC_BLENDING, m_bBlending);
}


BEGIN_MESSAGE_MAP(CPreferencesDialog, CDialog)
	//{{AFX_MSG_MAP(CPreferencesDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog message handlers

BOOL CPreferencesDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CSliderCtrl *sc;	
	
	sc=(CSliderCtrl*)GetDlgItem(IDC_ZOOMFACT);
	sc->SetRange(1,5);
	sc->SetPos(m_nZoomFactor);
	
	sc=(CSliderCtrl*)GetDlgItem(IDC_VIEWANGLE);
	sc->SetRange(10,120);
	sc->SetTicFreq(10);
	sc->SetPos(m_nViewAngle);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPreferencesDialog::OnOK() 
{
	CSliderCtrl *sc;
	
	sc=(CSliderCtrl*)GetDlgItem(IDC_ZOOMFACT);
	m_nZoomFactor=sc->GetPos();
	sc=(CSliderCtrl*)GetDlgItem(IDC_VIEWANGLE);
	m_nViewAngle=sc->GetPos();
	
	CDialog::OnOK();
}
