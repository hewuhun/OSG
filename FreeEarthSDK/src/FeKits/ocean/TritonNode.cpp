#include <FeKits/ocean/TritonDrawable.h>
#include <FeKits/ocean/TritonContext.h>
#include <FeKits/ocean/TritonNode.h>

using namespace FeKit;

TritonNode::TritonNode(osgViewer::View * viewer, const osgEarth::SpatialReference * srs)
	:HgOceanNode(viewer, srs)
{
	
	_TRITON = new TritonContext(  );

	_TRITON->SetSRS(srs);
	_drawable = new TritonDrawable(viewer, _TRITON);
	osg::Geode* geode = new osg::Geode();
	geode->addDrawable( _drawable );

	this->addChild( geode );

}

TritonNode::~TritonNode()
{
}

void TritonNode::OnSetSeaLevel()
{
	if ( _TRITON->ready() )
	{
		float d = GetSeaLevel();
		_TRITON->GetEnvironment()->SetSeaLevel( GetSeaLevel() );
	}
	dirtyBound();
}

osg::BoundingSphere
	TritonNode::computeBound() const
{
	return osg::BoundingSphere();
}

TritonContext * TritonNode::GetTritonContext()
{
    return _TRITON.get();
}


void FeKit::TritonNode::SetSeaLevel( float value )
{
    m_fSeaLevel = value;
    _TRITON->GetEnvironment()->SetSeaLevel(m_fSeaLevel);
}

float FeKit::TritonNode::GetSeaLevel() const
{
    return m_fSeaLevel;
}

void FeKit::TritonNode::SetWakeHeight( float value )
{
    m_fWakeHeight = value;
    Triton::WindFetch wf;
    wf.SetWind(m_fWakeHeight, 0.0 );
    _TRITON->GetEnvironment()->AddWindFetch(wf);

}

float FeKit::TritonNode::GetWakeHeight() const
{
    return m_fWakeHeight;
}
