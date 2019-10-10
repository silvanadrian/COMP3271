// DepthDlg.cpp : implementation file
//

#include "DepthDlg.h"


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


// DepthDlg message handlers
