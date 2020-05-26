
#ifndef FE_EX_NODE_MINI_ANIMATION_H
#define FE_EX_NODE_MINI_ANIMATION_H 1

#include <map>
#include <istream>
#include <float.h>

#include <osg/Matrixf>
#include <osg/Matrixd>
#include <osg/Quat>
#include <osg/NodeCallback>

#include <FeUtils/Export.h>

namespace FeUtil 
{

	class FEUTIL_EXPORT AnimationPath : public virtual osg::Object
	{
	public:

		AnimationPath():_loopMode(LOOP) {}

		AnimationPath(const AnimationPath& ap, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
		Object(ap,copyop),
			_timeControlPointMap(ap._timeControlPointMap),
			_loopMode(ap._loopMode) {}

		META_Object(FeUtil,AnimationPath);

		class ControlPoint
		{
		public:
			ControlPoint():
			  _scale(1.0,1.0,1.0) {}

			  ControlPoint(const osg::Vec3d& position):
			  _position(position),
				  _rotation(),
				  _scale(1.0,1.0,1.0) {}

			  ControlPoint(const osg::Vec3d& position, const osg::Quat& rotation):
			  _position(position),
				  _rotation(rotation),
				  _scale(1.0,1.0,1.0) {}

			  ControlPoint(const osg::Vec3d& position, const osg::Quat& rotation, const osg::Vec3d& scale):
			  _position(position),
				  _rotation(rotation),
				  _scale(scale) {}

			  void setPosition(const osg::Vec3d& position) { _position = position; }
			  const osg::Vec3d& getPosition() const { return _position; }

			  void setRotation(const osg::Quat& rotation) { _rotation = rotation; }
			  const osg::Quat& getRotation() const { return _rotation; }

			  void setScale(const osg::Vec3d& scale) { _scale = scale; }
			  const osg::Vec3d& getScale() const { return _scale; }

			  inline void interpolate(float ratio,const ControlPoint& first, const ControlPoint& second)
			  {
				  float one_minus_ratio = 1.0f-ratio;
				  _position = first._position*one_minus_ratio + second._position*ratio;
				  _rotation.slerp(ratio,first._rotation,second._rotation);
				  _scale = first._scale*one_minus_ratio + second._scale*ratio;
			  }

			  inline void interpolate(double ratio,const ControlPoint& first, const ControlPoint& second)
			  {
				  double one_minus_ratio = 1.0-ratio;
				  _position = first._position*one_minus_ratio + second._position*ratio;
				  _rotation.slerp(ratio,first._rotation,second._rotation);
				  _scale = first._scale*one_minus_ratio + second._scale*ratio;
			  }

			  inline void getMatrix(osg::Matrixf& matrix) const
			  {
				  matrix.makeRotate(_rotation);
				  matrix.preMultScale(_scale);
				  matrix.postMultTranslate(_position);
			  }

			  inline void getMatrix(osg::Matrixd& matrix) const
			  {
				  matrix.makeRotate(_rotation);
				  matrix.preMultScale(_scale);
				  matrix.postMultTranslate(_position);
			  }

			  inline void getInverse(osg::Matrixf& matrix) const
			  {
				  matrix.makeRotate(_rotation.inverse());
				  matrix.postMultScale(osg::Vec3d(1.0/_scale.x(),1.0/_scale.y(),1.0/_scale.z()));
				  matrix.preMultTranslate(-_position);
			  }

			  inline void getInverse(osg::Matrixd& matrix) const
			  {
				  matrix.makeRotate(_rotation.inverse());
				  matrix.postMultScale(osg::Vec3d(1.0/_scale.x(),1.0/_scale.y(),1.0/_scale.z()));
				  matrix.preMultTranslate(-_position);
			  }

		protected:

			osg::Vec3d _position;
			osg::Quat _rotation;
			osg::Vec3d _scale;

		};


		/** Given a specific time, return the transformation matrix for a point. */
		bool getMatrix(double time,osg::Matrixf& matrix) const
		{
			ControlPoint cp;
			if (!getInterpolatedControlPoint(time,cp)) return false;
			cp.getMatrix(matrix);
			return true;
		}

		/** Given a specific time, return the transformation matrix for a point. */
		bool getMatrix(double time,osg::Matrixd& matrix) const
		{
			ControlPoint cp;
			if (!getInterpolatedControlPoint(time,cp)) return false;
			cp.getMatrix(matrix);
			return true;
		}

		/** Given a specific time, return the inverse transformation matrix for a point. */
		bool getInverse(double time,osg::Matrixf& matrix) const
		{
			ControlPoint cp;
			if (!getInterpolatedControlPoint(time,cp)) return false;
			cp.getInverse(matrix);
			return true;
		}

		bool getInverse(double time,osg::Matrixd& matrix) const
		{
			ControlPoint cp;
			if (!getInterpolatedControlPoint(time,cp)) return false;
			cp.getInverse(matrix);
			return true;
		}

		/** Given a specific time, return the local ControlPoint frame for a point. */
		virtual bool getInterpolatedControlPoint(double time,ControlPoint& controlPoint) const;

		/** Insert a control point into the AnimationPath.*/
		void insert(double time,const ControlPoint& controlPoint);

		double getFirstTime() const { if (!_timeControlPointMap.empty()) return _timeControlPointMap.begin()->first; else return 0.0;}
		double getLastTime() const { if (!_timeControlPointMap.empty()) return _timeControlPointMap.rbegin()->first; else return 0.0;}
		double getPeriod() const { return getLastTime()-getFirstTime();}

		enum LoopMode
		{
			SWING,
			LOOP,
			NO_LOOPING
		};

		void setLoopMode(LoopMode lm) { _loopMode = lm; }

		LoopMode getLoopMode() const { return _loopMode; }


		typedef std::map<double,ControlPoint> TimeControlPointMap;

		void setTimeControlPointMap(TimeControlPointMap& tcpm) { _timeControlPointMap=tcpm; }

		TimeControlPointMap& getTimeControlPointMap() { return _timeControlPointMap; }

		const TimeControlPointMap& getTimeControlPointMap() const { return _timeControlPointMap; }

		bool empty() const { return _timeControlPointMap.empty(); }

		void clear() { _timeControlPointMap.clear(); }

		/** Read the animation path from a flat ASCII file stream. */
		void read(std::istream& in);

		/** Write the animation path to a flat ASCII file stream. */
		void write(std::ostream& out) const;

		/** Write the control point to a flat ASCII file stream. */
		void write(TimeControlPointMap::const_iterator itr, std::ostream& out) const;

	protected:

		virtual ~AnimationPath() {}

		TimeControlPointMap _timeControlPointMap;
		LoopMode            _loopMode;

	};


	class FEUTIL_EXPORT AnimationPathCallback : public osg::NodeCallback
	{
	public:

		AnimationPathCallback():
		  _pivotPoint(0.0,0.0,0.0),
			  _useInverseMatrix(false),
			  _timeOffset(0.0),
			  _timeMultiplier(1.0),
			  _firstTime(DBL_MAX),
			  _latestTime(0.0),
			  _pause(false),
			  _pauseTime(0.0),
			  _blCalculate(false),
			  _blFinish(false),
			  m_dHeiOffset(2.0){}

		  AnimationPathCallback(const AnimationPathCallback& apc,const osg::CopyOp& copyop):
		  osg::NodeCallback(apc,copyop),
			  _animationPath(apc._animationPath),
			  _pivotPoint(apc._pivotPoint),
			  _useInverseMatrix(apc._useInverseMatrix),
			  _timeOffset(apc._timeOffset),
			  _timeMultiplier(apc._timeMultiplier),
			  _firstTime(apc._firstTime),
			  _latestTime(apc._latestTime),
			  _pause(apc._pause),
			  _pauseTime(apc._pauseTime),
			  _blCalculate(false),
			  _blFinish(false),
			  m_dHeiOffset(2.0){}


		  META_Object(osg,AnimationPathCallback);

		  /** Construct an AnimationPathCallback with a specified animation path.*/
		  AnimationPathCallback(AnimationPath* ap,double timeOffset=0.0,double timeMultiplier=1.0):
		  _animationPath(ap),
			  _pivotPoint(0.0,0.0,0.0),
			  _useInverseMatrix(false),
			  _timeOffset(timeOffset),
			  _timeMultiplier(timeMultiplier),
			  _firstTime(DBL_MAX),
			  _latestTime(0.0),
			  _pause(false),
			  _pauseTime(0.0),
			  _blCalculate(false),
			  _blFinish(false),
			  m_dHeiOffset(2.0){}

		  /** Construct an AnimationPathCallback and automatically create an animation path to produce a rotation about a point.*/
		  AnimationPathCallback(const osg::Vec3d& pivot,const osg::Vec3d& axis,float angularVelocity);


		  void setAnimationPath(AnimationPath* path) { _animationPath = path; }
		  AnimationPath* getAnimationPath() { return _animationPath.get(); }
		  const AnimationPath* getAnimationPath() const { return _animationPath.get(); }

		  inline void setPivotPoint(const osg::Vec3d& pivot) { _pivotPoint = pivot; }
		  inline const osg::Vec3d& getPivotPoint() const { return _pivotPoint; }

		  void setUseInverseMatrix(bool useInverseMatrix) { _useInverseMatrix = useInverseMatrix; }
		  bool getUseInverseMatrix() const { return _useInverseMatrix; }

		  void setTimeOffset(double offset) { _timeOffset = offset; }
		  double getTimeOffset() const { return _timeOffset; }

		  void setTimeMultiplier(double multiplier) { _timeMultiplier = multiplier; }
		  double getTimeMultiplier() const { return _timeMultiplier; }


		  virtual void reset();

		  void setPause(bool pause);
		  bool getPause() const { return _pause; }

		  /** Get the animation time that is used to specify the position along
		  * the AnimationPath. Animation time is computed from the formula:
		  *   ((_latestTime-_firstTime)-_timeOffset)*_timeMultiplier.*/
		  virtual double getAnimationTime() const;

		  /** Implements the callback. */
		  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

		  virtual void update(osg::Node& node);

		  //判断一个周期结束
		  bool IsFinish();

		  bool SetAnimationLoopMode(AnimationPath::LoopMode mode);

		  void SetFrameCalculate(bool bCalculate);

		  void SetHeightOffset(double dHeiOffset);

		  osg::Vec3d GetCurrPosition();

		  osg::Vec3d GetDet();

		  osg::Vec3d GetCurrWorld();

		  osg::Matrixd GetCurrMatrix();

	public:

		osg::ref_ptr<AnimationPath>  _animationPath;
		osg::Vec3d					 _pivotPoint;
		bool						 _useInverseMatrix;
		double						 _timeOffset;
		double						 _timeMultiplier;
		double						 _firstTime;
		double						 _latestTime;
		bool						 _pause;
		double						 _pauseTime;

		double                       m_dHeiOffset;

	protected:

		bool				_blFinish;
		bool				_blCalculate;

		osg::Vec3d          m_vecWorldPos;
		osg::Vec3d          m_vecLLAPos;


		osg::Vec3d			m_vecDet;

		osg::Matrixd         m_matrix;

		~AnimationPathCallback(){}

	};

}

#endif //FE_EX_NODE_MINI_ANIMATION_H
