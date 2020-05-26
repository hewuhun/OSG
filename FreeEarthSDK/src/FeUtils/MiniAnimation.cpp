﻿
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Camera>
#include <osg/CameraView>
#include <osg/io_utils>

#include <FeUtils/MiniAnimation.h>

namespace FeUtil
{

	void AnimationPath::insert(double time,const ControlPoint& controlPoint)
	{
		_timeControlPointMap[time] = controlPoint;
	}

	bool AnimationPath::getInterpolatedControlPoint(double time,ControlPoint& controlPoint) const
	{
		if (_timeControlPointMap.empty()) return false;

		switch(_loopMode)
		{
		case(SWING):
			{
				double modulated_time = (time - getFirstTime())/(getPeriod()*2.0);
				double fraction_part = modulated_time - floor(modulated_time);
				if (fraction_part>0.5) fraction_part = 1.0-fraction_part;

				time = getFirstTime()+(fraction_part*2.0) * getPeriod();
				break;
			}
		case(LOOP):
			{
				double modulated_time = (time - getFirstTime())/getPeriod();
				double fraction_part = modulated_time - floor(modulated_time);
				time = getFirstTime()+fraction_part * getPeriod();
				break;
			}
		case(NO_LOOPING):
			// no need to modulate the time.
			break;
		}



		TimeControlPointMap::const_iterator second = _timeControlPointMap.lower_bound(time);
		if (second==_timeControlPointMap.begin())
		{
			controlPoint = second->second;
		}
		else if (second!=_timeControlPointMap.end())
		{
			TimeControlPointMap::const_iterator first = second;
			--first;

			// we have both a lower bound and the next item.

			// delta_time = second.time - first.time
			double delta_time = second->first - first->first;

			if (delta_time==0.0)
				controlPoint = first->second;
			else
			{
				controlPoint.interpolate((time - first->first)/delta_time,
					first->second,
					second->second);
			}
		}
		else // (second==_timeControlPointMap.end())
		{
			controlPoint = _timeControlPointMap.rbegin()->second;
			return false;
		}
		return true;
	}


	void AnimationPath::read(std::istream& in)
	{
		while (!in.eof())
		{
			double time;
			osg::Vec3d position;
			osg::Quat rotation;
			in >> time >> position.x() >> position.y() >> position.z() >> rotation.x() >> rotation.y() >> rotation.z() >> rotation.w();
			if(!in.eof())
				insert(time,AnimationPath::ControlPoint(position,rotation));
		}
	}

	void AnimationPath::write(TimeControlPointMap::const_iterator itr, std::ostream& fout) const
	{
		const ControlPoint& cp = itr->second;
		fout<<itr->first<<" "<<cp.getPosition()<<" "<<cp.getRotation()<<std::endl;
	}

	void AnimationPath::write(std::ostream& fout) const
	{
		int prec = fout.precision();
		fout.precision(15);

		const TimeControlPointMap& tcpm = getTimeControlPointMap();
		for(TimeControlPointMap::const_iterator tcpmitr=tcpm.begin();
			tcpmitr!=tcpm.end();
			++tcpmitr)
		{
			write(tcpmitr, fout);
		}

		fout.precision(prec);
	}

	AnimationPathCallback::AnimationPathCallback(const osg::Vec3d& pivot,const osg::Vec3d& axis,float angularVelocity):
	_pivotPoint(pivot),
		_useInverseMatrix(false),
		_timeOffset(0.0),
		_timeMultiplier(1.0),
		_firstTime(DBL_MAX),
		_latestTime(0.0),
		_pause(false),
		_pauseTime(0.0),
		_blCalculate(false),
		_blFinish(false),
		m_dHeiOffset(2.0)
	{
		_animationPath = new AnimationPath;
		_animationPath->setLoopMode(AnimationPath::LOOP);

		double time0 = 0.0;
		double time1 = osg::PI*0.5/angularVelocity;
		double time2 = osg::PI*1.0/angularVelocity;
		double time3 = osg::PI*1.5/angularVelocity;
		double time4 = osg::PI*2.0/angularVelocity;

		osg::Quat rotation0(0.0, axis);
		osg::Quat rotation1(osg::PI*0.5, axis);
		osg::Quat rotation2(osg::PI*1.0, axis);
		osg::Quat rotation3(osg::PI*1.5, axis);


		_animationPath->insert(time0,AnimationPath::ControlPoint(pivot,rotation0));
		_animationPath->insert(time1,AnimationPath::ControlPoint(pivot,rotation1));
		_animationPath->insert(time2,AnimationPath::ControlPoint(pivot,rotation2));
		_animationPath->insert(time3,AnimationPath::ControlPoint(pivot,rotation3));
		_animationPath->insert(time4,AnimationPath::ControlPoint(pivot,rotation0));
	}

	class AnimationPathCallbackVisitor : public osg::NodeVisitor
	{
	public:

		AnimationPathCallbackVisitor(const AnimationPath::ControlPoint& cp, const osg::Vec3d& pivotPoint, bool useInverseMatrix):
		  _cp(cp),
			  _pivotPoint(pivotPoint),
			  _useInverseMatrix(useInverseMatrix) {}

		  virtual void apply(osg::Camera& camera)
		  {
			  osg::Matrix matrix;
			  if (_useInverseMatrix)
				  _cp.getInverse(matrix);
			  else
				  _cp.getMatrix(matrix);

			  camera.setViewMatrix(osg::Matrix::translate(-_pivotPoint)*matrix);
		  }


		  virtual void apply(osg::CameraView& cv)
		  {
			  if (_useInverseMatrix)
			  {
				  osg::Matrix matrix;
				  _cp.getInverse(matrix);
				  cv.setPosition(matrix.getTrans());
				  cv.setAttitude(_cp.getRotation().inverse());
				  cv.setFocalLength(1.0f/_cp.getScale().x());

			  }
			  else
			  {
				  cv.setPosition(_cp.getPosition());
				  cv.setAttitude(_cp.getRotation());
				  cv.setFocalLength(_cp.getScale().x());
			  }
		  }

		  virtual void apply(osg::Group& node)
		  {
			  node.traverse(*this);
		  }

		  virtual void apply(osg::MatrixTransform& mt)
		  {
			  osg::Matrix matrix;
			  //osg::Vec3d vecLLA;
			  //osg::Vec3d vecWorld = _cp.getPosition();
			  //
			  //COORD_CONV->XYZ2DegreeLLH(vecWorld,vecLLA);
			  //COORD_CONV->LL2LLH(vecLLA);
			  //vecLLA.z() += 2.0;

			  //COORD_CONV->DegreeLLH2XYZ(vecLLA,vecWorld);

			  //_cp.setPosition(vecWorld);

			  if (_useInverseMatrix)
				  _cp.getInverse(matrix);
			  else
				  _cp.getMatrix(matrix);

			  mt.setMatrix(osg::Matrix::translate(-_pivotPoint)*matrix);
		  }

		  virtual void apply(osg::PositionAttitudeTransform& pat)
		  {
			  if (_useInverseMatrix)
			  {
				  osg::Matrix matrix;
				  _cp.getInverse(matrix);
				  pat.setPosition(matrix.getTrans());
				  pat.setAttitude(_cp.getRotation().inverse());
				  pat.setScale(osg::Vec3(1.0f/_cp.getScale().x(),1.0f/_cp.getScale().y(),1.0f/_cp.getScale().z()));
				  pat.setPivotPoint(_pivotPoint);

			  }
			  else
			  {
				  pat.setPosition(_cp.getPosition());
				  pat.setAttitude(_cp.getRotation());
				  pat.setScale(_cp.getScale());
				  pat.setPivotPoint(_pivotPoint);
			  }
		  }

		  AnimationPath::ControlPoint _cp;
		  osg::Vec3d _pivotPoint;
		  bool _useInverseMatrix;
	};

	void AnimationPathCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if (_animationPath.valid() &&
			nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR &&
			nv->getFrameStamp())
		{
			double time = nv->getFrameStamp()->getSimulationTime();
			_latestTime = time;

			if (!_pause)
			{
				// Only update _firstTime the first time, when its value is still DBL_MAX
				if (_firstTime==DBL_MAX) _firstTime = time;
				update(*node);
			}
		}

		// must call any nested node callbacks and continue subgraph traversal.
		osg::NodeCallback::traverse(node,nv);
	}

	double AnimationPathCallback::getAnimationTime() const
	{
		return ((_latestTime-_firstTime)-_timeOffset)*_timeMultiplier;
	}

	void AnimationPathCallback::update(osg::Node& node)
	{
		AnimationPath::ControlPoint cp;
		if (_animationPath->getInterpolatedControlPoint(getAnimationTime(),cp))
		{
// 			if(_blCalculate)
// 			{
// 				osg::Matrix matrix;
// 				osg::Vec3d vecLLA;
// 				osg::Vec3d vecWorld = cp.getPosition();
// 
// 				COORD_CONV->XYZ2DegreeLLH(vecWorld,vecLLA);
// 				COORD_CONV->LL2LLH(vecLLA);
// 				vecLLA.z() += m_dHeiOffset;
// 
// 				COORD_CONV->DegreeLLH2XYZ(vecLLA,vecWorld);
// 
// 				cp.setPosition(vecWorld);
// 			}
			//LPF 2015.04.09
			osg::Vec3d oldWorld = m_vecWorldPos;
			m_vecWorldPos = cp.getPosition();

			cp.getMatrix(m_matrix);
			m_vecDet =m_vecWorldPos- oldWorld;

			AnimationPathCallbackVisitor apcv(cp,_pivotPoint,_useInverseMatrix);
			node.accept(apcv);

			_blFinish = false;
			
		}
		else
		{
			_blFinish = true;
		}
	}


	void AnimationPathCallback::reset()
	{
#if 1
		_firstTime = DBL_MAX;
		_pauseTime = DBL_MAX;
#else
		_firstTime = _latestTime;
		_pauseTime = _latestTime;
#endif
	}

	void AnimationPathCallback::setPause(bool pause)
	{
		if (_pause==pause)
		{
			return;
		}

		_pause = pause;

		if (_firstTime==DBL_MAX) return;

		if (_pause)
		{
			_pauseTime = _latestTime;
		}
		else
		{
			_firstTime += (_latestTime-_pauseTime);
		}
	}


	bool AnimationPathCallback::IsFinish()
	{		
		return _blFinish;
	}

	bool AnimationPathCallback::SetAnimationLoopMode( AnimationPath::LoopMode mode )
	{
		if(!_animationPath)
		{
			return false;
		}
		_animationPath->setLoopMode(mode);

// 		switch(mode+1)
// 		{
// 		case 0:
// 			{
// 				_animationPath->setLoopMode(FeUtil::AnimationPath::SWING);
// 			}
// 			break;
// 
// 		case 1:
// 			{
// 				_animationPath->setLoopMode(FeUtil::AnimationPath::LOOP);
// 			}
// 			break;
// 
// 		case 2:
// 			{
// 				_animationPath->setLoopMode(FeUtil::AnimationPath::NO_LOOPING);
// 			}
// 			break;
// 
// 		default:
// 			break;
// 		}

		return true;
	}

	void AnimationPathCallback::SetFrameCalculate(bool bCalculate)
	{
		_blCalculate = bCalculate;
	}

	void AnimationPathCallback::SetHeightOffset(double dHeiOffset)
	{
		m_dHeiOffset = dHeiOffset;
	}

	osg::Vec3d AnimationPathCallback::GetCurrPosition()
	{
		return m_vecLLAPos;
	}

	osg::Vec3d AnimationPathCallback::GetDet()
	{
		return m_vecDet;
	}

	osg::Vec3d AnimationPathCallback::GetCurrWorld()
	{
		return m_vecWorldPos;
	}

	osg::Matrixd AnimationPathCallback::GetCurrMatrix()
	{
		return m_matrix;
	}


}