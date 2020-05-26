/**************************************************************************************************
* @file OceanSys.h
* @note 定义了海洋功能模块,用于创建并渲染海洋以及海洋的参数设置等
* @author c00005
* @data 2016-8-25
**************************************************************************************************/

#ifndef FE_OCEAN_SYSTEM_H
#define FE_OCEAN_SYSTEM_H

#include <FeOcean/Export.h>

#include <FeOcean/TritonNode.h>
#include <FeUtils/SceneSysCall.h>

namespace FeOcean
{
	using namespace FeUtil;

	const std::string OCEAN_SYSTEM_CALL_DEFAULT_KEY("OceanSystem");

    /**
      * @class COceanSys
      * @note 定义了海洋功能模块,用于创建并渲染海洋以及海洋的参数设置等
      * @author c00005
    */
	class FEOCEAN_EXPORT COceanSys : public CSceneSysCall
    {
    public:
		/**  
		  * @note 构造函数
		  * @param strTritonPath [in] 海洋创建所需的资源路径
		*/
        COceanSys(const std::string& strTritonPath);

		/**
		* @note 析构函数
		*/
        ~COceanSys();

    public:
		/**
		* @note 获取海洋节点
		*/
        TritonNode * GetOceanNode();

		/**
		* @note 设置大气节点
		*/
		void SetAtmosphere(osg::Node* pNode);

		/**
		* @note 设置太阳以及月亮节点
		*/
		void SetSunAndMoonNode(osg::Node* pSunNode, osg::Node* pMoonNode);
		
	protected:
		/**
        *@note: 实现本系统模型需要的初始化函数，此函数在父类中已经调用
        */
		virtual bool InitiliazeImplement();

		/**
		*@note: 实现本系统模型需要的反初始化函数，此函数在父类中已经调用
		*/
        virtual bool UnInitiliazeImplement();
		
	protected:
		///海洋节点
        osg::ref_ptr<TritonNode>				m_rpTritonNode;

		///创建海洋所需的资源路径
        std::string                             m_strTritonPath;
};
}

#endif//HG_MEASURE_SYSTEM_CALL_H
