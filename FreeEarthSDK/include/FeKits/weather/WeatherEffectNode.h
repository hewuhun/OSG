/**************************************************************************************************
* @file WeatherEffectNode.h
* @note 天气效果节点
* @author c00005
* @data 2014-2-28
**************************************************************************************************/
#ifndef WEATHEREFFECTNODE_H
#define WEATHEREFFECTNODE_H

#include <osg/Fog>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/BoundingBox>
#include <osgUtil/CullVisitor>
#include <osgEarth/MapNode>

#include <FeKits/Export.h>

#include <FeKits/KitsCtrl.h>

namespace FeKit
{
	/**  
	* 天气类型的枚举变量，分别代表小雨、大雨、小雪、大雪
	*/
	enum EWeatherType
	{
		LIGHT_RIAN = 1,
		HEAVY_RAIN = 2,
		LIGHT_SNOW = 3,
		HEAVY_SNOW = 4,
		NO_WEATHER = 0
	};

	class CWeatherEffectDrawable;

	/**
	* @class CWeatherEffectNode
	* @brief 天气特效节点
	* @author c00005
	*/
	class FEKIT_EXPORT CWeatherEffectNode : public osg::Node, public CKitsCtrl
	{
	public:
		/**  
		* @brief 构造函数
		*/
		CWeatherEffectNode();

		/**  
		* @brief 构造函数
		*/
		CWeatherEffectNode(FeUtil::CRenderContext* pContext);

		/**  
		* @brief 构造函数
		*/
		CWeatherEffectNode(const CWeatherEffectNode& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		/**  
		* @brief 析构函数
		*/
		~CWeatherEffectNode(void);

	public:
		/**
		*@note: 显示特效
		*/
		virtual void Show();

		/**
		*@note: 隐藏特效
		*/
		virtual void Hide();

		/**
		*@note: 是否隐藏特效
		*/
		virtual bool IsHide() const;

	public:
		virtual void accept(osg::NodeVisitor& nv);

		/**  
		* @brief 遍历器
		* @note 遍历器用于在每个阶段对该节点进行处理，是从Node继承的。 
		*/
		virtual void traverse(osg::NodeVisitor& nv);

		/**  
		* @note 雨效初始化  
		* @param intensity [in] 用于初始化雨的浓度。
		*/
		void rain(float intensity);

		/**  
		* @note 雪效初始化  
		* @param intensity [in] 用于初始化雪的浓度。
		*/
		void snow(float intensity);

		/**  
		* @note 设置天气的类型  
		* @param EWeatherType [in] 天气的类型的enum变量。
		*/
		void SetWeatherType(EWeatherType type);

		/**  
		* @note 设置天气的位置的经纬度以及天气区域的半径 
		* @param pos [in] 位置信息，分别代表经度、纬度、半径 。
		* @param isLocal [in] 是否为区域雨雪，如果不设置则默认是。
		*/
		void SetWeatherPosition(osg::Vec3 pos=osg::Vec3(-90.0,0.0,6000), bool isLocal=true)
		{
			_vecWeatherPosition = pos; 
			_bIsWorld = !isLocal;
			_dirty = true; 
		}

		/**  
		* @note 获取天气的位置的经纬度以及天气区域的半径
		*/
		osg::Vec3 GetWeatherPosition(){ return _vecWeatherPosition; }

		/**  
		* @note 设置天气产生的高度
		*/
		void SetWeatherHeight(float height){ _fWeatherHeight = height; _dirty = true; }

		/**  
		* @note 获取天气产生的高度
		*/
		float GetWeatherHeight(){ return _fWeatherHeight; }

		/**  
		* @note 设置是否为全局雨雪
		*/
		void SetWorld(bool isWorld){ _bIsWorld = isWorld; }

		/**  
		* @note 获取是否为全局雨雪
		*/
		bool GetWorld(){ return _bIsWorld; }

		/**  
		* @note 设置雨雪的范围，分别为最小最大的经度、纬度。  
		* @param range [in] 四个值分别代表最小最大的经度、纬度。
		*/
		void SetWeatherRange(osg::Vec4 range){ _vec4WeatherRange = range; }

		/**  
		* @note 获取雨雪的范围，分别为最小最大的经度、纬度。  
		*/
		osg::Vec4 GetWeatherRange(){ return _vec4WeatherRange; }

		/**  
		* @note 通过设置的雨雪效果的位置计算WeatherRange。  
		*/
		void CalculateWeatherRange(osg::Vec3 position)
		{
			double dEquator = 2 * osg::PI * osg::WGS_84_RADIUS_EQUATOR;
			double dEquatorRange = 360.0 * position.z() / dEquator;

			double dPolar = 2 * osg::PI * osg::WGS_84_RADIUS_POLAR;
			double dPolarRange = 180.0 * position.z() / dPolar;

			_vec4WeatherRange.set(position.x()-dEquatorRange, position.x()+dEquatorRange,
				position.y()-dPolarRange, position.y()+dPolarRange);
		}

		/**  
		* @brief 设置粒子的速度，即雨雪的下落速度
		*/
		void setParticleSpeed(float particleSpeed) 
		{
			if (_particleSpeed==particleSpeed) return;
			_particleSpeed = particleSpeed;
			_dirty = true;
		}

		/**  
		* @brief 获取粒子的速度，即雨雪的下落速度
		*/
		float getParticleSpeed() const { return _particleSpeed; }

		/**  
		* @brief 设置粒子的大小，即雨雪颗粒的大小
		*/
		void setParticleSize(float particleSize)
		{
			if (_particleSize==particleSize) return; 
			_particleSize = particleSize; 
			_dirty = true;
		}

		/**  
		* @brief 获取粒子的大小，即雨雪颗粒的大小
		*/
		float getParticleSize() const { return _particleSize; }

		/**  
		* @brief 设置粒子的颜色，即雨雪颗粒的颜色
		*/
		void setParticleColor(const osg::Vec4& color) 
		{
			if (_particleColor==color) return;
			_particleColor = color; 
			_dirty = true;
		}

		/**  
		* @brief 获取粒子的颜色，即雨雪颗粒的颜色
		*/
		const osg::Vec4& getParticleColor() const { return _particleColor; }

		/**  
		* @brief 设置雾效
		*/
		void setFog(osg::Fog* fog) { _fog = fog; }

		/**  
		* @brief 获取雾效
		*/
		osg::Fog* getFog() { return _fog.get(); }

		/**  
		* @brief 获取雾效
		*/
		const osg::Fog* getFog() const { return _fog.get(); }

		/**  
		* @brief 设置风向
		*/
		void setWind(const osg::Vec3& wind) { _wind = wind; }

		/**  
		* @brief 获取风向
		*/
		const osg::Vec3& getWind() const { return _wind; }

		/**  
		* @brief 设置位置
		*/
		void setPosition(const osg::Vec3& position) 
		{
			_origin = position; 

			CalculateWeatherRange(position);
		}

		/**  
		* @brief 获取位置
		*/
		const osg::Vec3& getPosition() const { return _origin; }

		/**  
		* @brief 设置每个立方体的大小
		*/
		void setCellSize(const osg::Vec3& cellSize) 
		{
			if (_cellSize==cellSize) return;
			_cellSize = cellSize; 
			_dirty = true; 
		}

		/**  
		* @brief 获取每个立方体的大小
		*/
		const osg::Vec3& getCellSize() const { return _cellSize; }

		/**  
		* @brief 设置最大的粒子浓度
		*/
		void setMaximumParticleDensity(float density) 
		{
			if (_maximumParticleDensity==density) return;
			_maximumParticleDensity = density;
			_dirty = true;
		}

		/**  
		* @brief 获取最大的粒子浓度
		*/
		float getMaximumParticleDensity() const { return _maximumParticleDensity; }

		void setNearTransition(float nearTransition) { _nearTransition = nearTransition; }
		float getNearTransition() const { return _nearTransition; }

		void setFarTransition(float farTransition) { _farTransition = farTransition; }
		float getFarTransition() const { return _farTransition; }

		void setUseFarLineSegments(bool useFarLineSegments) 
		{ 
			_useFarLineSegments = useFarLineSegments; 
		}
		bool getUseFarLineSegments() const { return _useFarLineSegments; }

		osg::Geometry* getQuadGeometry() { return _quadGeometry.get(); }
		osg::StateSet* getQuadStateSet() { return _quadStateSet.get(); }

		osg::Geometry* getLineGeometry() { return _lineGeometry.get(); }
		osg::StateSet* getLineStateSet() { return _lineStateSet.get(); }

		osg::Geometry* getPointGeometry() { return _pointGeometry.get(); }
		osg::StateSet* getPointStateSet() { return _pointStateSet.get(); }

	protected:
		/**  
		* @brief 通过已经传入的参数更新相应的参数
		*/
		void update();

		void compileGLObjects(osg::RenderInfo& renderInfo) const;

		void createGeometry(unsigned int numParticles,
			osg::Geometry* quad_geometry,
			osg::Geometry* line_geometry,
			osg::Geometry* point_geometry);

		void setUpGeometries(unsigned int numParticles);

		struct SWeatherEffectDrawableSet
		{
			osg::ref_ptr<CWeatherEffectDrawable> _quadWeatherDrawable;
			osg::ref_ptr<CWeatherEffectDrawable> _lineWeatherDrawable;
			osg::ref_ptr<CWeatherEffectDrawable> _pointWeatherDrawable;
		};

		void BuildAll_Lgb(SWeatherEffectDrawableSet& pds);
		void BuildSub_Lgb(osg::Vec3d& result, SWeatherEffectDrawableSet& pds);

		void cull(SWeatherEffectDrawableSet& pds, osgUtil::CullVisitor* cv) const;

		bool build(const osg::Vec3 eyeLocal, int i, int j, int k, float startTime, 
			SWeatherEffectDrawableSet& pds, osg::Polytope& frustum,
			osgUtil::CullVisitor* cv) const;
	private:
		double baseHeight_Lgb;

	protected:
		///全局或者局部雨雪
		bool                        _bIsWorld;
		///雨雪的位置中心使用经纬度表示，第三个值为高度值
		osg::Vec3                   _vecWeatherPosition;
		///半径，范围
		float                       _fWeatherHeight;
		///最大最小经纬度--最小经度，最大经度，最小纬度，最大纬度
		osg::Vec4                   _vec4WeatherRange;

		bool                        _dirty;
		osg::Vec3                   _wind;
		float                       _particleSpeed;
		float                       _particleSize;
		osg::Vec4                   _particleColor;
		float                       _maximumParticleDensity;
		osg::Vec3                   _cellSize;
		float                       _nearTransition;
		float                       _farTransition;
		bool                        _useFarLineSegments;
		osg::ref_ptr<osg::Fog>      _fog;

		osg::ref_ptr<osg::Uniform>  _inversePeriodUniform;
		osg::ref_ptr<osg::Uniform>  _particleSizeUniform;
		osg::ref_ptr<osg::Uniform>  _particleColorUniform;

		typedef std::pair< osg::NodeVisitor*, osg::NodePath > ViewIdentifier;
		typedef std::map< ViewIdentifier, SWeatherEffectDrawableSet >  ViewDrawableMap;

		OpenThreads::Mutex _mutex;
		ViewDrawableMap _viewDrawableMap;

		osg::ref_ptr<osg::Geometry> _quadGeometry;
		osg::ref_ptr<osg::StateSet> _quadStateSet;

		osg::ref_ptr<osg::Geometry> _lineGeometry;
		osg::ref_ptr<osg::StateSet> _lineStateSet;

		osg::ref_ptr<osg::Geometry> _pointGeometry;
		osg::ref_ptr<osg::StateSet> _pointStateSet;

		// cache variables.
		float       _period;
		osg::Vec3   _origin;
		osg::Vec3   _du;
		osg::Vec3   _dv;
		osg::Vec3   _dw;
		osg::Vec3   _inverse_du;
		osg::Vec3   _inverse_dv;
		osg::Vec3   _inverse_dw;

		double      _previousFrameTime;
	};

	/**
	* @class CWeatherEffectDrawable
	* @brief 绘制类
	* @note 用于雨滴雪滴等下落物的绘制，继承自Drawable
	* @author c00005
	*/
	class CWeatherEffectDrawable : public osg::Drawable
	{
	public:
		CWeatherEffectDrawable();
		CWeatherEffectDrawable(const CWeatherEffectDrawable& copy, 
			const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		META_Object(osgParticle, CWeatherEffectDrawable);

	public:
		virtual bool supports(const osg::PrimitiveFunctor&) const { return false; }

		virtual void accept(osg::PrimitiveFunctor&) const {}

		virtual bool supports(const osg::PrimitiveIndexFunctor&) const { return false; }

		virtual void accept(osg::PrimitiveIndexFunctor&) const {}

		/**  
		* @brief 设置是否需要前置矩阵变换
		*/
		void setRequiresPreviousMatrix(bool flag) { _requiresPreviousMatrix = flag; }

		bool getRequiresPreviousMatrix() const { return _requiresPreviousMatrix; }

		void setGeometry(osg::Geometry* geom) { _geometry = geom; }
		osg::Geometry* getGeometry() { return _geometry.get(); }
		const osg::Geometry* getGeometry() const { return _geometry.get(); }

		void setDrawType(GLenum type) { _drawType = type; }
		GLenum getDrawType() const { return _drawType; }

		void setNumberOfVertices(unsigned int numVertices) { _numberOfVertices = numVertices; }
		unsigned int getNumberOfVertices() const { return _numberOfVertices; }

		virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

		struct Cell
		{
			Cell(int in_i, int in_j, int in_k):
		i(in_i), j(in_j), k(in_k) {}

		inline bool operator < (const Cell& rhs) const
		{
			if (i<rhs.i) return true;
			if (i>rhs.i) return false;
			if (j<rhs.j) return true;
			if (j>rhs.j) return false;
			if (k<rhs.k) return true;
			if (k>rhs.k) return false;
			return false;
		}

		int i;
		int j;
		int k;
		};

		struct DepthMatrixStartTime
		{
			inline bool operator < (const DepthMatrixStartTime& rhs) const
			{
				return depth < rhs.depth;
			}

			float           depth;
			float           startTime;
			osg::Matrix     modelview;
		};

		typedef std::map< Cell, DepthMatrixStartTime >  CellMatrixMap;

		struct LessFunctor
		{
			inline bool operator () (const CellMatrixMap::value_type* lhs,
				const CellMatrixMap::value_type* rhs) const
			{
				return (*lhs).second<(*rhs).second;
			}
		};

		CellMatrixMap& getCurrentCellMatrixMap() { return _currentCellMatrixMap; }
		CellMatrixMap& getPreviousCellMatrixMap() { return _previousCellMatrixMap; }

		inline void newFrame()
		{
			_previousCellMatrixMap.swap(_currentCellMatrixMap);
			_currentCellMatrixMap.clear();
		}

	protected:
		virtual ~CWeatherEffectDrawable() {}

		bool _requiresPreviousMatrix;

		osg::ref_ptr<osg::Geometry> _geometry;

		mutable CellMatrixMap _currentCellMatrixMap;
		mutable CellMatrixMap _previousCellMatrixMap;

		GLenum          _drawType;
		unsigned int    _numberOfVertices;
	};
}
#endif 
