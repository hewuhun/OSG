/**************************************************************************************************
* @file CExternNode.h
* @note 外部扩展节点的接口类
* @author c00005
* @data 2016-7-4
**************************************************************************************************/
#ifndef FE_EXTERN_LOD_NODE_H
#define FE_EXTERN_LOD_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExternNode.h>

using namespace FeUtil;
using namespace osgEarth;

namespace FeExtNode
{
	/**
	* @class CExLodNodeOption
	* @note Lod节点的配置选项
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExLodNodeOption : public CExternNodeOption
	{
	public:
		/**  
		  * @note Lod节点的配置选项的构造函数，如果使用默认值，则节点始终在视距范围内
		  * @param dMinRange [in] 节点的最小可视视距
		  * @param dMaxRange [in] 节点的最大可视视距
		*/
		CExLodNodeOption(const double& dMinRange=-FLT_MAX, const double& dMaxRange=FLT_MAX);

		/**  
		  * @note 析构函数
		*/
		virtual ~CExLodNodeOption();

	public:
		/**  
		  * @note 获取节点的最小可视视距
		*/
		double& minRange() { return m_dMinRange; }
		
		/**  
		  * @note 获取节点的最大可视视距
		*/
		double& maxRange() { return m_dMaxRange; }

		/**  
		  * @note 获取用户设置的节点中心点
		*/
		osg::Vec3d& userCenter() { return m_vecUserCenter; }

		/**  
		  * @note 是否使用用户设置的中心点
		*/
		bool& isUserCetner() { return m_bUserCenter; }

	protected:
		///节点的最小可视视距
		double						m_dMinRange;

		///节点的最大可视视距
		double						m_dMaxRange;

		///用户设置的节点的中心点
		osg::Vec3d					m_vecUserCenter;

		///是否使用用户设置的中心点
		bool						m_bUserCenter;
	};
}

namespace FeExtNode
{
	/**
	* @class CExLodNodeOption
	* @note Lod节点，主要用于具有LOD功能的节点，可以设置最大最小视距
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExLodNode : public CExternNode
	{
	public:
		/**  
		  * @note Lod节点的构造函数
		  * @param opt [in] Lod节点的配置属性集合
		*/
		CExLodNode(CExLodNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExLodNode();

	public:
		/**  
		  * @note 设置节点的最小最大的可视视距
		  * @param dMinRange [in] Lod节点的最小视距
		  * @param dMaxRange [in] Lod节点的最大视距
		*/
		virtual void SetRange(const double& dMinRange, const double& dMaxRange);

		/**  
		  * @note 获取节点的最小最大的可视视距
		  * @param dMinRange [out] 返回Lod节点的最小视距
		  * @param dMaxRange [out] 返回Lod节点的最大视距
		*/
		virtual void GetRange(double& dMinRange, double& dMaxRange);

		/**  
		  * @note 设置节点的中心点
		  * @param center [in] 用户设置的节点中心点
		  * @param bUserCenter [in] 是否使用用户设置的中心点，true为使用，false为使用默认的中心点
		*/
		virtual void SetCenter(osg::Vec3d center, bool bUserCenter=true);

		/**  
		  * @note 获取节点的中心点
		  * @return osg::Vec3d 节点的中心点
		*/
		virtual osg::Vec3d GetCenter();
				
		/**  
		  * @note 设置节点是否可见
		*/
		//virtual void SetVisible(const bool& bVisible);

		/**  
		  * @note 设置根据LOD判断的是否显示或者隐藏，
		  *		  当LOD判断显示时，还需要判断用户设置的是否为隐藏
		*/
		virtual void SetLodVisible(bool bVisible);

		/**
		* @note 获取根据LOD判断的是否显示或者隐藏
		*/
		virtual bool GetLodVisible();

	public:
		/**  
		  * @note 继承自osg::Group,用于在每一帧中遍历节点，
		  *		  进而判断节点是否在可视视距范围内，从而控制显示或者隐藏
		*/
		virtual void traverse(osg::NodeVisitor& nv);

		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExLodNodeOption* GetOption();

	protected:
		/**  
		  * @note 更新节点的掩码，用于显示或者接受事件状态改变时调用
		*/
		virtual void UpdateVisibleAndEvent();

	protected:
		///用于保存根据LOD判断出来的是否显示或者隐藏
		bool					m_bLodVisible;
	};
}


#endif //FE_EXTERN_LOD_NODE_H
