#include <osg/Geometry>
#include <osg/Material>

#include <FeUtils/NodeUtils.h>

namespace FeUtil
{
	/**
	  * @class CNodeColorVisitor
	  * @brief 改变节点颜色的访问器
	  * @author c00005
	*/
	class CNodeColorVisitor : public osg::NodeVisitor
	{
	public:
		CNodeColorVisitor(const osg::Vec4d& vecClr)
			:osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
			, m_vecClr(vecClr)
		{
		}

	public:
		void apply(osg::Geometry &geometry)
		{
			float fRed = m_vecClr.r();
			float fGreen = m_vecClr.g();
			float fBlue = m_vecClr.b();

			osg::ref_ptr<osg::Vec4dArray> rpColor = new osg::Vec4dArray;
			rpColor->push_back(m_vecClr/*osg::Vec4d(fRed, fGreen, fBlue, 1.0)*/);
			geometry.setColorArray(rpColor);
			geometry.setColorBinding(osg::Geometry::BIND_OVERALL);

			osg::ref_ptr<osg::StateSet> state=geometry.getOrCreateStateSet();
			state->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
			state->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

			osg::ref_ptr<osg::Material> rpMaterial = new osg::Material;
			rpMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.3*fRed, 0.2*fGreen, 0.1*fBlue, 1.0));		//环境
			rpMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(fRed, fGreen, fBlue, 1.0));					//漫反射
			rpMaterial->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.4*fRed, 0.5*fGreen, 0.4*fBlue, 1.0));	//镜面反射
			rpMaterial->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0.1*fRed, 0.2*fGreen, 0.3*fBlue, 1.0));	//散射
			rpMaterial->setShininess(osg::Material::FRONT_AND_BACK, 30);
			state->setAttribute(rpMaterial.get());

			traverse(geometry);
		}

	public:
		osg::Vec4d m_vecClr;
	};



	bool SetNodeVisible(osg::Node* pNode, bool bVisible)
	{
		if (pNode)
		{
			if (bVisible)
			{
				pNode->setNodeMask(0xffffffff);
			}
			else
			{
				pNode->setNodeMask(0x00000000);
			}

			return true;
		}

		return false;
	}

    void ChangeNodeColor( osg::Node* pNode, const osg::Vec4d& vecClr )
	{
		if(pNode)
		{
			CNodeColorVisitor clrVisitor(vecClr);
			pNode->accept(clrVisitor);
		}
	}

}
