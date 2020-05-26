
#include <osg/PrimitiveSet>
#include <osg/AnimationPath>
#include <osg/LineWidth>
#include <osg/MatrixTransform>
#include <osg/Depth>

#include <FeEffects/LineConnect.h>

namespace FeEffect
{
	CLineConnect::CLineConnect( osg::Vec3d start, osg::Vec3d end, float startAlpha, float endAlpha, int lineWidth)
		:osg::Group()
		,m_color(osg::Vec4(0.0, 1.0, 1.0, 1.0))
		,m_start(start)
		,m_end(end)
		,m_startAlpha(startAlpha)
		,m_endAlpha(endAlpha)
		,m_rpTransMT(NULL)
		,m_rpLineDrawCallback(NULL)
	{
		this->getOrCreateStateSet()->setAttribute(new osg::LineWidth(lineWidth), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		m_rpTransMT = new osg::MatrixTransform;
		addChild(m_rpTransMT);
	}

	void CLineConnect::Init()
	{
		m_rpTransMT->removeChild(0,m_rpTransMT->getNumChildren());

		this->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		this->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		//c00005 at huayin
		//this->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		//this->getOrCreateStateSet()->setMode(GL_SMOOTH, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		osg::ref_ptr<osg::Depth> pDepth = new osg::Depth();
		pDepth->setWriteMask(false);
		this->getOrCreateStateSet()->setAttribute(pDepth, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		m_gnode = new osg::Geode;
		m_rpTransMT->addChild(m_gnode);

		m_geom = new osg::Geometry;
		m_gnode->addDrawable(m_geom);
		m_rpLineDrawCallback = new LineDrawCallback(this,m_start, m_end,m_color);
		m_geom->setDrawCallback(m_rpLineDrawCallback.get());
		m_geom->setUseDisplayList(false);
		m_geom->setUseVertexBufferObjects(true);

		//顶点序列
		osg::Vec3dArray* vertex = new osg::Vec3dArray;
		m_geom->setVertexArray(vertex);

		//颜色序列
		osg::Vec4dArray* colorArray = new osg::Vec4dArray;
		m_geom->setColorArray(colorArray);
		m_geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		//CCommon::Instance()->InterpolationByLLH_N_XYZ(m_start, m_end, vertex, 100);
		int nTotalNum = 100;

		double dlen = (m_end - m_start).length();
		for(int i = 0; i<nTotalNum; i++)
		{
			osg::Vec3d fromCenter = osg::Vec3d(dlen, 0.0, 0.0)*(i*1.0/nTotalNum);
			vertex->push_back(fromCenter);
		}

		osg::Vec3d toCenter = osg::Vec3d(dlen, 0.0, 0.0);
		vertex->push_back(toCenter);


		//100等分，分成10段，分别压入颜色
		float deltaColorAlpha = (m_endAlpha-m_startAlpha)/3;

		for(int i = 0; i<vertex->size(); i++)
		{
			colorArray->push_back(osg::Vec4d(m_color.r(), m_color.g(), m_color.b(), m_startAlpha + (i%10)*deltaColorAlpha));
		}

		m_geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, vertex->size()));

		m_rpTransMT->setMatrix(osg::Matrix::rotate(osg::Vec3d(1.0, 0.0, 0.0), m_end-m_start)*osg::Matrix::translate(m_start));


	}

	void CLineConnect::SetStartAndEnd( osg::Vec3d start, osg::Vec3d end )
	{
		m_start = start;
		m_end = end;
		Update();
		LineDrawCallback* lcb = (LineDrawCallback*)(m_geom->getDrawCallback());
		if(lcb != NULL)
		{
			lcb->SetStartAndEnd(start, end);
		}
	}

	void CLineConnect::SetColor( osg::Vec4d color )
	{
		m_color = color;
		if(m_rpLineDrawCallback.valid())
		{
			m_rpLineDrawCallback->SetLineColor(color);
		}
	}

	void CLineConnect::SetStart( osg::Vec3d start )
	{
		m_start = start;

		LineDrawCallback* lcb = (LineDrawCallback*)(m_geom->getDrawCallback());
		if(lcb != NULL)
		{
			lcb->SetStartAndEnd(start, m_end);
		}
	}

	void CLineConnect::SetEnd( osg::Vec3d end )
	{
		m_end = end;

		LineDrawCallback* lcb = (LineDrawCallback*)(m_geom->getDrawCallback());
		if(lcb != NULL)
		{
			lcb->SetStartAndEnd(m_start, end);
		}
	}

	void CLineConnect::Update()
	{

		osg::Vec4dArray* colorArray = (osg::Vec4dArray*)m_geom->getColorArray();
		colorArray->dirty();

		osg::Vec4d endC = colorArray->at(colorArray->size()-1);

		for(int i = colorArray->size()-1; i >0; i--)
		{
			colorArray->at(i).set(m_color.x(), m_color.y(), m_color.z(), colorArray->at(i-1).a());
		}

		colorArray->at(0).set(m_color.r(), m_color.g(), m_color.b(), endC.a());

		//////////////////////////
		//设置顶点
		osg::Vec3dArray* vertex = (osg::Vec3dArray*)m_geom->getVertexArray();
		vertex->clear();
		vertex->dirty();

		double dlen = (m_end - m_start).length();
		int nTotalNum =100;
		for(int i = 0; i<nTotalNum; i++)
		{
			osg::Vec3d fromCenter = osg::Vec3d(dlen, 0.0, 0.0)*(i*1.0/nTotalNum);
			vertex->push_back(fromCenter);
		}

		osg::Vec3d toCenter = osg::Vec3d(dlen, 0.0, 0.0);
		vertex->push_back(toCenter);

		m_rpTransMT->setMatrix(osg::Matrix::rotate(osg::Vec3d(1.0, 0.0, 0.0), m_end-m_start)*osg::Matrix::translate(m_start));

		m_geom->dirtyDisplayList();
		m_geom->dirtyBound();
	}
}

namespace FeEffect
{
	LineDrawCallback::LineDrawCallback(CLineConnect *pLinkLine,osg::Vec3d start, osg::Vec3d end,osg::Vec4d vecColor)
		:osg::Drawable::DrawCallback(),
	m_start(start),
	m_end(end),
	m_opLinkLine(pLinkLine),
	m_nframeTiaoGuo(0),
	m_vecColor(vecColor)
	{

	}

	void LineDrawCallback::SetStartAndEnd( osg::Vec3d start, osg::Vec3d end )
	{
		m_start = start;
		m_end = end;
	}

	void LineDrawCallback::drawImplementation( osg::RenderInfo& renderInfo,const osg::Drawable* drawable ) const
	{
		//两帧来一次
		//static int frameTiaoGuo = 0;
	
		m_nframeTiaoGuo ++;
	
		if(m_nframeTiaoGuo>2)
		{
			m_nframeTiaoGuo = 0;
	
			osg::Geometry* m_geom = (osg::Geometry*)drawable;
	
			osg::Vec4dArray* m_colorArray = (osg::Vec4dArray*)m_geom->getColorArray();
			m_colorArray->dirty();
	
			osg::Vec4d endC = m_colorArray->at(m_colorArray->size()-1);
	
			for(int i = m_colorArray->size()-1; i >0; i--)
			{
				m_colorArray->at(i).set(m_vecColor.x(), m_vecColor.y(), m_vecColor.z(), m_colorArray->at(i-1).a());
			}
	
			m_colorArray->at(0).set(m_vecColor.r(), m_vecColor.g(), m_vecColor.b(), endC.a());
	
			//////////////////////////
			//设置顶点
			osg::Vec3dArray* vertex = (osg::Vec3dArray*)m_geom->getVertexArray();
			vertex->clear();
			vertex->dirty();
	
			double dlen = (m_end - m_start).length();
			int nTotalNum =100;
			for(int i = 0; i<nTotalNum; i++)
			{
				osg::Vec3d fromCenter = osg::Vec3d(dlen, 0.0, 0.0)*(i*1.0/nTotalNum);
				vertex->push_back(fromCenter);
			}

			osg::Vec3d toCenter = osg::Vec3d(dlen, 0.0, 0.0);
			vertex->push_back(toCenter);

			if(m_opLinkLine.valid())
			{
				m_opLinkLine->m_rpTransMT->setMatrix(osg::Matrix::rotate(osg::Vec3d(1.0, 0.0, 0.0), m_end-m_start)*osg::Matrix::translate(m_start));
				  
			}

			m_geom->dirtyDisplayList();
			m_geom->dirtyBound();
	
		}
	
		drawable->drawImplementation(renderInfo);
	}

	void LineDrawCallback::SetLineColor( osg::Vec4d vecColor )
	{
		m_vecColor = vecColor;
	}

}


