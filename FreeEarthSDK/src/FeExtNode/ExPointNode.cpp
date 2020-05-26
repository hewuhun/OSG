#include <FeUtils/CoordConverter.h>
#include <ExternNodeEditing.h>
#include <FeExtNode/ExPointNode.h>

namespace FeExtNode
{
	CExPointNodeOption::CExPointNodeOption()
		:CExLodNodeOption()
	{
		
	}

	CExPointNodeOption::~CExPointNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExPointNode::CExPointNode(FeUtil::CRenderContext* pRender,  CExPointNodeOption* opt )
		:CExLodNode(opt)
		, m_opRenderContext(pRender)
	{
		if(m_opRenderContext.valid())
		{
			m_opMapNode = m_opRenderContext->GetMapNode();
		}
	}

	CExPointNode::~CExPointNode()
	{

	}

	void CExPointNode::SetName( const std::string& strName )
	{
		if(GetName() == strName)
		{
			return ;
		}

		CExLodNode::SetName(strName);
	}

	void CExPointNode::SetPosition( const osgEarth::GeoPoint& geoPosition )
	{
		if(GetOption()->geoPoint() == geoPosition)
		{
			return;
		}

		GetOption()->geoPoint() = geoPosition;
	}

	osgEarth::GeoPoint CExPointNode::GetPosition()
	{
		return GetOption()->geoPoint();
	}

	FeNodeEditor::CFeNodeEditTool* CExPointNode::CreateEditorTool()
	{
		return new CExPointEditing(m_opRenderContext.get(), this);
	}
	
	void CExPointNode::DraggerStart()
	{
		m_pointStartDrag = GetPosition();
	}

	void CExPointNode::DraggerUpdateMatrix(const osg::Matrix& matrix)
	{
		osg::Matrix matrixNew;
		FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), m_pointStartDrag.vec3d(), matrixNew);
		matrixNew = matrixNew * matrix;

		osgEarth::GeoPoint pt;
		pt.fromWorld(m_pointStartDrag.getSRS(), matrixNew.getTrans());

		//pt.z() = GetPosition().z();
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(), pt.vec3d());

		SetPosition(pt);	
	}

	osg::Vec3d CExPointNode::GetEditPointPositionXYZ()
	{
// 		osg::Vec3d vecXYZ;
// 		GetPosition().toWorld(vecXYZ);
// 		return vecXYZ;

		/// 折中解决方案，解决高程加载导致编辑位置到地形底下，进而导致编辑器被裁减
		osg::Vec3d llh, xyz;
		osgEarth::GeoPoint geoPt = GetPosition();
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(), geoPt.vec3d());
		geoPt.toWorld(xyz);
		return xyz;
	}

	CExPointNodeOption* CExPointNode::GetOption()
	{
		return dynamic_cast<CExPointNodeOption*>(m_rpOptions.get());
	}

	osgEarth::MapNode* CExPointNode::GetMapNode()
	{
		return m_opMapNode.get();
	}

}



