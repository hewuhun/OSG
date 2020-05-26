/**************************************************************************************************
* @file InfoMaker.h
* @note 鼠标信息以及视点信息制造器，用于获取鼠标以及视点、相机的位置信息
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef FE_SCREEN_INFO_FUNCTION_MAKER_H
#define FE_SCREEN_INFO_FUNCTION_MAKER_H

#include <FeUtils/RenderContext.h>

namespace FeKit
{
	/**
	* @class CInfoMaker
	* @brief 鼠标信息以及视点信息制造器
	* @note 鼠标信息以及视点信息制造器，用于获取鼠标以及视点、相机的位置信息
	* @author c00005
	*/
	class CInfoMaker : public osg::Referenced
	{
	public:
		/**  
		* @brief 构造函数
		* @param pContext [in] 三维渲染上下文
		*/
		CInfoMaker(FeUtil::CRenderContext* pContext);

		virtual ~CInfoMaker();

	public:
		/**  
		* @brief 获取鼠标位置的地理经纬高的位置信息
		*/
		virtual void GetLLH(double dMouseX, double dMouseY, double& dLon, double& dLat, double& dHei) = 0;

		/**  
		* @brief 获取鼠标位置的地理经纬高的位置信息
		*/
		virtual void GetLLH(double dMouseX, double dMouseY, std::string& strLon, std::string& strLat, std::string& strHei) = 0;

		/**  
		* @brief 获取当前的相机的姿态信息
		*/
		virtual void GetPosture(double& dPitch, double& dHeading, double& dRange);

	protected:
		///三维渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;
	};

	/**
	* @class CCommonInfo
	* @brief 普通鼠标信息以及视点信息制造器
	* @note 鼠标信息以及视点信息制造器，用于获取鼠标以及视点、相机的位置信息
	* @author c00005
	*/
	class CCommonInfo : public CInfoMaker
	{
	public:
		CCommonInfo(FeUtil::CRenderContext* pContext);

		virtual ~CCommonInfo();

	public:
		/**  
		* @brief 获取鼠标位置的地理经纬高的位置信息
		*/
		virtual void GetLLH(double dMouseX, double dMouseY, double& dLon, double& dLat, double& dHei);

		/**  
		* @brief 获取鼠标位置的地理经纬高的位置信息
		*/
		virtual void GetLLH(double dMouseX, double dMouseY, std::string& strLon, std::string& strLat, std::string& strHei);
	};

	/**
	* @class CArmyInfo
	* @brief 军用鼠标信息以及视点信息制造器
	* @note 鼠标信息以及视点信息制造器，用于获取鼠标以及视点、相机的位置信息
	* @author c00005
	*/
	class CArmyInfo : public CInfoMaker
	{
	public:
		CArmyInfo(FeUtil::CRenderContext* pContext);

		virtual ~CArmyInfo();

	public:
		/**  
		* @brief 获取鼠标位置的地理经纬高的位置信息
		*/
		virtual void GetLLH(double dMouseX, double dMouseY, double& dLon, double& dLat, double& dHei);

		/**  
		* @brief 获取鼠标位置的地理经纬高的位置信息
		*/
		virtual void GetLLH(double dMouseX, double dMouseY, std::string& strLon, std::string& strLat, std::string& strHei);
	};
}


#endif //FE_SCREEN_INFO_FUNCTION_MAKER_H
