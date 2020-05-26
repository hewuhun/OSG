#include "FeKits/ocean/WakeGeneratorCallback.h"

using namespace FeKit;

WakeGeneratorCallback::WakeGeneratorCallback(TritonNode* t)
    : _triton(t)
    , _lastFrameTime(0.0)
    ,_dirty(true)
    ,_wakeGenerator(NULL)
{

}

void WakeGeneratorCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    bool firstRun = false;
    if ( _dirty )
    {
        if ( _wakeGenerator!=NULL)
        {
            delete _wakeGenerator;
            _wakeGenerator = NULL;
        }

        if ( _triton.valid() && _triton->GetTritonContext()->GetOcean() )
        {
            _wakeGenerator = new Triton::WakeGenerator(_triton->GetTritonContext()->GetOcean(), _parameters );
            _dirty = false;
            firstRun = true;
        }
    }

    if ( _wakeGenerator!=NULL )
    {
        osg::Vec3 dir, pos = node->getWorldMatrices()[0].getTrans();
        double velocity = 0.0, time = nv->getFrameStamp()->getSimulationTime();
        if ( !firstRun )
        {
            dir = pos - _lastPosition;
            velocity = dir.normalize();
            velocity /= (time > _lastFrameTime) ? (time - _lastFrameTime) : 1.0;
        }

        _wakeGenerator->Update( Triton::Vector3(pos[0], pos[1], pos[2]), Triton::Vector3(dir[0], dir[1], dir[2]), velocity, time );
        _lastPosition = pos;
        _lastFrameTime = time;
    }
    traverse( node, nv );
}