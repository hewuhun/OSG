#include <osgEarthAnnotation/Draggers>
#include <FeUtils/RenderContext.h>

#include <FeExtNode/ExFeatureNode.h>
#include <ExternNodeEditing.h>
#include <FeUtils/CoordConverter.h>

const osg::Vec4 g_editPtColor(0.6, 1.0, 0.5, 1);
const osg::Vec4 g_editPtPickColor(0, 1.0, 0, 1);
const osg::Vec4 g_editPtActiveColor(1, 0.2, 0.1, 1);

namespace FeExtNode
{
	CExFeatureNodeOption::CExFeatureNodeOption()
		:CExLodNodeOption()
	{
		m_vecLineColor = osg::Vec4d(1.0, 1.0, 0.0, 1.0);
		m_vecFillColor = osg::Vec4d(1.0, 0.0, 1.0, 1.0);
		m_nLineWidth = 1;
		m_unLineStipple = 0xFFFFFFFF;
	}

	CExFeatureNodeOption::~CExFeatureNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExFeatureNode::CExFeatureNode( FeUtil::CRenderContext* pRender, CExFeatureNodeOption* opt )
		:CExLodNode(opt)
		,m_opRenderContext(pRender)
		,m_nCurEditPoint(0)
	{
		if(m_opRenderContext.valid())
		{
			m_opMapNode = m_opRenderContext->GetMapNode();
		}
	
	}

	CExFeatureNode::~CExFeatureNode()
	{
	}

	bool CExFeatureNode::InitFeatureNode()
	{
		if(m_rpFeatureNode.valid())
		{
			/// 已存在则先移除，防止多次调用生成多个节点
			this->removeChild(m_rpFeatureNode);
		}

		m_rpFeature = GetFeature();
		osgEarth::Symbology::Style style;
		osgEarth::Features::GeometryCompilerOptions options;
		options.maxGranularity() = 180.0;
		m_rpFeatureNode = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpFeature, style, options);

		if(m_rpFeatureNode)
		{
			m_rpFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			m_rpFeatureNode->setStyle(GetFeatureStyle());
			m_rpFeatureNode->setDynamic(true);
			addChild(m_rpFeatureNode.get());
			return true;
		}
		
		return false;
	}

	void CExFeatureNode::SetVertex( osg::Vec3dArray* pCoord )
	{
		if(m_rpFeature.valid() && m_rpFeatureNode.valid())
		{
			m_rpFeature->getGeometry()->asVector() = pCoord->asVector();
			m_rpFeatureNode->init();
			//RebuildEditPoints();
		}
	}

	void CExFeatureNode::GetVertex(osg::Vec3dArray* pCoord)
	{
		if(m_rpFeature.valid() && pCoord)
		{
			osg::ref_ptr<osg::Vec3dArray> vecArray = m_rpFeature->getGeometry()->toVec3dArray();

			if(vecArray.valid())
			{
				pCoord->insert(pCoord->begin(), vecArray->begin(), vecArray->end());
			}
		}
	}

	bool CExFeatureNode::PushBackVertex( const osg::Vec3d& vecCoord )
	{
		if(m_rpFeature.valid())
		{
			m_rpFeature->getGeometry()->asVector().push_back(vecCoord);
			m_rpFeatureNode->init();
			RebuildEditPoints();

			return true;
		}
		else
		{
			return false;
		}
	}

	osg::Vec3d CExFeatureNode::Back()
	{
		if(m_rpFeature.valid() && m_rpFeature->getGeometry()->asVector().size() > 0)
		{
			return m_rpFeature->getGeometry()->asVector().back();
		}
		return osg::Vec3d();
	}

	void CExFeatureNode::PushFrontVertex( const osg::Vec3d& vecCoord )
	{
		if(m_rpFeature.valid())
		{
			m_rpFeature->getGeometry()->asVector().insert(
				m_rpFeature->getGeometry()->asVector().begin(), vecCoord);
			m_rpFeatureNode->init();
			RebuildEditPoints();
		}
	}

	osg::Vec3d CExFeatureNode::Front()
	{
		if(m_rpFeature.valid())
		{
			return m_rpFeature->getGeometry()->asVector().front();
		}
		return osg::Vec3d();
	}

	void CExFeatureNode::InsertVertex( int nIndex, const osg::Vec3d& vecCoord )
	{
		if(m_rpFeature.valid())
		{
			if(nIndex > m_rpFeature->getGeometry()->asVector().size() || nIndex < 0)
			{
				return;
			}
	
			m_rpFeature->getGeometry()->asVector().insert(
				m_rpFeature->getGeometry()->asVector().begin()+nIndex, vecCoord);

			m_rpFeatureNode->init();
			RebuildEditPoints();
		}
	}

	void CExFeatureNode::ReplaceVertex( int nIndex, const osg::Vec3d& vecCoord )
	{
		if(m_rpFeature.valid())
		{
			if(nIndex >= m_rpFeature->getGeometry()->asVector().size() || nIndex < 0)
			{
				return;
			}

			m_rpFeature->getGeometry()->asVector()[nIndex] = vecCoord;
			m_rpFeatureNode->init();
			UpdateEditor();
		}
	}

	bool CExFeatureNode::DeleteVertex( int nIndex )
	{
		if(!m_rpFeature.valid())
		{
			return false;
		}
		else if(nIndex >= m_rpFeature->getGeometry()->asVector().size() || nIndex < 0)
		{
			return false;
		}

		m_rpFeature->getGeometry()->asVector().erase(
			m_rpFeature->getGeometry()->asVector().begin()+nIndex);

		m_rpFeatureNode->init();
		RebuildEditPoints();
		return true;
	}

	int CExFeatureNode::GetNumVertex()
	{
		if(m_rpFeature.valid())
		{
			return m_rpFeature->getGeometry()->asVector().size();
		}
		return 0;
	}

	void CExFeatureNode::SetLineColor( const osg::Vec4d& vecLineColor )
	{
		if (GetLineColor() != vecLineColor)
		{
			GetOption()->lineColor() = vecLineColor;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = 
				osgEarth::Symbology::Color(vecLineColor);

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
		}
	}

	osg::Vec4d CExFeatureNode::GetLineColor()
	{
		return GetOption()->lineColor();
	}

	void CExFeatureNode::SetFillColor( const osg::Vec4d& vecFillColor )
	{
		if (GetFillColor() != vecFillColor)
		{
			GetOption()->fillColor() = vecFillColor;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() = 
				osgEarth::Symbology::Color(vecFillColor);

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
		}
	}

	osg::Vec4d& CExFeatureNode::GetFillColor()
	{
		return GetOption()->fillColor();
	}

	void CExFeatureNode::SetLineWidth( const int& nLineWidth )
	{
		if (GetLineWidth() != nLineWidth)
		{
			GetOption()->lineWidth() = nLineWidth;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = nLineWidth;

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
		}
	}

	int CExFeatureNode::GetLineWidth()
	{
		return GetOption()->lineWidth();
	}

	void CExFeatureNode::SetLineStipple( const unsigned int& unLineStipple )
	{
		if (GetLineStipple() != unLineStipple)
		{
			GetOption()->lineStipple() = unLineStipple;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = unLineStipple;

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
		}
	}

	unsigned int CExFeatureNode::GetLineStipple()
	{
		return GetOption()->lineStipple();
	}

	CExFeatureNodeOption* CExFeatureNode::GetOption()
	{
		return dynamic_cast<CExFeatureNodeOption*>(m_rpOptions.get());
	}

	osg::Group* CExFeatureNode::CreateEditPoints()
	{
		osg::Group* rootNode = new osg::Group;

		if(m_rpFeature.valid())
		{
			/// create a editPoint for each point
			osg::ref_ptr<osg::Vec3dArray> vecArray = m_rpFeature->getGeometry()->toVec3dArray();

			for (unsigned int i = 0; i < vecArray->size(); i++)
			{
				FeNodeEditor::CFeEditPoint* editPoint = new FeNodeEditor::CFeEditPoint(m_opMapNode.get(), this, i);
				if(!editPoint) continue;

				editPoint->SetPosition(GeoPoint(m_rpFeature->getSRS()
					,  vecArray->at(i).x()
					,  vecArray->at(i).y()
					, vecArray->at(i).z() +5
					, osgEarth::ALTMODE_ABSOLUTE));

				editPoint->SetColor(g_editPtColor);

				rootNode->addChild(editPoint);   
			}
		}

		return rootNode;
	}

	bool CExFeatureNode::StartEditImplement()
	{
		// 创建编辑点
		if(!m_rpEditPoints.valid())
		{
			m_rpEditPoints = CreateEditPoints();
			addChild(m_rpEditPoints);
			
			/// active last point
			FeNodeEditor::CFeEditPoint* pointActive = GetEditPoint(m_rpEditPoints->getNumChildren()-1);
			if(pointActive)
				ActiveEditPoint(*pointActive);
		}
		
		return CExLodNode::StartEditImplement();
	}

	bool CExFeatureNode::StopEditImplement()
	{
		// 移除编辑点
		if(m_rpEditPoints.valid())
		{
			removeChild(m_rpEditPoints);
			m_rpEditPoints.release();
		}

		return CExLodNode::StopEditImplement();
	}

	FeNodeEditor::CFeEditPoint* CExFeatureNode::GetEditPoint(unsigned int unIndex)
	{
		FeNodeEditor::CFeEditPoint* point = 0;
		if(m_rpEditPoints.valid() && unIndex < m_rpEditPoints->getNumChildren())
		{
			point = dynamic_cast<FeNodeEditor::CFeEditPoint*>(m_rpEditPoints->getChild(unIndex));
		}
		return point;
	}

	osgEarth::GeoPoint CExFeatureNode::GetEditPointPosition()
	{
		if(m_rpFeature.valid())
		{
			osg::ref_ptr<osg::Vec3dArray> vecArray = m_rpFeature->getGeometry()->toVec3dArray();

			if(vecArray.valid() && m_nCurEditPoint < vecArray->size())
			{
				return osgEarth::GeoPoint(m_opMapNode->getMapSRS(), vecArray->at(m_nCurEditPoint));
			}
		}
		return osgEarth::GeoPoint();
	}

	osg::Vec3d CExFeatureNode::GetEditPointPositionXYZ()
	{
// 		osg::Vec3d xyz;
// 		GetEditPointPosition().toWorld(xyz);
// 		return xyz;

		/// 折中解决方案，解决高程加载导致编辑位置到地形底下，进而导致编辑器被裁减
		osg::Vec3d llh, xyz;
		osgEarth::GeoPoint geoPt = GetEditPointPosition();
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(), geoPt.vec3d());
		geoPt.toWorld(xyz);
		return xyz;
	}

	void CExFeatureNode::ActiveEditPoint( const FeNodeEditor::CFeEditPoint& editPoint )
	{
		if(!m_rpEditPoints.valid())
		{
			return;
		}

		/// 恢复上一个编辑点颜色
		FeNodeEditor::CFeEditPoint* lastPoint = GetEditPoint(m_nCurEditPoint);
		if(lastPoint)
		{
			lastPoint->SetColor(g_editPtColor);
		}

		/// 设置当前编辑点颜色
		FeNodeEditor::CFeEditPoint* curPoint = GetEditPoint(editPoint.GetPointIndex());
		if(curPoint)
		{
			curPoint->SetColor(g_editPtActiveColor);
			m_nCurEditPoint = editPoint.GetPointIndex();

			/// 更新编辑器
			UpdateEditor();
		}
	}

	FeNodeEditor::CFeNodeEditTool* CExFeatureNode::CreateEditorTool()
	{
		return new CExFeatureNodeEditing(m_opRenderContext.get(), this);
	}

	void CExFeatureNode::DraggerStart()
	{
		m_pointStartDrag = GetEditPointPosition();
	}

	void CExFeatureNode::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		osg::Matrix matrixNew;
		m_pointStartDrag.createLocalToWorld(matrixNew);
		matrixNew = matrixNew * matrix;

		osgEarth::GeoPoint pt;
		pt.fromWorld(m_pointStartDrag.getSRS(), matrixNew.getTrans());
		//pt.z() = m_pointStartDrag.z();
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(), pt.vec3d());

		ReplaceVertex(m_nCurEditPoint, pt.vec3d());

		FeNodeEditor::CFeEditPoint* point = GetEditPoint(m_nCurEditPoint);
		if(point)
		{
			pt.z() += 5;
			point->SetPosition(pt);
		}
	}

	void CExFeatureNode::RebuildEditPoints()
	{
		if(!IsEditting()) return;

		StopEditImplement();
		StartEditImplement();
		UpdateEditor();
	}

}

