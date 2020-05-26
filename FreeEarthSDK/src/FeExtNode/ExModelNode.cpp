#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osgEarthAnnotation/AnnotationEditing>
#include <osg/MatrixTransform>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/StrUtil.h>
#include <FeUtils/PathRegistry.h>

#include <FeExtNode/ExModelNode.h>

namespace FeExtNode
{
	CExModelNode::CExModelNode(FeUtil::CRenderContext* pRender, CExModelNodeOption* opt)
		:CExternNode(opt)
		,m_opRenderContext(pRender)
	{
		CreateElement();
	}

	CExModelNode::~CExModelNode()
	{
	}

	bool CExModelNode::CreateElement()
	{
		m_mxScale.makeScale(1, 1, 1);
		m_mxRotate.makeRotate(0, 0, 0, 0);
		m_mxTrans.makeTranslate(0,0,0);
		
		m_rpMtPosition = new osg::MatrixTransform;
		addChild(m_rpMtPosition.get());
		
		m_rpRotateMT = new osg::MatrixTransform;
		m_rpMtPosition->addChild(m_rpRotateMT.get());
		
// 		m_rpAutoScreenAT = new osg::AutoTransform;
// 		m_rpAutoScreenAT->setAutoScaleToScreen(true);
// 		m_rpAutoScreenAT->setMinimumScale(1.0);
// 		m_rpRotateMT->addChild(m_rpAutoScreenAT);

		m_rpScaleMT = new osg::MatrixTransform;
		//m_rpAutoScreenAT->addChild(m_rpScaleMT);
		m_rpRotateMT->addChild(m_rpScaleMT);
		
		m_pSceneNode = new osg::Group;
		m_rpScaleMT->addChild(m_pSceneNode);

		SetModelPath(GetOption()->ModelPath());
		SetPosition(GetOption()->Position());
		SetPosture(GetOption()->Posture());
		SetScale(GetOption()->Scale());

		return true;
	}

	const std::string& CExModelNode::GetModelPath() 
	{
		return GetOption()->ModelPath();
	}

	void CExModelNode::SetModelPath(const std::string& strModelPath)
	{
		osg::Node* newNode = osgDB::readNodeFile(strModelPath);
		if(newNode)
		{
			if(m_pModelNode.valid())
			{
				m_pSceneNode->removeChild(m_pModelNode.get());
			}
			m_pModelNode = newNode;
			m_pSceneNode->addChild(m_pModelNode.get());
			GetOption()->ModelPath() = strModelPath;
		}
	}

	void CExModelNode::SetPosition( const osg::Vec3d& vecPos )
	{
		GetOption()->Position() = vecPos;
		FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), vecPos, m_mxTrans);
		m_rpMtPosition->setMatrix(m_mxTrans);
	}

	osg::Vec3d& CExModelNode::GetPosition()
	{
		return GetOption()->Position();
	}

	void CExModelNode::SetPosture( const osg::Vec3d& vecPosture )
	{
		GetOption()->Posture() = vecPosture;
		m_mxRotate.makeRotate(
			osg::DegreesToRadians(vecPosture.x()), osg::Vec3d(1,0,0)
			, osg::DegreesToRadians(vecPosture.y()), osg::Vec3d(0,1,0)
			, osg::DegreesToRadians(vecPosture.z()), osg::Vec3d(0,0,1));
		m_rpMtPosition->setMatrix(m_mxRotate * m_mxTrans);
	}

	osg::Vec3d& CExModelNode::GetPosture()
	{
		return GetOption()->Posture();
	}

	void CExModelNode::SetScale( const osg::Vec3d& vecScale )
	{
		GetOption()->Scale() = vecScale;
		m_mxScale.makeScale(vecScale);
		m_rpScaleMT->setMatrix(m_mxScale);
	}

	osg::Vec3d& CExModelNode::GetScale()
	{
		return GetOption()->Scale();
	}

	osg::Node * CExModelNode::GetModelNode()
	{
		return m_pModelNode.get();
	}
		
	osg::MatrixTransform* CExModelNode::GetTransMT()
	{
		return m_rpMtPosition.get();
	}

	osg::Group* CExModelNode::GetSceneNode()
	{
		return m_pSceneNode.get();
	}

	osg::MatrixTransform* CExModelNode::GetRotateMT() const
	{
		return m_rpRotateMT.get();
	}
	
	osg::MatrixTransform* CExModelNode::GetScaleMT() const
	{
		return m_rpScaleMT.get();
	}
	
	void CExModelNode::SetModelColor( osg::Vec4d vecColor )
	{
		if(m_pModelNode.valid())
		{
			osg::StateSet* StateSet = m_pModelNode->getOrCreateStateSet();

			//打开混合融合模式
			StateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
			StateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
			StateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			//使用BlendFunc实现透明效果
			osg::ref_ptr<osg::BlendColor> blendColor =new osg::BlendColor(vecColor);
			osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc();
			StateSet->setAttributeAndModes(blendFunc,osg::StateAttribute::PROTECTED | osg::StateAttribute::ON );
			StateSet->setAttributeAndModes(blendColor,osg::StateAttribute::PROTECTED | osg::StateAttribute::ON );
			blendFunc->setSource(osg::BlendFunc::ONE_MINUS_CONSTANT_COLOR);
			blendFunc->setDestination(osg::BlendFunc::ONE_MINUS_CONSTANT_ALPHA);
			osg::Vec4d vecFull(1.0,1.0,1.0,1.0);
			osg::Vec4d VecColorNew = vecFull - vecColor;
			VecColorNew.a() = vecColor.a();
			blendColor->setConstantColor(VecColorNew);

			//如果没有在场景在选中模型，就将设置的颜色记录先来
			if (!m_bSelected)
			{
				m_vecColor = vecColor;
				m_bSetColorBeforSelected = true;
			}
		}
	}

	void CExModelNode::SetSelectedState( bool bSelected )
	{
		if(!m_pModelNode) return;

		m_bSelected = bSelected;
		if (m_bSelected)
		{
			SetModelColor(osg::Vec4d(0.0,1.0,1.0,0.3));
		}
		else
		{
			osg::StateSet* StateSet = m_pModelNode->getOrCreateStateSet();

			osg::Vec4d vecColor = osg::Vec4d(0.0,1.0,1.0,0.3);
			osg::ref_ptr<osg::BlendColor> blendColor =new osg::BlendColor(vecColor);
			osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc();
			StateSet->setAttributeAndModes(blendFunc, osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF );
			StateSet->setAttributeAndModes(blendColor,osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF  );
		    
			//如果在选中前对模型的颜色进行了设置，就在退出选中时，进行恢复
			if (m_bSetColorBeforSelected)
		    {
				SetModelColor(m_vecColor);
		    }
		}
	}

	bool CExModelNode::GetSelectedState() const
	{
		return m_bSelected;
	}

	void CExModelNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	CExModelNodeOption* CExModelNode::GetOption()
	{
		return dynamic_cast<CExModelNodeOption*>(m_rpOptions.get());
	}

}
