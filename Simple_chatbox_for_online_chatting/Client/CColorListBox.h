#include <vector>
using namespace std;

class CColorListBox : public CListBox
{
public: 
	CColorListBox();
	virtual ~CColorListBox();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//жиди
	//int InsertString(int n, LPCTSTR lpszItem, COLORREF itemColor = RGB(0,0,0));
	void SetColour(int n, COLORREF itemColor);
};
