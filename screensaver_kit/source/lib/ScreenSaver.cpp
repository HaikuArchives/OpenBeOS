#include "ScreenSaver.h"



BScreenSaver::BScreenSaver(BMessage *archive,

                           image_id)

{

        ticksize = 50000;

        looponcount = 0;

        loopoffcount = 0;

}





BScreenSaver::~BScreenSaver()

{

}





status_t

BScreenSaver::InitCheck()

{

    return 0;

}





status_t

BScreenSaver::StartSaver(BView *view,

                         bool preview)

{

    return 0;

}





void

BScreenSaver::StopSaver()

{

}





void

BScreenSaver::Draw(BView *view,

                   int32 frame)

{

}





void

BScreenSaver::DirectConnected(direct_buffer_info *info)

{

}





void

BScreenSaver::DirectDraw(int32 frame)

{

}





void

BScreenSaver::StartConfig(BView *configView)

{

}





void

BScreenSaver::StopConfig()

{

}





void

BScreenSaver::SupplyInfo(BMessage *info) const

{

}





void

BScreenSaver::ModulesChanged(const BMessage *info)

{

}





status_t

BScreenSaver::SaveState(BMessage *into) const

{

    return -1;

}





void

BScreenSaver::SetTickSize(bigtime_t ts)

{

        ticksize = ts;

}





bigtime_t

BScreenSaver::TickSize() const

{

    return ticksize;

}





void

BScreenSaver::SetLoop(int32 on_count,

                      int32 off_count)

{

        looponcount = on_count;

        loopoffcount = off_count;

}





int32

BScreenSaver::LoopOnCount() const

{

    return looponcount;

}





int32

BScreenSaver::LoopOffCount() const

{

    return loopoffcount;

}





void

BScreenSaver::_ReservedScreenSaver1()

{

}





void

BScreenSaver::_ReservedScreenSaver2()

{

}





void

BScreenSaver::_ReservedScreenSaver3()

{

}





void

BScreenSaver::_ReservedScreenSaver4()

{

}





void

BScreenSaver::_ReservedScreenSaver5()

{

}





void

BScreenSaver::_ReservedScreenSaver6()

{

}





void

BScreenSaver::_ReservedScreenSaver7()

{

}





void

BScreenSaver::_ReservedScreenSaver8()

{

}

