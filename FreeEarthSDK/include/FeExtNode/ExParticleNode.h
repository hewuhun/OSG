/**************************************************************************************************
* @file ExParticleNode.h
* @note 粒子编辑器
* @author w00040
* @data 2017-3-16
**************************************************************************************************/
#ifndef FE_PARTICLE_NODE_H
#define FE_PARTICLE_NODE_H

#include <osg/Group>
#include <osg/NodeVisitor>
#include <osgGA/GUIEventHandler>
#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/DomainOperator>
#include <osgParticle/ParticleEffect>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <vector>
#include <map>
#include <string>

#include <osgEarth/Config>

#include <FeUtils/RenderContext.h>

#include <FeExtNode/Export.h>
#include <FeExtNode/ExLodNode.h>
#include <FeExtNode/ExParticleInfoStruct.h>

namespace FeExtNode
{
	/**
	* @class CExParticleNodeOption
	* @brief 粒子配置信息
	* @note 粒子的配置属性信息
	* @author w00040
	*/
	class FEEXTNODE_EXPORT CExParticleNodeOption : public CExLodNodeOption
	{
	public:
		/**  
		* @brief 构造函数
		*/
		CExParticleNodeOption(void)
			:CExLodNodeOption()
		{
			m_vecCenter = osg::Vec3d(0, 0, 0);
			m_vecAngle = osg::Vec3d(0, 0, 0);
			m_strPath = "";
			m_dScale = 1.0;
		};

		/**  
		* @brief 析构函数
		*/
		~CExParticleNodeOption(void){};

	public:
		/**  
		* @note 获取粒子位置
		*/
		osg::Vec3d& GetPosition() { return m_vecCenter; }

		/**  
		* @note 获取粒子角度
		*/
		osg::Vec3d& GetAngle() { return m_vecAngle; }

		/**  
		  * @brief 获取粒子路径
		*/
		std::string& GetPath() {return m_strPath;}
		
		/**  
		  * @brief 获取粒子缩放比例
		*/
		double& GetScale() {return m_dScale;}

	protected:
		///粒子位置
		osg::Vec3d					m_vecCenter;

		///粒子角度
		osg::Vec3d					m_vecAngle;

		///粒子路径
		std::string					m_strPath;

		///缩放比例
		double						m_dScale;
	};
}

namespace FeExtNode
{
	/**
	* @class     ParticleNodeEffect
	* @brief     粒子属性管理类
	* @note      实现粒子属性设置
	* @author    w00040
	*/
	class FEEXTNODE_EXPORT ParticleNodeEffect : public osgParticle::ParticleEffect
	{
	public:
		ParticleNodeEffect(ST_PARTICLE_INFO sParticleInfo, bool automaticSetup = true);

	public:
		/**  
		* @note 设置粒子信息
		*/
		void SetParticleInfo( ST_PARTICLE_INFO sParticleInfo);

		/**  
		* @note 添加域操作
		*/
		void AddDomianOpertor( osgParticle::DomainOperator* op,ST_EFFECT_OPITION_DOMIAN sDomian,unsigned int uiCount );

		/**  
		* @note 设置粒子默认值
		*/
		virtual void setDefaults();

		/**  
		* @note 设置发射极 编程器
		*/
		virtual void setUpEmitterAndProgram();

		/**  
		* @note 获取发射极
		*/
		virtual osgParticle::Emitter* getEmitter() { return _emitter.get(); }
		virtual const osgParticle::Emitter* getEmitter() const { return _emitter.get(); }

		/**  
		* @note 获取编程器
		*/
		virtual osgParticle::Program* getProgram() { return _program.get(); }
		virtual const osgParticle::Program* getProgram() const { return _program.get(); }

	protected:

		virtual ~ParticleNodeEffect() {}

		///发射器
		osg::ref_ptr<osgParticle::ModularEmitter> _emitter;

		///编程器
		osg::ref_ptr<osgParticle::ModularProgram> _program;

		///粒子属性值
		ST_PARTICLE_INFO m_sParticleInfo;
	};
}

namespace FeExtNode
{
	/**
	* @class CExParticleNode
	* @brief 粒子属性管理类
	* @note 实现场景中的粒子
	* @author w00040
	*/
	class FEEXTNODE_EXPORT CExParticleNode : public CExLodNode
	{
	public:
		/**  
		* @brief 构造函数
		*/
		CExParticleNode(CExParticleNodeOption *opt, FeUtil::CRenderContext* pRender);

		/**  
		* @brief 析构函数
		*/
		~CExParticleNode(void);

	public:
		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		  * @note 初始化粒子模板
		*/
		bool InitializeParicleTemplate();

	public:
		/**  
		  * @note 设置粒子位置
		*/
		void SetParticlePosition(const osg::Vec3d& vecLLHDegree);

		/**  
		  * @note 获取粒子位置
		*/
		const osg::Vec3d GetParticlePosition();

		/**  
		* @brief 设置粒子数据路径
		*/
		void SetParticlePath(const std::string& strXmlPath);

		/**  
		  * @brief 获取粒子数据路径
		*/
		std::string  GetParticlePath();

		/**  
		  * @brief 设置角度
		*/
		void SetParticleAngle(osg::Vec3 vecAngle);

		/**  
		  * @brief 获取角度
		*/
		const osg::Vec3 GetParticleAngle();

		/**  
		  * @brief 设置缩放
		*/
		void SetParticleScale(double dScale);

		/**  
		  * @brief 获取缩放系数
		*/
		const double GetParticleScale();

		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExParticleNodeOption* GetOption();

	protected:
		/**  
		* @note 添加粒子
		*/
		unsigned int AddParticleNode( std::string strTexture);

		/**  
		 * @note 设置粒子为空
		*/
		void SetNullTexture();

		/**  
		 * @note 设置当前编辑粒子
		*/
		void SetCurParticle( unsigned int unIDTexture);

		/**  
	     * @note 设置当前粒子ID
	    */
		void SetCurParticleID( unsigned int unID);

		/**  
	     * @note 获取当前粒子ID
	    */
		unsigned int GetCurParticleID();

		/**  
		* @note 清空场景
		*/
		void ClearScene();

		/**  
	     * @note  获取域配置
	    */
		bool ReadDomianOptConfig( osgEarth::Config config,ST_EFFECT_OPITION_DOMIAN& stDomainOpt);

		/**  
		* @note  获取粒子模板信息
		*/
		void ReadParticleTemplate( std::string strPath,ST_PARTICLE_TEMPLATE_INFO& stParticleTemplate );

		/**  
		* @note  获取单个粒子信息
		*/
		ST_PARTICLE_INFO ReadParticleInfo( osgEarth::Config config,std::string strPath);

		/**  
		* @note  获取粒子配置
		*/
		bool ReadParticleOptConfig( osgEarth::Config config,ST_PARTICLE_OPITION& sParticleOpt);

		/**  
		* @note  获取颜色
		*/
		bool ReadColorOptConfig( osgEarth::Config config,ST_COLOR_OPITION& sColorOpt);

		/**  
		* @note  获取发射器
		*/
		bool ReadShootOptConfig( osgEarth::Config config,ST_SHOOT_OPITION& sShootOpt);

		/**  
		* @note  获取操作器配置
		*/
		bool ReadEffectOptConfig( osgEarth::Config config,ST_EFFECT_OPITION& sEffectOpt);

		virtual void traverse(osg::NodeVisitor& nv);

	protected:
		///粒子根节点
		osg::ref_ptr<osg::Group>			m_rpRoot;

		///粒子移动根节点
		osg::ref_ptr<osg::MatrixTransform>  m_rpTranslateGroup;

		///粒子旋转根节点
		osg::ref_ptr<osg::MatrixTransform>  m_rpRotateGroup;

		///粒子更新器
		osg::ref_ptr<osgParticle::ParticleSystemUpdater> m_rpSysUpdater;

		///粒子节点 粒子信息 关联
		std::map<osg::Group*,ST_PARTICLE_INFO> m_mapParticleNodetoInfo;

		///当前粒子信息
		ST_PARTICLE_INFO m_stCurParticleInfo;

		///当前粒子模板信息
		ST_PARTICLE_TEMPLATE_INFO m_stCurParticleTemplate;

		///当前粒子ID
		unsigned int m_unCurParticleID;

		///粒子路径
		std::string m_strParticlePath;

		FeUtil::CRenderContext* m_pRender;
	};
}

#endif