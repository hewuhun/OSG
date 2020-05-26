
#include <FeEffects/EntityTrackEffect.h>

namespace FeEffect
{
	CEntityTrackEffect::CEntityTrackEffect(osg::MatrixTransform * pMt)
		:m_trackLineGeometry(NULL)
		,m_trackLineGeode(NULL)
		,m_pLineDraw(NULL)
		,m_opMt(pMt)
		,m_opTrackMt(NULL)
		,m_nCount(0)
	{
		
		m_opTrackMt = new osg::MatrixTransform;
		CreateEffect();
	}

	CEntityTrackEffect::~CEntityTrackEffect()
	{


	}

	bool CEntityTrackEffect::CreateEffect()
	{
		m_trackLineGeometry = new osg::Geometry;
		m_trackLineGeometry->setDataVariance(osg::Object::DYNAMIC);
		m_trackLineGeometry->setUseDisplayList(false);
		m_trackLineGeometry->setUseVertexBufferObjects(true);
		osg::ref_ptr<osg::Vec3dArray> vec = new osg::Vec3dArray;
		m_trackLineGeometry->setVertexArray(vec);
		osg::ref_ptr<osg::Vec4Array> vecColor = new osg::Vec4Array;//设置颜色数组
		vecColor->push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
		m_trackLineGeometry->setColorArray(vecColor);
		m_trackLineGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);//设置绑定方式，所有顶点都绑定

		m_trackLineGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		m_trackLineGeometry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		//m_trackLineGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,2));
		m_pLineDraw = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP);
		m_trackLineGeometry->addPrimitiveSet(m_pLineDraw);
		m_trackLineGeode = new osg::Geode;
		m_trackLineGeode->addDrawable(m_trackLineGeometry.get());

 		m_opTrackMt->addChild(m_trackLineGeode);
 		addChild(m_opTrackMt.get());

		m_opTrackMt->addUpdateCallback(new CTrackEffectCallback());
		
		return true;
	}

	void CEntityTrackEffect::UpdateTrackLine()
	{

		osg::Vec3d vecPosition = m_opMt->getMatrix().getTrans();
		osg::Vec3dArray* pLineVa = dynamic_cast<osg::Vec3dArray*>(m_trackLineGeometry->getVertexArray()); 

		if (m_nCount>=200)
		{
			pLineVa->erase(pLineVa->begin());
			m_pLineDraw->setCount(200);	
		}
		else
		{
			m_pLineDraw->setCount(m_nCount);		
		}
		pLineVa->push_back(vecPosition);
		pLineVa->dirty();
		//m_pLineDraw->setCount(m_nCount);	

		m_trackLineGeometry->dirtyDisplayList();
		m_trackLineGeometry->dirtyBound();
		++m_nCount;
	}

	CTrackEffectCallback::CTrackEffectCallback()
	{

	}

	CTrackEffectCallback::~CTrackEffectCallback()
	{

	}

	void CTrackEffectCallback::operator()( osg::Node *node,osg::NodeVisitor *nv )
	{
		if(node)
		{
			osg::MatrixTransform* pMt = dynamic_cast<osg::MatrixTransform*>(node);
			if(pMt)
			{
				CEntityTrackEffect* nodeeffect = dynamic_cast<CEntityTrackEffect*>(pMt->getParent(0));
				if(nodeeffect)
					nodeeffect->UpdateTrackLine();
			}
		}
		traverse(node,nv);
	}
}




