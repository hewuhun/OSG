#include <FeTriton/TritonOpenGL.h>
#include <FeOcean/TritonDrawable.h>
#include <FeOcean/TritonContext.h>
#include <osg/Texture2D>
#include <osgEarth/NodeUtils>
#include <osgEarth/MapNode>
#include <OpenThreads/Mutex>
#include <FeOcean/TritonNode.h>
#include <FeUtils/logger/LoggerDef.h>
#include <FeTriton/Configuration.h>

using namespace FeOcean;


TritonDrawable::TritonDrawable(osgViewer::View * viewer, TritonNode* tn)
	: m_opTritonNode(tn)
{
	setUseDisplayList(false);
    setUseVertexBufferObjects(true);
	setDataVariance(osg::Object::DYNAMIC);
}

void TritonDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	if(!m_opTritonNode.valid()) 
	{
		return;
	}
	
	if(m_opTritonNode->IsInitialized())
	{
		osgEarth::Threading::ScopedMutexLock lock1(m_mutex);
		osg::State& state = *renderInfo.getState();

		//state.disableAllVertexArrays();
	
		if (!m_opTritonNode->GetTritonContext() || !m_opTritonNode->GetTritonContext()->IsReady() )
			return;

		::Triton::Environment* environment = m_opTritonNode->GetTritonContext()->GetEnvironment();

		if ( environment )
		{
			environment->SetCameraMatrix( state.getModelViewMatrix().ptr() );
			environment->SetProjectionMatrix( state.getProjectionMatrix().ptr() );
		}

		//state.dirtyAllVertexArrays();

		if ( environment )
		{
			osg::Light* light = renderInfo.getView() ? renderInfo.getView()->getLight() : NULL;

			osg::Matrix lightLocalToWorldMatrix = osg::Matrix::identity();

			if ( light && light->getPosition().w() == 0 )
			{
				osg::Vec4 ambient = light->getAmbient();
				osg::Vec4 diffuse = light->getDiffuse();
				osg::Vec4 position = light->getPosition();

				position = position * lightLocalToWorldMatrix;

				environment->SetDirectionalLight(
					::Triton::Vector3( position[0], position[1], position[2] ),
					::Triton::Vector3( diffuse[0],  diffuse[1],  diffuse[2] ) );

				environment->SetAmbientLight(
					::Triton::Vector3( ambient[0], ambient[1], ambient[2] ) );

			}

			if ( m_opTritonNode->GetTritonContext()->GetOcean() )
			{
				m_opTritonNode->GetTritonContext()->GetOcean()->Draw( renderInfo.getView()->getFrameStamp()->getSimulationTime() );
			}
		}

		//state.dirtyAllVertexArrays();
	}
	else
	{		
		m_opTritonNode->Initialize(renderInfo.getContextID());
	}
}

void FeOcean::TritonDrawable::setTritonBound(const osg::Vec3d& camPos)
{
	double ms = 0, earthRadius = 0;
	Triton::Configuration::GetDoubleValue("mesh-size", ms);
	Triton::Configuration::GetDoubleValue("equatorial-earth-radius-meters", earthRadius);

	if(earthRadius < 1 || ms <= 0)
		return;

	if(ms >= earthRadius)
		ms = earthRadius;

	osg::Vec3d tc = camPos;
	tc.normalize();
	tc = tc * (earthRadius - ms);

	osg::BoundingSphere bs(tc,ms);
	osg::BoundingBox bb;
	bb.expandBy(bs);
	this->setBound(bb);
}

FeOcean::TritonDrawable::~TritonDrawable()
{
}

FeOcean::GodRayDrawable::GodRayDrawable( TritonNode* tn )
	:m_opTritonNode(tn)
{
	setUseDisplayList(false);
	setUseVertexBufferObjects(true);
	setDataVariance(osg::Object::DYNAMIC);
	this->setCullingActive(false);
}


void FeOcean::GodRayDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
	osgEarth::Threading::ScopedMutexLock lock(m_mutex);
	if(m_opTritonNode.valid() && m_opTritonNode->GetTritonContext())
	{
		if(m_opTritonNode->GetTritonContext()->GetOcean())
		{	
			m_opTritonNode->GetTritonContext()->GetOcean()->DrawGodRay();
		}
	}
}

