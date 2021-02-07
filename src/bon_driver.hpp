//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_BON_DRIVER_HPP
#define BONDRIVER_EPGSTATION_BON_DRIVER_HPP

#include "IBonDriver2.h"
#include "config.hpp"

class BonDriver : public IBonDriver2 {
public:
    BonDriver(const Config& config);
    ~BonDriver();
protected:
    void Release(void) override;

    const BOOL OpenTuner(void) override;
    void CloseTuner(void) override;
    const BOOL SetChannel(const BYTE bCh) override;
    const float GetSignalLevel(void) override;
    const DWORD WaitTsStream(const DWORD dwTimeOut = 0) override;
    const DWORD GetReadyCount(void) override;
    const BOOL GetTsStream(BYTE* pDst, DWORD* pdwSize, DWORD* pdwRemain) override;
    const BOOL GetTsStream(BYTE** ppDst, DWORD* pdwSize, DWORD* pdwRemain) override;
    void PurgeTsStream(void) override;

    LPCTSTR GetTunerName(void) override;
    const BOOL IsTunerOpening(void) override;
    LPCTSTR EnumTuningSpace(const DWORD dwSpace) override;
    LPCTSTR EnumChannelName(const DWORD dwSpace, const DWORD dwChannel) override;
    const BOOL SetChannel(const DWORD dwSpace, const DWORD dwChannel) override;
    const DWORD GetCurSpace(void) override;
    const DWORD GetCurChannel(void) override;
private:
    const Config& config_;
};


#endif // BONDRIVER_EPGSTATION_BON_DRIVER_HPP
