#ifndef ROTOR_WASH_CALLBACK__HH__
#define ROTOR_WASH_CALLBACK__HH__

#include <osg/NodeCallback>
#include <FeOcean/TritonNode.h>

namespace FeOcean
{

class FEOCEAN_EXPORT RotorWashCallback : public osg::NodeCallback
{
public:
    RotorWashCallback(TritonNode * tritonNode, float size = 10.0);
    void dirty();
    virtual void operator()(osg::Node * node, osg::NodeVisitor * nv);
protected:
    osg::observer_ptr<TritonNode>   _tritonNode;
    bool                            _dirty;
    Triton::RotorWash            *  _rotorWash;
    float                           _size;
    osg::Vec3d                      _lastPosition;
    double                          _lastFrameTime;
};

}
#endif
