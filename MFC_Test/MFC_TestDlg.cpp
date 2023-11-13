
// MFC_TestDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MFC_Test.h"
#include "MFC_TestDlg.h"
#include "afxdialogex.h"
#include "afxwin.h"
#include <iostream>

#include <fstream>
#include <vector>
#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCTestDlg 대화 상자



CMFCTestDlg::CMFCTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFC_TEST_DIALOG, pParent)
	, print_file(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE, print_file);
}

BEGIN_MESSAGE_MAP(CMFCTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_RUN, &CMFCTestDlg::OnBnClickedButtonRun)
	ON_BN_CLICKED(IDC_BUTTON_RUN, &CMFCTestDlg::OnBnClickedButtonRun)
	ON_WM_CTLCOLOR()
//	ON_WM_ICONERASEBKGND()
END_MESSAGE_MAP()


// CMFCTestDlg 메시지 처리기

BOOL CMFCTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMFCTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMFCTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMFCTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCTestDlg::OnBnClickedButtonRun()
{
	CString strDefaultPath = _T("");

	CFileDialog dlg(TRUE, _T("Files (*.wav)"), NULL,
		OFN_FILEMUSTEXIST |		// 존재하는 파일만 선택 가능
		OFN_PATHMUSTEXIST |	    // 존재하는 경로만 선택 가능
		OFN_HIDEREADONLY |		// ReadOnly 체크박스 숨김
		OFN_LONGNAMES			//긴 파일 이름 포맷 지원
		, _T("Files (*.wav)|*.wav|All Files (*.*)|*.*|"));

	// 파일이 선택 된다면 문장 실행
	if (IDOK == dlg.DoModal())
	{
		// 경로 가져오기
		strDefaultPath = dlg.GetPathName();

		// WAV 파일 읽기
		CStdioFile file;
		if (file.Open(strDefaultPath, CFile::modeRead))
		{
			// WAV 헤더 읽기
			WavHeader header;
			file.Read(&header, sizeof(WavHeader));

			// 데이터 부분을 읽어 벡터에 저장
			std::vector<short> audioData;
			short sample;
			// short 크기만큼 데이터를 읽어서 sample에 저장
			while (file.Read(&sample, sizeof(short))) {
				// push_back(): 벡터에 추가
				audioData.push_back(sample);
			}

			// 파일 닫기
			file.Close();

			// Edit Control에 데이터 출력
			CString outputText;
			for (const auto& sample : audioData) {
				outputText.AppendFormat(_T("%d\r\n"), sample);
			}
			SetDlgItemText(IDC_EDIT_FILE, outputText);
		}
	}

	
}


HBRUSH CMFCTestDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	// IDC_EDIT_FILE 색상 변경(흰색)
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_FILE)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		hbr = ::CreateSolidBrush(RGB(255, 255, 255));
	}

	return hbr;
}