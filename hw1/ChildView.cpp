
// ChildView.cpp: CChildView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
#include "hw1.h"
#include "ChildView.h"
#include "Resource.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CChildView

CChildView::CChildView()
{
	m_iSelectedNodeForLine = -1;
	m_iSelectedNodeForPath = -1;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
    ON_WM_CREATE()
END_MESSAGE_MAP()



// CChildView 메시지 처리기

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.
	
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

    if (m_bgBitmap.GetSafeHandle() != NULL) 
    {
        CDC memDC;
        memDC.CreateCompatibleDC(&dc);

        CBitmap* pOldBitmap = memDC.SelectObject(&m_bgBitmap);

        BITMAP bmpInfo;
        m_bgBitmap.GetBitmap(&bmpInfo);

        CRect rect;
        GetClientRect(&rect);

        dc.StretchBlt(0, 0, rect.Width(), rect.Height(),
            &memDC,
            0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, 
            SRCCOPY);

        memDC.SelectObject(pOldBitmap);
    }

    CPen penLine(PS_SOLID, 1, RGB(0, 0, 0));
    CPen* pOldPen = dc.SelectObject(&penLine);

    for (int i = 0; i < m_adjList.size(); i++) {
        for (const auto& edge : m_adjList[i]) {
            if (i < edge.target) {
                dc.MoveTo(m_nodes[i].pt);
                dc.LineTo(m_nodes[edge.target].pt);
            }
        }
    }

    if (m_shortestPath.size() > 1) {
        CPen penPath(PS_SOLID, 3, RGB(255, 0, 0));
        dc.SelectObject(&penPath);

        for (size_t i = 0; i < m_shortestPath.size() - 1; i++) {
            int u = m_shortestPath[i];
            int v = m_shortestPath[i + 1];
            dc.MoveTo(m_nodes[u].pt);
            dc.LineTo(m_nodes[v].pt);
        }
        dc.SelectObject(&penLine); 
    }
    dc.SelectObject(pOldPen);

    CBrush brushNode(RGB(0, 0, 255)); 
    CBrush brushSel(RGB(0, 255, 0));  
    CBrush* pOldBrush = dc.SelectObject(&brushNode);

    int radius = 5;
    for (const auto& node : m_nodes) {
        if (node.id == m_iSelectedNodeForLine || node.id == m_iSelectedNodeForPath)
            dc.SelectObject(&brushSel);
        else
            dc.SelectObject(&brushNode);

        dc.Ellipse(node.pt.x - radius, node.pt.y - radius,
            node.pt.x + radius, node.pt.y + radius);
    }
    dc.SelectObject(pOldBrush);
}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
    BOOL bCtrl = (GetKeyState(VK_CONTROL)&0X8000);
    BOOL bShift = (GetKeyState(VK_SHIFT)&0X8000);
    BOOL bAlt = (GetKeyState(VK_MENU)&0X8000);

    // 1. Ctrl + Shift + 좌클릭: 최단 경로 구하기 (두 점 선택)
    if (bCtrl && bShift)
    {
        int clickedNode = GetClickedNodeIndex(point);
        if (clickedNode != -1)
        {
            if (m_iSelectedNodeForPath == -1) {
                m_iSelectedNodeForPath = clickedNode;
                m_shortestPath.clear(); 
            }
            else {
                // 두 번째 점 선택 -> 다익스트라 실행
                RunDijkstra(m_iSelectedNodeForPath, clickedNode);
                m_iSelectedNodeForPath = -1;
            }
            Invalidate();
        }
    }
    // 2. Ctrl + 좌클릭: 점(Node) 추가
    else if (bCtrl && !bAlt)
    {
        Node newNode;
        newNode.pt = point;
        newNode.id = (int)m_nodes.size();
        m_nodes.push_back(newNode);

        m_adjList.resize(m_nodes.size());

        Invalidate();
    }
    // 3. Alt + 좌클릭: 선(Edge) 긋기 (두 점 선택)
    else if (bAlt)
    {
        int clickedNode = GetClickedNodeIndex(point);
        if (clickedNode != -1)
        {
            if (m_iSelectedNodeForLine == -1) {
                m_iSelectedNodeForLine = clickedNode; // 첫 번째 점
            }
            else if (m_iSelectedNodeForLine != clickedNode) {
                // 두 번째 점 -> 선 연결 
                int u = m_iSelectedNodeForLine;
                int v = clickedNode;

                double dist = sqrt(pow(m_nodes[u].pt.x - m_nodes[v].pt.x, 2) +
                    pow(m_nodes[u].pt.y - m_nodes[v].pt.y, 2));

                m_adjList[u].push_back({ v, dist });
                m_adjList[v].push_back({ u, dist });

                m_iSelectedNodeForLine = -1;
                Invalidate();
            }
        }
    }
	CWnd::OnLButtonDown(nFlags, point);
}

// 클릭한 좌표 근처에 노드가 있는지 확인 (반경 10픽셀 이내)
int CChildView::GetClickedNodeIndex(CPoint point)
{
    for (const auto& node : m_nodes) {
        if (abs(point.x - node.pt.x) < 10 && abs(point.y - node.pt.y) < 10) {
            return node.id;
        }
    }
    return -1; // 없음
}

// 다익스트라 알고리즘 구현
void CChildView::RunDijkstra(int startNode, int endNode)
{
    int n = (int)m_nodes.size();
    std::vector<double> dist(n, (std::numeric_limits<double>::max)());
    std::vector<int> parent(n, -1);

    std::priority_queue<std::pair<double, int>,
        std::vector<std::pair<double, int>>,
        std::greater<std::pair<double, int>>> pq;

    dist[startNode] = 0;
    pq.push({ 0, startNode });

    while (!pq.empty()) {
        double d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > dist[u]) continue;
        if (u == endNode) break; 

        for (const auto& edge : m_adjList[u]) {
            if (dist[u] + edge.weight < dist[edge.target]) {
                dist[edge.target] = dist[u] + edge.weight;
                parent[edge.target] = u;
                pq.push({ dist[edge.target], edge.target });
            }
        }
    }

    m_shortestPath.clear();
    if (dist[endNode] != (std::numeric_limits<double>::max)()) {
        for (int v = endNode; v != -1; v = parent[v]) {
            m_shortestPath.push_back(v);
        }
        std::reverse(m_shortestPath.begin(), m_shortestPath.end());
    }
    else {
        AfxMessageBox(_T("경로가 존재하지 않습니다."));
    }
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    if (!m_bgBitmap.LoadBitmap(IDB_BITMAP2))
    {
        AfxMessageBox(_T("비트맵 리소스를 로드할 수 없습니다."));
    }
    // TODO:  여기에 특수화된 작성 코드를 추가합니다.
    return 0;
}
