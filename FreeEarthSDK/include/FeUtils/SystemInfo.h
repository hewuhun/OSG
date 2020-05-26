/**************************************************************************************************
* @file SystemInfo.h
* @note 系统信息获取
* @author g00034
* @data 2017-04-08
**************************************************************************************************/

#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <string>
#include <list>

#include <FeUtils/Export.h>

namespace FeUtil
{
	/**
	  * @class CSystemInfo
	  * @brief 系统信息类
	  * @note 用于获取系统信息
	  * @author g00034
	*/
	class FEUTIL_EXPORT CSystemInfo
	{
	public:
		/// 显卡型号枚举
		enum EDisplayCardType
		{
			E_NVIDIA,
			E_ATI,
			E_INTEGRATED,
			E_Default
		};

    public:
        /**  
          * @brief 获得其实例
          * @note 获得其实例  
          * @return 实例对象
        */
        static CSystemInfo* Instance();

		/**  
		  * @note 获取当前显卡类型  
		  * @return 返回显卡类型枚举
		*/
		EDisplayCardType GetDisplayCardType();

	protected:
		void GetDisplayCardInfo(int &dwNum,std::list<std::string>& names);

	protected:
		 /**  
          * @brief 构造函数
        */
		CSystemInfo() ;
		~CSystemInfo();

	protected:
		/// 显卡类型枚举
		EDisplayCardType		m_nDisplayCardType;

		/// 静态实例
		static CSystemInfo*     m_pSystemInfo;
	};
};

#endif //SYSTEM_INFO_H