#include "FeOcean/RotorWashCallback.h"
#include <FeOcean/TritonContext.h>

using namespace FeOcean;

RotorWashCallback::RotorWashCallback( TritonNode * tritonNode, float size /*= 10.0*/ ) 
    :_tritonNode(tritonNode) 
    ,_dirty(true)
    ,_rotorWash(NULL)
    ,_size(size)
{

}

void RotorWashCallback::dirty()
{
    _dirty = true;
}
void RotorWashCallback::operator()( osg::Node * node, osg::NodeVisitor * nv ) 
{
    {
        bool firstRun = false;


        if (_dirty)
        {
            if (_rotorWash != NULL)
            {
                delete _rotorWash;
                _rotorWash = NULL;
            }
            if (_tritonNode.valid() && _tritonNode->GetTritonContext()->GetOcean())
            {
                _rotorWash = new Triton::RotorWash(_tritonNode->GetTritonContext()->GetOcean(), 10, true, true);
                _dirty =false; 
                firstRun = true;
            }
        }

        if (_rotorWash != NULL)
        {
			osg::Vec3d pos = node->getWorldMatrices()[0].getTrans();
            osg::Vec3d temp = pos - _lastPosition;
            double time = nv->getFrameStamp()->getSimulationTime();
            double velocity = temp.length()/(time - _lastFrameTime);
            //_rotorWash->Update(Triton::Vector3(pos[0], pos[1], pos[2]), Triton::Vector3(-pos[0], -pos[1], -pos[2]), 200000, time);
            _rotorWash->Update(Triton::Vector3(pos[0], pos[1], pos[2]), Triton::Vector3(-pos[0], -pos[1], -pos[2]), 10, time);
			_lastPosition = pos;
            _lastFrameTime = time;
        }
        traverse(node, nv);
    }
}



