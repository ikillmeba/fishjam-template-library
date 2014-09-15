#pragma once

#include <ftlBase.h>
#include "ColorQuantizer.h"

namespace FTL
{
    class CFColorQuantizerBase : public IFColorQuantizer{
    public:
        CFColorQuantizerBase();
        virtual ~CFColorQuantizerBase();
    public:
        //IFColorQuantizer
        virtual BOOL SetBmpInfo(UINT nWidth, UINT nHeight, UINT nBpp, BYTE* pBmpData, UINT nBmpDataSize, BOOL bCopyMem);
        virtual BOOL ProcessQuantizer(UINT colorCount, UINT *pResultClrCount);
        virtual COLORREF*   GetPalette(UINT* pResultCount);
        virtual int* GetQuantizerBuffer(INT* pSize);
    protected:
        virtual BOOL OnPrepare();
        virtual void OnFinish();
        virtual BOOL OnProcessQuantizer(UINT colorCount, UINT *pResultClrCount) = 0;
    protected:
        UINT m_nWidth;
        UINT m_nHeight;
        UINT m_nBpp;
        UINT m_nBmpDataSize;    //总的字节数
        UINT m_nBmpLineBytes;   //每行的字节数(4字节对齐)
        UINT m_nPaletteItemCount;

        BOOL        m_bCopyMem;
        BYTE*       m_pBmpData;
        COLORREF*   m_pResultPalette;
        UCHAR*      m_pResultBuffer;
        std::vector<int> m_indices;
    };
}
