#include <FeEffects/MutableCylinder.h>

#include <osg/PrimitiveSet>
#include <osg/Depth>
#include <osg/CullFace>
#include <osg/PolygonOffset>
#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osgUtil/SmoothingVisitor>

namespace FeEffect
{
	CMutableCylinder::CMutableCylinder()
		: m_vecStartPoint(osg::Vec3d(0.0, 0.0, 0.0))
		, m_vecEndPoint(osg::Vec3d(0.0, 0.0, 0.0))
		, m_dStartRadius(5.0)
		, m_dEndRadius(8.0)
		, m_dStartAlpha(0.0)
		, m_dEndAlpha(0.5)
		, m_color(osg::Vec4d(1.0, 1.0, 0.0, 0.8))
		, m_nFrameDelta(2)
		, m_nSection(98)
		, m_nDensity(36)
		, m_bUpdate(false)
		,m_bUpdatePos(false)
	{
		Init();
		Update();
	}

	CMutableCylinder::CMutableCylinder(osg::Vec3d posStart, osg::Vec3d posEnd,
		double dStartRadius, double dEndRadius,osg::Vec4d color,
		double dStartAlpha, double dEndAlpha,  bool bColorTrans)
		: m_vecStartPoint(posStart)
		, m_vecEndPoint(posEnd)
		, m_dStartRadius(dStartRadius)
		, m_dEndRadius(dEndRadius)
		, m_dStartAlpha(dStartAlpha)
		, m_dEndAlpha(dEndAlpha)
		, m_color(color)
		, m_bColorTrans(bColorTrans)
		, m_nFrameDelta(2)
		, m_nSection(98)
		, m_nDensity(36)
		, m_bUpdate(false)
		,m_bUpdatePos(false)
	{
		Init();
		Update();
	}

	CMutableCylinder::~CMutableCylinder()
	{

	}

	void CMutableCylinder::Init()
	{
		//c00005 at huayin
		this->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		getOrCreateStateSet()->setMode(GL_LINE_SMOOTH,osg::StateAttribute::ON);
		getOrCreateStateSet()->setMode(GL_POINT_SMOOTH,osg::StateAttribute::ON);
		getOrCreateStateSet()->setMode(GL_POLYGON_SMOOTH,osg::StateAttribute::ON);

		osg::ref_ptr<osg::Depth> pDepth = new osg::Depth();
		pDepth->setWriteMask(false);
		this->getOrCreateStateSet()->setAttribute(pDepth, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		m_rpGeode = new osg::Geode;
		m_rpGeom = new osg::Geometry;
		m_rpMatrixTransform = new osg::MatrixTransform;
		m_rpGeode->addDrawable(m_rpGeom);
		m_rpMatrixTransform->addChild(m_rpGeode);

		m_rpGeom->setUseDisplayList(false);
		m_rpGeom->setUseVertexBufferObjects(true);

		addChild(m_rpMatrixTransform);

		m_rpCallback = new CMutableCylinderCallback(this);
		m_rpGeom->setDrawCallback(m_rpCallback);

		//顶点序列
		osg::ref_ptr<osg::Vec3dArray> pCoord = new osg::Vec3dArray;
		m_rpGeom->setVertexArray(pCoord);

		//颜色序列
		osg::ref_ptr<osg::Vec4dArray> pColor = new osg::Vec4dArray;
		m_rpGeom->setColorArray(pColor);
		m_rpGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	}

	void CMutableCylinder::Update()
	{
		osg::ref_ptr<osg::Vec3dArray> pCoord = dynamic_cast<osg::Vec3dArray*>(m_rpGeom->getVertexArray());
		osg::ref_ptr<osg::Vec4dArray> pColor = dynamic_cast<osg::Vec4dArray*>(m_rpGeom->getColorArray());
		if (false == pColor.valid() || false == pCoord.valid())
		{
			return;
		}
		pCoord->clear();
		pColor->clear();

		//首先确定其长度
		double dLenght = (m_vecEndPoint - m_vecStartPoint).length();
		if (dLenght <= 0)
		{
			return;
		}

		//计算颜色透明度的过渡的差量值
		float deltaAlpha = (m_dEndAlpha - m_dStartAlpha) / 5;
		float fromAlpha = m_dStartAlpha;
		float toAlpha = fromAlpha + deltaAlpha;

		int nCoordNum = -1;

		for (int i = 0; i < m_nSection; i++)
		{
			//本端圆与下一圆面的中心点，和半径
			osg::Vec3d fromCenter = osg::Vec3d(dLenght, 0.0, 0.0)*(i*1.0 / m_nSection);
			osg::Vec3d toCenter = osg::Vec3d(dLenght, 0.0, 0.0)*((i + 1.0) / m_nSection);
			float fromRadius = m_dStartRadius + (m_dEndRadius - m_dStartRadius)*((i*1.0) / m_nSection);
			float toRadius = m_dStartRadius + (m_dEndRadius - m_dStartRadius)*((i + 1.0) / m_nSection);

			double dDeltaAngle = osg::PI * 2 / m_nDensity;
			for (int j = 0; j <= m_nDensity; j++)
			{
				osg::Vec3d fromPos = osg::Vec3d(fromCenter.x(), fromRadius*cos(j*dDeltaAngle), fromRadius*sin(j*dDeltaAngle));
				osg::Vec3d toPos = osg::Vec3d(toCenter.x(), toRadius*cos(j*dDeltaAngle), toRadius*sin(j*dDeltaAngle));

				pCoord->push_back(fromPos);
				pColor->push_back(osg::Vec4d(m_color.r(), m_color.g(), m_color.b(), fromAlpha));

				pCoord->push_back(toPos);
				pColor->push_back(osg::Vec4d(m_color.r(), m_color.g(), m_color.b(), toAlpha));
			}

			fromAlpha = toAlpha;
			toAlpha += deltaAlpha;

			if (toAlpha > m_dEndAlpha)
			{
				deltaAlpha = -deltaAlpha;
			}

			if (toAlpha < m_dStartAlpha)
			{
				deltaAlpha = -deltaAlpha;
			}
			
			if (nCoordNum < 0)
			{
				nCoordNum = pCoord->size();
			}
			m_rpGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, pCoord->size() - nCoordNum, nCoordNum));
		}

		m_rpMatrixTransform->setMatrix(
			osg::Matrix::rotate(osg::Vec3d(1.0, 0.0, 0.0), m_vecEndPoint - m_vecStartPoint)
			*osg::Matrix::translate(m_vecStartPoint));

		m_bUpdate = false;
	}

	void CMutableCylinder::SetColor(osg::Vec4d color)
	{
		m_color = color;
		m_bUpdate = true;
		Update();
	}

	void CMutableCylinder::SetPoint(osg::Vec3d posStart, osg::Vec3d posEnd)
	{
		if(m_vecStartPoint == posStart && m_vecEndPoint == posEnd)
		{
			return;
		}
		m_vecStartPoint = posStart;
		m_vecEndPoint = posEnd;

		m_bUpdatePos = true;
		//m_bUpdate = true;
		//Update();
	}

	void CMutableCylinder::GetPoint(osg::Vec3d &posStart, osg::Vec3d &posEnd)
	{
		posStart = m_vecStartPoint;
		posEnd = m_vecEndPoint;
	}

	void CMutableCylinder::SetAlpha(double dStartAlpha, double dEndAlpha)
	{
		if(m_dStartAlpha == dStartAlpha && m_dEndAlpha == dEndAlpha)
		{
			return;
		}
		m_dStartAlpha = dStartAlpha;
		m_dEndAlpha = dEndAlpha;

		m_bUpdate = true;
		Update();
	}

	void CMutableCylinder::SetColorTransform(bool bChanged)
	{
		m_bColorTrans = bChanged;
	}

	void CMutableCylinder::SetFrameDelta(int nFrameDelta)
	{
		m_nFrameDelta = nFrameDelta;
	}

	void CMutableCylinder::SetSection(int nSection)
	{
		if(m_nSection == nSection)
		{
			return;
		}
		m_nSection = nSection;

		m_bUpdate = true;
		Update();
	}

	int CMutableCylinder::GetSection()
	{
		return m_nSection;
	}

	void CMutableCylinder::SetSectionPoint(int nPoint)
	{
		m_nDensity = nPoint;

		m_bUpdate = true;
		Update();
	}

	int CMutableCylinder::GetSectionPoint()
	{
		return m_nDensity;
	}

	void CMutableCylinder::SetRadius( double dStartRadius,double dEndRadius )
	{
		m_dStartRadius = dStartRadius;
		m_dEndRadius = dEndRadius;
		m_bUpdate = true;
		Update();
	}

	///////////////////////////////////////////////////////
	//CMutableCylinderCallback
	///////////////////////////////////////////////////////
	CMutableCylinderCallback::CMutableCylinderCallback(CMutableCylinder* pCylinder)
		:m_opCylinder(pCylinder)
		, m_nCurrentFrame(0)
	{
	}

	CMutableCylinderCallback::~CMutableCylinderCallback()
	{

	}

	void CMutableCylinderCallback::drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const
	{
		if (m_opCylinder->m_bUpdate)
		{
			return;
		}

		m_nCurrentFrame++;

		if (m_opCylinder->m_bColorTrans)
		{
			if (m_nCurrentFrame > m_opCylinder->m_nFrameDelta)
			{
				m_nCurrentFrame = 0;

				osg::ref_ptr<osg::Vec4dArray> m_colorArray = dynamic_cast<osg::Vec4dArray*>(m_opCylinder->m_rpGeom->getColorArray());
				if (m_colorArray.valid())
				{
					int nNumPoint = m_opCylinder->m_nDensity * 2 + 2;

					osg::Vec4d colorM0From = m_colorArray->at(m_colorArray->size() - 1); 
					int sizeColor = m_colorArray->size() / nNumPoint;

					for (int i = (sizeColor - 1); i >= 0; i--)
					{
						osg::Vec4d m1 = m_colorArray->at(i * nNumPoint);
						osg::Vec4d mNext1;
						if (i == 0)
						{
							mNext1 = colorM0From;
						}
						else
						{
							mNext1 = m_colorArray->at((i - 1) * nNumPoint);
						}

						for (int j = 0; j < (nNumPoint / 2); j++)
						{
							m_colorArray->at(i * nNumPoint + j * 2 + 1).set(m1.r(), m1.g(), m1.b(), m1.a());
							m_colorArray->at(i * nNumPoint + j * 2 + 0).set(mNext1.r(), mNext1.g(), mNext1.b(), mNext1.a());
						}
					}

					m_colorArray->dirty();   
				}
			}

			if(m_opCylinder->m_bUpdatePos)
			{
				m_opCylinder->m_bUpdatePos = false;
				//设置顶点
				osg::Vec3dArray* vertex = (osg::Vec3dArray*)m_opCylinder->m_rpGeom->getVertexArray();
				vertex->clear();
				vertex->dirty();

				//首先确定其长度
				double len = (m_opCylinder->m_vecEndPoint-m_opCylinder->m_vecStartPoint).length();

				int numPoint = -1;

				int jieN = m_opCylinder->m_nSection;

				for(int i = 0; i<jieN; i++)
				{
					//本端圆与下一圆面的中心点，和半径
					osg::Vec3d fromCenter = osg::Vec3d(len, 0.0, 0.0)*(i*1.0/jieN);
					osg::Vec3d toCenter = osg::Vec3d(len, 0.0, 0.0)*((i+1.0)/jieN);
					float fromRadius = m_opCylinder->m_dStartRadius + (m_opCylinder->m_dEndRadius - m_opCylinder->m_dStartRadius)*((i*1.0)/jieN);
					float toRadius = m_opCylinder->m_dStartRadius + (m_opCylinder->m_dEndRadius - m_opCylinder->m_dStartRadius)*((i+1.0)/jieN);

					double dDeltaAngle = osg::PI * 2 / m_opCylinder->m_nDensity;
					for(int j = 0; j<=m_opCylinder->m_nDensity; j++) 
					{
						//本端圆
						//osg::Vec3d fromPos = osg::Vec3d(fromCenter.x(), fromRadius*cos(osg::inDegrees(j*dDeltaAngle)), fromRadius*sin(osg::inDegrees(j*dDeltaAngle)));
						//osg::Vec3d toPos = osg::Vec3d(toCenter.x(), toRadius*cos(osg::inDegrees(j*dDeltaAngle)), toRadius*sin(osg::inDegrees(j*dDeltaAngle)));

						osg::Vec3d fromPos = osg::Vec3d(fromCenter.x(), fromRadius*cos(j*dDeltaAngle), fromRadius*sin(j*dDeltaAngle));
						osg::Vec3d toPos = osg::Vec3d(toCenter.x(), toRadius*cos(j*dDeltaAngle), toRadius*sin(j*dDeltaAngle));

						vertex->push_back(fromPos);

						vertex->push_back(toPos);
					}
				}

				m_opCylinder->m_rpGeom->dirtyBound();
				m_opCylinder->m_rpMatrixTransform->setMatrix(osg::Matrix::rotate(osg::Vec3d(1.0, 0.0, 0.0), (m_opCylinder->m_vecEndPoint-m_opCylinder->m_vecStartPoint))*osg::Matrix::translate(m_opCylinder->m_vecStartPoint));
			}
			
		}

		drawable->drawImplementation(renderInfo);
	}

}