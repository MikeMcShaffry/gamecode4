#include "dll.h"

export {
    typedef struct {
        int ival;
    } MyStruct;
}

export int one(MyStruct *ms) {
    return ms->ival + 1
}

export int two(MyStruct *ms) {
    return 2*ms->ival;
}

finis
