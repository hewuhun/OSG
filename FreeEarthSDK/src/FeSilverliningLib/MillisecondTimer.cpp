// Copyright (c) 2007-2009 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/MillisecondTimer.h>
#include <FeSilverliningLib/Utils.h>

using namespace SilverLining;

unsigned long MillisecondTimer::GetMilliseconds() const
{
    return getMilliseconds();
}
