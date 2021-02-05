//
// @author magicxqq <xqq@xqq.im>
//

#include <cstdio>
#include <cstdlib>
#include "IBonDriver.h"

int main(int argc, char** argv) {
    auto ptr = CreateBonDriver();
    printf("IsDebuggerPresent(): %d", IsDebuggerPresent());
    fflush(stdout);
    return 0;
}