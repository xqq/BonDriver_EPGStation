//
// @author magicxqq <xqq@xqq.im>
//

#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <Windows.h>
#include <cstdint>
#include "IBonDriver.h"
#include "IBonDriver2.h"

int main(int argc, char** argv) {
    setlocale(LC_ALL, "japanese");
    printf("IsDebuggerPresent(): %d\n", IsDebuggerPresent());

    auto bon = dynamic_cast<IBonDriver2*>(CreateBonDriver());
    bon->OpenTuner();

    printf("Tuner: %ls\n", bon->GetTunerName());

    DWORD space = 0;
    DWORD channel = 0;

    while (true) {
        const wchar_t* space_name = bon->EnumTuningSpace(space++);
        if (space_name == nullptr) {
            break;
        }
        printf("%ls\n", space_name);
        fflush(stdout);
    }

    space = 0;

    while (true) {
        const wchar_t* space_name = bon->EnumTuningSpace(space);
        if (space_name == nullptr) {
            break;
        }

        const wchar_t* channel_name = bon->EnumChannelName(space, channel);
        if (channel_name == nullptr) {
            space++;
            channel = 0;
            continue;
        } else {
            channel++;
        }

        printf("%ls: %ls \n", space_name, channel_name);
        fflush(stdout);
    }

    system("pause");

    bon->SetChannel(0, 0);
    bon->WaitTsStream();

    uint8_t* stream_data = nullptr;
    DWORD stream_size = 0;
    DWORD stream_remain = 0;

    for (int i = 0; i < 100; i++) {
        BOOL ret = bon->GetTsStream(&stream_data, &stream_size, &stream_remain);
        float signal_level = bon->GetSignalLevel();
        printf("stream_size: %u, stream_remain: %u, signal_level: %lf\n", stream_size, stream_remain, signal_level);

        if (!stream_remain) {
            Sleep(10);
        }
    }

    bon->CloseTuner();
    bon->Release();

    fflush(stdout);
    return 0;
}
