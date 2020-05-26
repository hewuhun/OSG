/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef FEUUTIL_ANIMATION_PATH_MANIPULATOR
#define FEUUTIL_ANIMATION_PATH_MANIPULATOR 1

//#include <FeUtils/MiniAnimation.h>
#include <osg/Notify>
#include <osg/AnimationPath>
#include <osgGA/CameraManipulator>
#include <FeKits/Export.h>

namespace FeKit
{

//
// The AnimationPathManipulator is a Matrix Manipulator that reads an
// animation path from a file and plays it back.  The file is expected
// to be ascii and a succession of lines with 8 floating point values
// per line.  The succession of values are:
// time  px py pz ax ay az aw
// where:
//    time = elapsed time in seconds from the beginning of the animation
//    px py pz = World position in cartesian coordinates
//    ax ay az aw = Orientation (attitude) defined as a quaternion

class FEKIT_EXPORT AnimationPathManipulator : public osgGA::CameraManipulator
{
    public:

        AnimationPathManipulator( osg::AnimationPath* animationPath=0 );

        AnimationPathManipulator( const std::string& filename );

        virtual const char* className() const { return "AnimationPath"; }

        void setTimeScale(double s) { _timeScale = s; }
        double getTimeScale() const { return _timeScale; }

        void setTimeOffset(double o) { _timeOffset = o; }
        double getTimeOffset() const { return _timeOffset; }

        struct AnimationCompletedCallback : public virtual osg::Referenced
        {
            virtual void completed(const AnimationPathManipulator* apm) = 0;
        };

        void setAnimationCompletedCallback(AnimationCompletedCallback* acc) { _animationCompletedCallback = acc; }
        AnimationCompletedCallback* getAnimationCompletedCallback() { return _animationCompletedCallback.get(); }
        const AnimationCompletedCallback* getAnimationCompletedCallback() const { return _animationCompletedCallback.get(); }

        void setPrintOutTimingInfo(bool printOutTimingInfo) { _printOutTimingInfo=printOutTimingInfo; }
        bool getPrintOutTimingInfo() const { return _printOutTimingInfo; }

        /** set the position of the matrix manipulator using a 4x4 Matrix.*/
        virtual void setByMatrix(const osg::Matrixd& matrix) { _matrix = matrix; }

        /** set the position of the matrix manipulator using a 4x4 Matrix.*/
        virtual void setByInverseMatrix(const osg::Matrixd& matrix) { _matrix.invert(matrix); }

        /** get the position of the manipulator as 4x4 Matrix.*/
        virtual osg::Matrixd getMatrix() const { return _matrix; }

        /** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
        virtual osg::Matrixd getInverseMatrix() const { return osg::Matrixd::inverse(_matrix); }


        void setAnimationPath( osg::AnimationPath* animationPath ) { _animationPath=animationPath; }

        osg::AnimationPath* getAnimationPath() { return _animationPath.get(); }

        const osg::AnimationPath* getAnimationPath() const { return _animationPath.get(); }

        bool valid() const { return _animationPath.valid(); }

        void init(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

        void home(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);
        void home(double currentTime);

        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

        /** Get the keyboard and mouse usage of this manipulator.*/
        virtual void getUsage(osg::ApplicationUsage& usage) const;

		void Start();

		void Stop();

		void Paused();

		void Acceleration();

		void SlowDown();

		bool IsRun();

    protected:

        bool _valid;

        bool _printOutTimingInfo;

        void handleFrame( double time );
		
        osg::ref_ptr<osg::AnimationPath> _animationPath;

        double  _timeOffset;
        double  _timeScale;

        osg::ref_ptr<AnimationCompletedCallback> _animationCompletedCallback;

        double  _pauseTime;
        bool    _isPaused;

        double  _realStartOfTimedPeriod;
        double  _animStartOfTimedPeriod;
        int     _numOfFramesSinceStartOfTimedPeriod;

        osg::Matrixd _matrix;

		bool						m_blRun;

		const osgGA::GUIEventAdapter     * _ea;
		osgGA::GUIActionAdapter    *_us;

};

}

#endif//FEUUTIL_ANIMATION_PATH_MANIPULATOR
