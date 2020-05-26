/**************************************************************************************************
* @file GeneratorID.h
* @note 生成唯一id的类
* @author x00028
* @data 2016-4-18
**************************************************************************************************/
#ifndef GENERATOR_ID_H
#define GENERATOR_ID_H

#include <iostream>

#include <FeUtils/Export.h>

#include <osgEarth/ThreadingUtils>

namespace FeUtil
{
	typedef unsigned int FEID;

	 /**
      * @class CUIDGenerator
      * @brief 唯一id的生成类
      * @note 主要用于在系统中生成唯一的数字ID
      * @author x00028
    */
	class FEUTIL_EXPORT CUIDGenerator
	{
	public:
		/**  
          * @brief 获取实例对象，使用单例模式
          * @return 实例对象
        */
        static CUIDGenerator* Instance();

	public:
		/**  
          * @brief 创建全局唯一ID
          * @return ID
        */
		FEID CreateUID();

		/**  
          * @brief 设置初始ID
          * @return 是否设置成功
        */
		bool SetOriginalID(FEID id);

	protected:
		CUIDGenerator();
		virtual ~CUIDGenerator();

	protected:
		/**  
		  * @class CDestoryInstance
          * @brief 销毁CUIDGenerator对象的类
        */
		class CDestoryInstance
		{
		public:
			CDestoryInstance(){}
			~CDestoryInstance()
			{
				if(CUIDGenerator::Instance())
				{
					delete CUIDGenerator::Instance();
				}
			}
		};
		static CDestoryInstance destroy;

	protected:
		/// 实例对象指针
		static CUIDGenerator*				m_pInstance;

		/// ID
		unsigned int						m_unId;

		/// 互斥对象
		mutable osgEarth::Threading::Mutex  m_IDGenMutex;
	};

	#define FeIDGenerator FeUtil::CUIDGenerator::Instance()
}
#endif //GENERATOR_ID_H

