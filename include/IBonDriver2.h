//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_IBONDRIVER2_H
#define BONDRIVER_EPGSTATION_IBONDRIVER2_H

#include "IBonDriver.h"

class IBonDriver2 : public IBonDriver
{
public:
// IBonDriver2
    virtual LPCTSTR GetTunerName(void) = 0;

    virtual const BOOL IsTunerOpening(void) = 0;

    virtual LPCTSTR EnumTuningSpace(const DWORD dwSpace) = 0;
    virtual LPCTSTR EnumChannelName(const DWORD dwSpace, const DWORD dwChannel) = 0;

    virtual const BOOL SetChannel(const DWORD dwSpace, const DWORD dwChannel) = 0;

    virtual const DWORD GetCurSpace(void) = 0;
    virtual const DWORD GetCurChannel(void) = 0;

// IBonDriver
    virtual void Release(void) = 0;
};

#endif // BONDRIVER_EPGSTATION_IBONDRIVER2_H
