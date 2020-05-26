#include <osg/Depth>
#include <osg/Notify>
#include <osg/CullFace>
#include <osg/LineWidth>
#include <osg/Transform>
#include <osg/Geometry>
#include <osg/TriangleIndexFunctor> 
#include <osg/ShapeDrawable>
#include <osg/io_utils>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Geode> 
#include <osg/Matrixd>
#include <osg/BlendFunc>
#include <algorithm>
#include <list>

#include <FeEffects/RadarWaveEffect.h>

using namespace osgSim;

namespace FeEffect
{
	//CSphereSegment.cpp
	CRadarWaveEffect::CRadarWaveEffect(void)
		:CSphereSegment()
	{
		Init();
	}

	CRadarWaveEffect::CRadarWaveEffect (const osg::Vec3 &centre, float radius, float azMin, float azMax, float elevMin, float elevMax, int density) 
		:CSphereSegment(centre, radius, azMin, azMax, elevMin, elevMax, density)
	{
		Init();
	}
	CRadarWaveEffect::CRadarWaveEffect (const osg::Vec3 &centre, float radius, const osg::Vec3 &vec, float azRange, float elevRange, int density) 
		:CSphereSegment(centre, radius, vec, azRange, elevRange, density)
	{
		Init();
	}
	CRadarWaveEffect::CRadarWaveEffect (const SphereSegment &rhs, const osg::CopyOp &co) 
		:CSphereSegment(rhs, co)
	{
		Init();
	}

	CRadarWaveEffect::~CRadarWaveEffect(void)
	{

	}

	void CRadarWaveEffect::Init()
	{
		this->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		this->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		osg::ref_ptr<osg::Depth> pDepth = new osg::Depth();
		pDepth->setWriteMask(false);
		this->getOrCreateStateSet()->setAttribute(pDepth, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		m_geom = new osg::Geometry;
		m_geom->setUseDisplayList(false);
		m_geom->setUseVertexBufferObjects(true);

		//顶点序列
		osg::Vec3dArray* vertex = new osg::Vec3dArray;
		m_geom->setVertexArray(vertex);

		//颜色序列
		osg::Vec4dArray* colorArray = new osg::Vec4dArray;
		m_geom->setColorArray(colorArray);
		m_geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX); 

		const int nNum = 45;

		const float azIncr = (m_azMax - m_azMin) / m_density;
		const float elevIncr = (m_elevMax - m_elevMin) / m_density;

		float deltaAlpha = (0.0 - 1.0)/5.0;
		float fromAlpha = 1.0;

		float toAlpha = fromAlpha + deltaAlpha;

		double dUnit =  m_radius/nNum;
		for(int i =0;i<(nNum-1);i++)
		{
			for(int n =0 ;n<=m_density*4;n++)
			{     
				float az = m_azMin;
				float elev = m_elevMin; 
				if(n <m_density)
				{
					az =  m_azMin +n*azIncr;
				}
				else if(n<m_density*2)
				{
					az = m_azMax;// - (n-m_density)* azIncr;
					elev = m_elevMin + (n-m_density)* elevIncr;
				}
				else if(n<m_density*3)
				{
					az = m_azMax- (n-m_density*2)* azIncr;
					elev = m_elevMax;// - (n-m_density)* elevIncr;
				}
				else if(n<m_density*4)
				{
					az = m_azMin;// - (n-m_density)* azIncr;
					elev = m_elevMax - (n-m_density*3)* elevIncr;
				}
				
				
				//if(n == 1)
				//{
				//	az = m_azMax;
				//}
				//else if(n == 2)
				//{
				//	az = m_azMax;
				//	elev = m_elevMax;
				//}
				//else if(n == 3)
				//{
				//	az = m_azMin;
				//	elev = m_elevMax;
				//}
    
				float x = cos(elev)*sin(az);
				float y = cos(elev)*cos(az);
				float z = sin(elev); 

				osg::Vec3d fromPos(m_centre.x() + dUnit*i*x, 
					m_centre.y() + dUnit*i*y,
					m_centre.z() + dUnit*i*z);
				osg::Vec3d toPos(m_centre.x() + dUnit*(i+1)*x, 
					m_centre.y() + dUnit*(i+1)*y,
					m_centre.z() + dUnit*(i+1)*z);

				vertex->push_back(fromPos);
				colorArray->push_back(osg::Vec4d(0.0,0.0,1.0, fromAlpha));

				vertex->push_back(toPos);
				colorArray->push_back(osg::Vec4d(0.0,0.0,1.0, toAlpha));

			}
			fromAlpha = toAlpha;
			toAlpha += deltaAlpha;

			if(toAlpha > 0.0)
			{
				deltaAlpha = -deltaAlpha;
			}

			if(toAlpha < 1.0)
			{
				deltaAlpha = -deltaAlpha;
			}
		}
		m_geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, vertex->size()));
		addDrawable(m_geom.get());
		m_geom->setDrawCallback(new WaveDrawableDrawCallback(osg::Vec4d(0.5f,1.0f,1.0f,0.1f),(4*m_density+1)*2));
	}

}

namespace FeEffect
{
	WaveDrawableDrawCallback::WaveDrawableDrawCallback(osg::Vec4d vecColor,int density)
		:osg::Drawable::DrawCallback()
		,m_vecColor(vecColor)
		,m_density(density)
		,m_dTime(0.0)
	{
		m_bChangeColor = true;
		m_frameTiaoGuo = 0;

	}

	WaveDrawableDrawCallback::~WaveDrawableDrawCallback()
	{

	}

	void WaveDrawableDrawCallback::drawImplementation( osg::RenderInfo& renderInfo,const osg::Drawable* drawable ) const
	{
		
		static double dTime = renderInfo.getView()->getFrameStamp()->getSimulationTime();
		double dTimeTemp = renderInfo.getView()->getFrameStamp()->getSimulationTime();
		if(dTimeTemp - dTime < 0.05)
		{
			drawable->drawImplementation(renderInfo);
			return;
		}
		dTime = dTimeTemp;
		//static int nindex = 0;
		//nindex++;
		//if(nindex < 50)
		//{
		//	return;
		//}
		osg::Geometry* pGeom = (osg::Geometry*)drawable;
		if(!pGeom)
		{
			return;
		}

		{
			osg::Vec4dArray* pColorArray = (osg::Vec4dArray*)pGeom->getColorArray();
			pColorArray->dirty();

			//m_density个是一圈，一圈是两层，第m0层要取第m1层的值， m1层取下一圈m1层的值，最后一圈的m1取最开始m0层的值
			osg::Vec4d colorM0From = pColorArray->at(pColorArray->size()-1); //最开始m0的值

			int sizeColor = pColorArray->size()/(m_density);

			for(int i = (sizeColor-1); i>=0; i--)
			{
				osg::Vec4d m1 = pColorArray->at(i*m_density);
				osg::Vec4d mNext1;
				if(i == 0)
				{
					mNext1 = colorM0From;
				}
				else
				{
					mNext1 = pColorArray->at((i-1)*m_density);
				}


				for(int j = 0; j<(m_density/2);j++)
				{
					pColorArray->at(i*m_density + j*2 + 1).set(m_vecColor.r(), m_vecColor.g(), m_vecColor.b(), m1.a());
					pColorArray->at(i*m_density + j*2 + 0).set(m_vecColor.r(), m_vecColor.g(), m_vecColor.b(), mNext1.a());
				}
			}  
		}

		pGeom->dirtyBound();

		drawable->drawImplementation(renderInfo);
	}
	
	void WaveDrawableDrawCallback::SetColor(osg::Vec4d &vecColor)
	{
		m_vecColor = vecColor;
	}
	
	void WaveDrawableDrawCallback::SetChangeColor( bool bChange )
	{
		m_bChangeColor = bChange;
	}
}