//
// @author magicxqq <xqq@xqq.im>
//

#include "bon_driver.hpp"

BonDriver::BonDriver(const Config& config) : config_(config) {

}

BonDriver::~BonDriver() {

}

void BonDriver::Release(void) {

}

const BOOL BonDriver::OpenTuner(void) {
    return 0;
}

void BonDriver::CloseTuner(void) {

}

const BOOL BonDriver::SetChannel(const BYTE bCh) {
    return 0;
}

const BOOL BonDriver::SetChannel(const DWORD dwSpace, const DWORD dwChannel) {
    return 0;
}

const float BonDriver::GetSignalLevel(void) {
    return 0;
}

const DWORD BonDriver::WaitTsStream(const DWORD dwTimeOut) {
    return 0;
}

const DWORD BonDriver::GetReadyCount(void) {
    return 0;
}

const BOOL BonDriver::GetTsStream(BYTE *pDst, DWORD *pdwSize, DWORD *pdwRemain) {
    return 0;
}

const BOOL BonDriver::GetTsStream(BYTE **ppDst, DWORD *pdwSize, DWORD *pdwRemain) {
    return 0;
}

void BonDriver::PurgeTsStream(void) {

}

LPCTSTR BonDriver::GetTunerName(void) {
    return nullptr;
}

const BOOL BonDriver::IsTunerOpening(void) {
    return 0;
}

LPCTSTR BonDriver::EnumTuningSpace(const DWORD dwSpace) {
    return nullptr;
}

LPCTSTR BonDriver::EnumChannelName(const DWORD dwSpace, const DWORD dwChannel) {
    return nullptr;
}


const DWORD BonDriver::GetCurSpace(void) {
    return 0;
}

const DWORD BonDriver::GetCurChannel(void) {
    return 0;
}
