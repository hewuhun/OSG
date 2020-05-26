/**************************************************************************************************
* @file PathRegistry.h
* @note 定义了文件注册类
* @author l00008
* @data 2014-11-03
**************************************************************************************************/

#ifndef KERNEL_FILE_REGISTRY_H
#define KERNEL_FILE_REGISTRY_H

#include <string>
#include <osg/Node>
#include <FeUtils/Export.h>

namespace FeUtil
{
    /**
      * @class CFileReg
      * @brief 文件注册类
      * @note 主要解读文件的系统路径、翻译文件等一系列关于文件的操作
      * @author l00008
    */
    class FEUTIL_EXPORT CFileReg
    {
    public:
		friend class CFileRegInitAgent;

        /**  
          * @brief 获得其实例
          * @note 获得其实例  
          * @return 实例对象
        */
        static CFileReg* Instance();

        /**  
          * @brief 获得全路径
          * @note 根据文件的部分路径获得全路径  
          * @param strFileName [in]
          * @return 文件的全路径
        */
        std::string GetFullPath(const std::string& strFileName);

        /**  
          * @brief 可翻译的文件
          * @note 需要翻译的文件,程序中所有的信息都使用中文完成  
          * @param strKeyWords [in] 原始的字符
          * @return 返回翻译后的字符
        */
        std::string GetTransWord(const std::string& strKeyWords);


		 /**  
          * @brief 获得系统的数据环境变量路径
          * @note 获得系统的数据环境变量路径
          * @return 环境变量路径
        */
		std::string GetDataPath() const;

    protected:
        CFileReg();
        virtual ~CFileReg();

		/**  
          * @brief 设置系统的数据环境变量路径
        */
		void SetDataPath(const std::string& strValue)
		{
			m_strFilePath = strValue;
		}

    protected:
		/**
		  * @class CDestoryInstance
		  * @brief 销毁CFileReg单例对象的类
		  * @author g00034
		*/
        class CDestoryInstance
        {
        public:
            CDestoryInstance(){}
            ~CDestoryInstance()
            {
                if(CFileReg::Instance())
                {
                    delete CFileReg::Instance();
                }
            }
        };
        static CDestoryInstance destroy;

    protected:
		/// CFileReg实例对象指针
        static CFileReg*    m_pInstance;

		/// 数据环境变量路径
        std::string         m_strFilePath;

    };
	#define FeFileReg FeUtil::CFileReg::Instance()


	class FEUTIL_EXPORT CFileRegInitAgent
	{
	public:
		CFileRegInitAgent()
		{
			
		}
		virtual ~CFileRegInitAgent()
		{

		}

		void InitDataPath(const std::string& strDataPath)
		{
			FeFileReg->SetDataPath(strDataPath);
		}
	};
}

namespace FeUtil
{
	class FEUTIL_EXPORT CReadFileRegistry
    {
    public:
        static CReadFileRegistry* Instance();

		osg::Node* ReadNodeFileInstance(const std::string& strPath);

		osg::Image* ReadImageFileInstance(const std::string& strPath);

    protected:
        CReadFileRegistry();

        virtual ~CReadFileRegistry();

    protected:
        class CDestoryInstance
        {
        public:
            CDestoryInstance(){}
            ~CDestoryInstance()
            {
                if(CReadFileRegistry::Instance())
                {
                    delete CReadFileRegistry::Instance();
                }
            }
        };
        static CDestoryInstance destroy;

    protected:
        static CReadFileRegistry*    m_pInstance;

        typedef std::map<std::string, osg::observer_ptr<osg::Node> >	LoadNodeInstance;
		typedef std::map<std::string, osg::observer_ptr<osg::Image>>	LoadImageInstance;
		LoadNodeInstance											m_loadNodeInstance;
		LoadImageInstance											m_loadImageInstance;

    };
}

#endif //KERNEL_FILE_REGISTRY_H
