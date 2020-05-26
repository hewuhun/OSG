/**************************************************************************************************
* @file SilverliningSys.h
* @note 定义了云层管理的模块
* @author z00013
* @data 
**************************************************************************************************/
#ifndef FE_SILVERLINING_SYSTEM_H
#define FE_SILVERLINING_SYSTEM_H

#include <FeSilverlining/Export.h>

#include <FeSilverlining/SilverLiningNode.h>

#include <FeUtils/SceneSysCall.h>


namespace FeSilverLining
{
	using namespace FeUtil;

	const std::string SILVERLINING_CALL_DEFAULT_KEY("SilverLiningSystem");

    /**
      * @class CSilverLiningSys
      * @brief 云层管理的系统模块
      * @note 定义了云层功能的系统模块
      * @author z00013
    */
    class FESILVERLINING_EXPORT CSilverLiningSys : public CSceneSysCall
    {
    public:
        CSilverLiningSys(const std::string& strSilverLiningPath = "");
        
		~CSilverLiningSys();

    public:
        FeSilverLiningNode * GetSilverLiningNode();

	protected:
		/**
        *@note: 实现本系统模型需要的初始化函数，此函数在父类中已经调用
        */
		virtual bool InitiliazeImplement();

		/**
		*@note: 实现本系统模型需要的反初始化函数，此函数在父类中已经调用
		*/
        virtual bool UnInitiliazeImplement();

	public:
		/**
		*@note: 设置光照 H00021 2016-10-14 解决光照冲突问题
		*/
        void SetLight(osg::Light* pLight);

		/**
		*@note: 设置相机 H00021 2016-10-20 解决相机冲突问题
		*/
        void SetCamera(osg::Camera* pCamera);
		
    private:
        osg::ref_ptr<FeSilverLiningNode>					m_rpSilverLiningNode;

        std::string											m_strSilverLiningPath;
		osg::ref_ptr<osg::Light>                            m_rpLight;
		osg::ref_ptr<osg::Camera>                           m_rpCamera;
};
}

#endif//FE_SILVERLINING_SYSTEM_H
