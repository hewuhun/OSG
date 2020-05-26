#include <FeUtils/UtilityGeom.h>
#include <FeUtils/CoordConverter.h>
#include <FeExtNode/ExBillBoardNode.h>
#include <FeExtNode/ExPointNodeEditor.h>
#include <FeExtNode/ExternNodeEditing.h>

namespace FeExtNode
{
	CExBillBoardNodeOption::CExBillBoardNodeOption()
		:CExPointNodeOption()
	{
		m_vecSize = osg::Vec2(1, 1);
	}

	CExBillBoardNodeOption::~CExBillBoardNodeOption()
	{

	}

}

namespace FeExtNode
{
	const float SQURE_2 = 1.414;  //\/2

	CExBillBoardNode::CExBillBoardNode( FeUtil::CRenderContext* pRender, CExBillBoardNodeOption* opt )
		:CExPointNode(pRender, opt)
		,m_rpBillBoardNode(NULL)
		,m_dOriginWidth(1.0)
		,m_dOriginHeight(1.0)
	{
		m_opTransformMT = new osg::MatrixTransform;
		addChild(m_opTransformMT);

		m_opScaleMT = new osg::MatrixTransform();
		m_opTransformMT->addChild(m_opScaleMT);

		m_rpBillBoardNode = new osg::Billboard();
		m_rpBillBoardNode->getOrCreateStateSet()->setMode(GL_LIGHTING, 0);
		m_rpBillBoardNode->getOrCreateStateSet()->setMode(GL_BLEND, 1);
		m_rpBillBoardNode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		m_rpBillBoardNode->setMode(osg::Billboard::POINT_ROT_EYE);
		m_opScaleMT->addChild(m_rpBillBoardNode);

		UpdateImage();
		UpdatePosition();
	}

	CExBillBoardNode::~CExBillBoardNode()
	{

	}

	void CExBillBoardNode::SetName( const std::string& strName )
	{
		if(GetOption()->name() != strName)
		{
			GetOption()->name() = strName;

			m_rpBillBoardNode->setName(strName);
		}
	}

	void CExBillBoardNode::UpdateImage()
	{
		m_rpImageNode = osgDB::readImageFile(GetImagePath());

		if (m_rpImageNode.valid())
		{
			float fWidth = m_dOriginWidth;
			float fHeight = m_dOriginHeight;

			float fAspectRatio = m_rpImageNode->getPixelAspectRatio();

			if(fAspectRatio >= 1)
			{
				fHeight = fWidth / fAspectRatio; 
			}
			else
			{
				fWidth = fHeight * fAspectRatio;
			}

			osg::Drawable* pDrawable = FeUtil::CreateImageSquare(osg::Vec3(-fWidth/2, 0.0f, -fHeight/2), 
				osg::Vec3(fWidth, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, fHeight), m_rpImageNode.get());

			if(pDrawable)
			{
				m_rpBillBoardNode->removeDrawables(0, m_rpBillBoardNode->getNumDrawables());
				m_rpBillBoardNode->addDrawable(pDrawable);
				
			}
		}
	}

	std::string CExBillBoardNode::GetImagePath()
	{
		return GetOption()->imagePath();
	}

	void CExBillBoardNode::SetImagePath( const std::string& strImage )
	{
		if ( strImage != GetOption()->imagePath())
		{
			GetOption()->imagePath() = strImage;

			UpdateImage();
		}
	}

	osg::Vec2 CExBillBoardNode::GetImageSize()
	{
		return GetOption()->imageSize();
	}

	void CExBillBoardNode::SetImageSize( const osg::Vec2& vecSize )
	{
		if (GetOption()->imageSize() != vecSize)
		{
			GetOption()->imageSize() = vecSize;
			
			UpdatePosition();
		}
	}

	void CExBillBoardNode::SetPosition( const osgEarth::GeoPoint& geoPosition )
	{
		if(geoPosition != GetPosition())
		{
			GetOption()->geoPoint() = geoPosition;
			
			UpdatePosition();
			UpdateEditor();
		}
	}

	osgEarth::GeoPoint CExBillBoardNode::GetPosition()
	{
		return GetOption()->geoPoint();
	}
	
	CExBillBoardNodeOption* CExBillBoardNode::GetOption()
	{
		return dynamic_cast<CExBillBoardNodeOption*>(m_rpOptions.get());
	}

	void CExBillBoardNode::UpdatePosition()
	{
		osg::Matrix matrix;

		osg::Vec3d vecLLH = GetOption()->geoPoint().vec3d() 
			+ osg::Vec3d(0.0, 0.0, GetOption()->imageSize().y() * SQURE_2 * 0.5);

		FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), vecLLH, matrix);
		m_opTransformMT->setMatrix(matrix);

		m_opScaleMT->setMatrix(osg::Matrix::scale(
			GetOption()->imageSize().x(), GetOption()->imageSize().y(), GetOption()->imageSize().y()));
	}

	void CExBillBoardNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	FeNodeEditor::CFeNodeEditTool* CExBillBoardNode::CreateEditorTool()
	{
		return new CExternNodeEditing(m_opRenderContext.get(), this, CExternNodeEditing::EDITOR_TRANS);
	}

	void CExBillBoardNode::DraggerStart()
	{
		m_pointStartDrag = GetPosition();
	}

	void CExBillBoardNode::DraggerUpdateMatrix(const osg::Matrix& matrix)
	{
		osg::Matrix mxStart;
		FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), m_pointStartDrag.vec3d(), mxStart);
		
		/// Trans
		osg::Matrix mxTrans = mxStart * osg::Matrix::translate(matrix.getTrans());
		
		osgEarth::GeoPoint pt;
		pt.fromWorld(m_pointStartDrag.getSRS(), mxTrans.getTrans());
		GetOption()->geoPoint() = pt	;

		/// Scale
// 		osg::Vec3d vecScale = matrix.getScale();
// 		osg::Vec2 vecSize = g_sizeStartDrag;
// 
// 		vecSize.x() *= vecScale.x();
// 		if(vecScale.y() > 1)
// 		{
// 			vecSize.y() *= vecScale.y();
// 		}
// 		else if(vecScale.z() > 1)
// 		{
// 			vecSize.y() *= vecScale.z();
// 		}
// 
// 		GetOption()->imageSize() = vecSize;

		UpdatePosition();
	}

	osg::Vec3d CExBillBoardNode::GetEditPointPositionXYZ()
	{
		return getBound().center();
	}

}
