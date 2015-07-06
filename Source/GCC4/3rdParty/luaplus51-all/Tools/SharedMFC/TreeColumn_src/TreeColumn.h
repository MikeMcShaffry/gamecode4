/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#if !defined(AFX_TREECOLUMN_H__55D4E2A3_EDF9_11D3_B75E_00C04F6A7AE6__INCLUDED_)
#define AFX_TREECOLUMN_H__55D4E2A3_EDF9_11D3_B75E_00C04F6A7AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\GRIDCTRL_SRC\GridCtrl.h"

// This structure sent to Grid's parent in a WM_NOTIFY message
typedef struct tagNM_TREECLICK {
    NMHDR hdr;
    int   iRow;
	bool  expand;
} NM_TREECLICK;

#define TC_NOTIFY_TREECLICK 5000

class CTreeColumn
{
public:
    CTreeColumn();
    virtual ~CTreeColumn();

// accessors
public:
    virtual void SetFormat(DWORD nFormat) { m_nFormat = nFormat; }
    virtual DWORD GetFormat() const { return m_nFormat; }

    virtual void SetBkClr(COLORREF crBkClr) { m_crBkClr = crBkClr; }
    virtual COLORREF GetBkClr() const { return m_crBkClr; }

    virtual void SetFgClr(COLORREF crFgClr) { m_crFgClr = crFgClr; }
    virtual COLORREF GetFgClr() const { return m_crFgClr; }

    virtual void SetParam(LPARAM lParam) { m_lParam = lParam; }
    virtual LPARAM GetParam() const { return m_lParam; }

    virtual void SetFont(const LOGFONT* plf)
    {
        // One Tree Column will serve multiple cells, so setting fonts can be tricky
        // since there is wonderful scope for overwriting one's self. Let's be cautious.
        if (plf == NULL)
        {
            delete m_plfFont;
            m_plfFont = NULL;
        }
        else
        {
            static LOGFONT lf;
            memcpy(&lf, plf, sizeof(LOGFONT));
            if (!m_plfFont)
                m_plfFont = new LOGFONT;
            if (m_plfFont)
                memcpy(m_plfFont, &lf, sizeof(LOGFONT)); 
        }
    }

    virtual LOGFONT* GetFont() const
    {
        if (m_plfFont)
            return (LOGFONT*) m_plfFont; 
        else
            return GetGrid()->GetDefaultCell(FALSE, FALSE)->GetFont();
    }

    virtual void SetAllowDraw(BOOL bAllowDraw) { m_bAllowDraw = bAllowDraw; }
    virtual BOOL GetAllowDraw() const { return m_bAllowDraw; }

    virtual void SetEditWnd(CWnd* pEditWnd) { m_pEditWnd = pEditWnd; }
    virtual CWnd* GetEditWnd() const { return m_pEditWnd; }

    virtual void SetGrid(CGridCtrl* pGrid) { m_pGrid = pGrid; }
    virtual CGridCtrl* GetGrid() const { return m_pGrid; }

    virtual void SetColumnWithTree( int iColumnWithTree) { m_iColumnWithTree = iColumnWithTree; }
    virtual int GetColumnWithTree() const { return m_iColumnWithTree; }

    virtual void SetDefTreeIndent( int nDefTreeIndent) { m_nDefTreeIndent = nDefTreeIndent; }
    virtual int GetDefTreeIndent() const { return m_nDefTreeIndent; }

    virtual void SetFixedRowCount( int iFixedRowCount) { m_iFixedRowCount = iFixedRowCount; }
    virtual int GetFixedRowCount() const { return m_iFixedRowCount; }

    virtual void SetRowCount( int iRowCount) { m_iRowCount = iRowCount; }
    virtual int GetRowCount() const { return m_iRowCount; }

// tree column operations
public:
    BOOL TreeSetup( CGridCtrl* apGridCtrl,  // tree acts on a column in this grid
                    int aiColumn,       // which column has tree
                    int aiTotalRows,    // total number of rows in the
                                        //  tree if totally expanded
                    int aiFixedRowCount,// Set fixed row count now, too
                                        //  Grid total rows=aiTotalRows+aiFixedRowCount
                    const unsigned char* apucTreeLevelAry,    // Tree Level data array --
                                                     //  must have aiTotalRows of entries
                    BOOL abShowTreeLines,   // T=show tree (not grid) lines; F=no tree lines
                    BOOL abUseImages,  // T=use 1st 3 images from already set image list
                                        //  to display folder graphics
                    CRuntimeClass* apRuntimeClass = NULL);
                                        // can use your special
                                        //  CGridTreeCellBase-derived class
    // returns:  success / fail

    int InsertTreeBranch(  
                   const unsigned char* apucTreeLevelAry,    // Tree Level data array --
                                       //  must have aiNbrElements of entries
                   int aiNbrElements,  // NUmber of tree elements to add
                   int aiRow,      // insert tree elements beginning at this row
                                   //  If -1, append to end; If 0, Insert
                                   //  at top no matter how many fixed rows
                   BOOL abRedraw); // T=redraw; F=don't
    // returns:  row where elements inserted; -1 on error

    int DeleteTreeBranch( int aiRow,        // Row that the tree branch begins on
                          BOOL abRedraw);   // T=redraw; F=don't
    // returns:  nbr of rows deleted

    void SetTreeUsesImages( BOOL abUseImages);   // T=use images
    BOOL GetTreeUsesImages() const              { return m_bTreeUsesImages; }

    int GetTreeUsesNbrImages() const;

    void SetTreeLines( BOOL abShowTreeLines);   // T=show tree lines
    BOOL GetTreeLines() const                   { return m_bShowTreeLines; }

    void     SetTreeLineColor(COLORREF clr)       { m_crTreeLineColour = clr;         }
    COLORREF GetTreeLineColor() const             { return m_crTreeLineColour;        }

    unsigned char GetTreeLevel(  int aiRow);  // row
    // returns:  tree level, =0 if invalid input
    BOOL IsTreeRowDisplayed(  int aiRow);  // row
    // returns:  T=tree row is displayed
    unsigned char GetLevelAndHide( int aiRow); // row

    void TreeRefreshRows();

    void TreeDataPrepOutline(unsigned char aucLevel, // level to display >= 0x80 displays all
                             int aiIndex,            // Index to tree display data to modify
                             int aiNbrElements);     // nbr of elements in tree display data

    void TreeDisplayOutline( unsigned char aucLevel );    // level to display >= 0x80 displays all

    void TreeDataExpandOneLevel( int aiGridRow);

    void TreeDataCollapseAllSubLevels( int aiGridRow);

    void TreeExpandCollapseToggle( int aiGridRow); // Grid row of node to toggle

    BOOL TreeCellHasPlusMinus(   int aiRow,          // row of Cell to check
                                 BOOL* apbIsPlus,    // returns:  T=Is a plus
                                 BOOL* apbIsMinus,   // returns:  T=Is a minus
                                 BOOL* apbIsLastLeaf);// returns:  T=Is Last Leaf
    // returns:  T=cell has a plus or minus;  F=not

    virtual BOOL IsDefaultFont() const       { return (m_plfFont == NULL); }

// let the CGridTreeCtrl maintain a single instance of the following
//  on the behalf of the expected column of CGridTreeCell's
protected:
	void SendTreeClickNotifyMessageToParent( int row, bool expand );

	DWORD    m_nFormat;     // Cell format
    COLORREF m_crBkClr;     // Background colour (or CLR_DEFAULT)
    COLORREF m_crFgClr;     // Forground colour (or CLR_DEFAULT)
    LPARAM   m_lParam;      // 32-bit value to associate with item
    LOGFONT *m_plfFont;     // Cell font

    BOOL m_bAllowDraw;  // private switch to turn off / on drawing
    CWnd* m_pEditWnd;   // can edit only 1 at a time
    CGridCtrl* m_pGrid;
    CRuntimeClass* m_pRuntimeClassTreeCell;

    // tree column properties
    int m_iColumnWithTree;
    int m_nDefTreeIndent;
    BOOL m_bShowTreeLines;
    BOOL m_bTreeUsesImages;
    COLORREF m_crTreeLineColour;
    int m_iFixedRowCount;
    int m_iRowCount;

};

#endif // !defined(AFX_TREECOLUMN_H__55D4E2A3_EDF9_11D3_B75E_00C04F6A7AE6__INCLUDED_)
