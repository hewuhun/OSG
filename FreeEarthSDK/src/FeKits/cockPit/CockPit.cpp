#include <FeKits/cockPit/CockPit.h>

namespace FeKit
{


CCockPit::CCockPit()
    :osg::Referenced()
    ,m_bSHow(false)
{

}

CCockPit::~CCockPit()
{

}


bool CCockPit::IsHidden() const
{
    return !m_bSHow;
}

}
