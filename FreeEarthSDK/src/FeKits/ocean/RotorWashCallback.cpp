#include "FeKits/ocean/RotorWashCallback.h"

using namespace FeKit;

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
                _rotorWash = new Triton::RotorWash(_tritonNode->GetTritonContext()->GetOcean(), _size, true, false);
                _dirty =true; 
                firstRun = true;
            }
        }

        if (_rotorWash != NULL)
        {

            osg::Vec3 pos = node->getWorldMatrices()[0].getTrans();
            double time = nv->getFrameStamp()->getSimulationTime();
            _rotorWash->Update(Triton::Vector3(pos[0], pos[1], 1.0), Triton::Vector3(0, 0.0, -1.0), _size, time);
            std::cout << _size << std::endl;
        }
        traverse(node, nv);
    }
}



