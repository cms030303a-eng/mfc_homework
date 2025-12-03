
// ChildView.h: CChildView 클래스의 인터페이스
//
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <algorithm>

#pragma once

// CChildView 창

class CChildView : public CWnd
{
// 생성입니다.
public:
	CChildView();

// 특성입니다.
public:

// 작업입니다.
public:

// 재정의입니다.
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected: // 멤버 변수
    struct Node {
        CPoint pt;
        int id;
    };

    struct Edge {
        int target;
        double weight;
    };

    CBitmap m_bgBitmap;

    std::vector<Node> m_nodes;                  // 점들의 좌표 저장
    std::vector<std::vector<Edge>> m_adjList;   // 그래프 인접 리스트 (연결 정보)

    int m_iSelectedNodeForLine;                 // 선 긋기를 위한 첫 번째 점 인덱스
    int m_iSelectedNodeForPath;                 // 경로 찾기를 위한 첫 번째 점 인덱스

    std::vector<int> m_shortestPath;            // 다익스트라 결과 경로 (점들의 인덱스)

    // 헬퍼 함수
    int GetClickedNodeIndex(CPoint point);      // 클릭한 위치에 점이 있는지 확인
    void RunDijkstra(int startNode, int endNode); // 다익스트라 알고리즘

// 구현입니다.
public:
	virtual ~CChildView();

	// 생성된 메시지 맵 함수
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
//	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
//	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

