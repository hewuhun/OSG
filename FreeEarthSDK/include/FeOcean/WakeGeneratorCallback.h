#ifndef WAKE_GENERATOR_CALLBACK__HH__
#define WAKE_GENERATOR_CALLBACK__HH__
#include "TritonNode.h"

namespace FeOcean
{
class FEOCEAN_EXPORT WakeGeneratorCallback : public osg::NodeCallback
{
public:
    WakeGeneratorCallback( TritonNode* t , const Triton::WakeGeneratorParameters& parameters);
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

    void setTritonNode( TritonNode* t ) { _triton = t; dirty(); }
    TritonNode* getTritonNode() { return _triton.get(); }
    const TritonNode* getTritonNode() const { return _triton.get(); }

    Triton::WakeGenerator* getWakeGenerator() { return _wakeGenerator; }
    const Triton::WakeGenerator* getWakeGenerator() const { return _wakeGenerator; }

    Triton::WakeGeneratorParameters& getParameters() { return _parameters; }
    const Triton::WakeGeneratorParameters& getParameters() const { return _parameters; }

    void dirty() { _dirty = true; }

protected:
    osg::observer_ptr<TritonNode> _triton;
    Triton::WakeGenerator* _wakeGenerator;
    Triton::WakeGeneratorParameters _parameters;
    osg::Vec3d _lastPosition;
    double _lastFrameTime;
    bool _dirty;

};

}
#endif