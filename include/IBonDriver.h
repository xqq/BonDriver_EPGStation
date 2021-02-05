//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_IBONDRIVER_H
#define BONDRIVER_EPGSTATION_IBONDRIVER_H

#include "export.hpp"

#ifdef _WIN32
    #include <Windows.h>
#else
    #include "min_win32_typedef.hpp"
#endif

class IBonDriver {
public:
    virtual const BOOL OpenTuner(void) = 0;
    virtual void CloseTuner(void) = 0;

    virtual const BOOL SetChannel(const BYTE bCh) = 0;
    virtual const float GetSignalLevel(void) = 0;

    virtual const DWORD WaitTsStream(const DWORD dwTimeOut = 0) = 0;
    virtual const DWORD GetReadyCount(void) = 0;

    virtual const BOOL GetTsStream(BYTE* pDst, DWORD* pdwSize, DWORD* pdwRemain) = 0;
    virtual const BOOL GetTsStream(BYTE** ppDst, DWORD* pdwSize, DWORD* pdwRemain) = 0;

    virtual void PurgeTsStream(void) = 0;

    virtual void Release(void) = 0;
};

extern "C" EXPORT_API IBonDriver* CreateBonDriver();

#endif // BONDRIVER_EPGSTATION_IBONDRIVER_H
