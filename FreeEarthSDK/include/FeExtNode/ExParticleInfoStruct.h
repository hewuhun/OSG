/**************************************************************************************************
* @file ExParticleInfoStruct.h
* @note 粒子信息结构体
* @author w00040
* @data 2017-3-16
**************************************************************************************************/
#ifndef FE_PARTICLE_INFO_STRUCT_H
#define FE_PARTICLE_INFO_STRUCT_H

#include <osg/Shader>
#include <osg/Group>
#include <osg/AnimationPath>
#include <osg/MatrixTransform>
#include <OpenThreads/Mutex>
#include <map>

///浮点数比较
#define FLOAT_EQUAL(x, y)  ((x<y+0.000001) && (x>y-0.000001))

//粒子配置
#define PARTICLE_LIFE_MIN 0
#define PARTICLE_LIFE_MAX 1000
#define PARTICLE_LIFE_STEP 0.1
#define PARTICLE_DEGREE_STEP 1

#define PARTICLE_SPEED   2000
#define PARTICLE_ANGLE   360

#define PARTICLE_LIFE_START_MIN 0
#define PARTICLE_LIFE_START_MAX 1000
#define PARTICLE_LIFE_GAP  10.0

//颜色配置
#define PARTICLE_COLOR_STEP  1
#define PARTICLE_COLOR_MIN   0
#define PARTICLE_COLOR_MAX   255.0
#define PARTICLE_COLOR_SCALE 255

#define PARTICLE_PLACE_MIN  -200.0
#define PARTICLE_PLACE_MAX  200.0
#define PARTICLE_PLACE_STEP 20.0

//发射
#define PARTICLE_SHOOT_LIFE_MIN -1
#define PARTICLE_SHOOT_LIFE_MAX 100

#define PARTICLE_SHOOT_START_MIN 0
#define PARTICLE_SHOOT_START_MAX 100

#define PARTICLE_SHOOT_PHI_MIN 0
#define PARTICLE_SHOOT_PHI_MAX 360

#define PARTICLE_SHOOT_SPEED_MIN 0
#define PARTICLE_SHOOT_SPEED_MAX 1000

#define PARTICLE_SHOOT_COUNT_MIN 0
#define PARTICLE_SHOOT_COUNT_MAX 5000


#define PARTICLE_SHOOT_RORATE_MIN -180
#define PARTICLE_SHOOT_RORATE_MAX 180

#define FREE_MAX_PATH  256
#define PARTICLE_NAME_LENGHT  30
#define MAX_TEXTURE_NUM       20      //每个粒子最多包含纹理数

#define ACCEL_OPERATOR           0    //加速度
#define ANGULAR_ACCEL_OPAERATOR  1    //角加速度
#define ANGULAR_DAMPING_OPERATOR 2    //角加速阻尼
#define DAMPING_OPERATOR         3    //弹力
#define EXPLOSION_OPERATOR       4    //爆炸
#define FLUID_FRICTION_OPERATOT  5    //水流
//#define FRICTION_OPERATOR      6    //气流
#define FORCE_OPERATOR           6    //力
#define ORBIT_OPERATOR           7    //轨道
#define BOUNCE_OPERATOR          8    //弹力
#define SINK_OPERATOR            9    //下沉


#define DOMIAN_POINT             0    //点
#define DOMIAN_LINE              1    //线
#define DOMIAN_TRIANGLE          2    //三角形
#define DOMIAN_RECT              3    //矩形
#define DOMIAN_PLANE             4    //平面
#define DOMIAN_SPHERE            5    //球
#define DOMIAN_BOX               6    //盒子
#define DOMIAN_DISK              7    //碟子


#define OPERATOR_STEP            0.05
#define OPERATOR_X               2000
#define OPERATOR_Y               2000
#define OPERATOR_Z               2000
#define OPERATOR_SCALE           20.0

#define OPERATOR_GRAVITY         2000

#define OPERATOR_LOW             2000
#define OPERATOR_HEIGHT          2000

#define OPERATOR_RESILIENCE      2000
#define OPERATOR_FRECTION        2000

#define OPERATOR_RADIUS          2000
#define OPERATOR_MAGNITUDE       2000

#define OPERATOR_DENSITY         2000

#define OPERATOR_SPEED           2000

#define OPERATOR_EPSLOSON        2000

#define OPERATOR_SIGMA           2000

#define OPERATOR_DOMAIN          1000
#define OPERATOR_DOMIAN_SCALE    10.0
#define OPERATOR_DOMIAN_STEP     0.1

typedef struct _ST_3_FLOAT
{
	float fX;
	float fY;
	float fZ;
}ST_3_FLOAT,*PST_3_FLOAT;

//粒子配置信息
typedef struct _ST_PARTICLE_OPITION
{
	float fLife;	            //生命	
	float fSizeStart;           //生命开始大小
	float fSizeEnd;             //生命结束大小
	float fAlphaStart;           //开始透明度
	float fAlphaEnd;             //结束透明度
	float fX;                   //初始位置
	float fY;
	float fZ;
	float fRadius;              //半径
	float fMass;                 //质量
	ST_3_FLOAT sAngle;           //角度
	ST_3_FLOAT sSpeed;           //速度
	ST_3_FLOAT sAngular;        //角角速度
	float fDepth;               //深度
}ST_PARTICLE_OPITION,*PST_PARTICLE_OPITION;

//粒子颜色信息
typedef struct _ST_COLOR_OPITION
{
	float fBeginAlpha;	            //开始透明度	
	float fBeginR;                 //
	float fBeginG;                 //
	float fBeginB;                 //
	float fEndAlpha;	            //结束透明度	
	float fEndR;                    //
	float fEndG;                   //
	float fEndB;                    //
}ST_COLOR_OPITION,*PST_COLOR_OPITION;

//发射器配置
typedef struct _ST_SHOOT_OPITION 
{
	float fLife;
	float fRadiusMin;
	float fRadiusMax;
	float fPhiMin;
	float fPhiMax;
	float fSpeedMin;
	float fSpeedMax;
	float fCountMin;
	float fCountMax;
	float fPhiRangeMin;
	float fPhiRangeMax;
	float fThetaRangeMin;
	float fThetaRangeMax;
	float fRotateBeginX;
	float fRotateBeginY;
	float fRotateBeginZ;
	float fRotateEndX;
	float fRotateEndY;
	float fRotateEndZ;
	float fStartTime;
}ST_SHOOT_OPITION ,*PST_SHOOT_OPITION ;


typedef struct _ST_POINT
{
	unsigned int unNO;
	ST_3_FLOAT s3F;

}ST_POINT,*PST_POINT;

typedef struct _ST_LINE
{
	unsigned int unNO;
	ST_3_FLOAT s3FV1;
	ST_3_FLOAT s3FV2;

}ST_LINE,*PST_LINE;

typedef struct _ST_TRIANGLE
{
	unsigned int unNO;
	ST_3_FLOAT s3FV1;
	ST_3_FLOAT s3FV2;
	ST_3_FLOAT s3FV3;

}ST_TRIANGLE,*PST_TRIANGLE;

typedef struct _ST_RECT
{
	unsigned int unNO;
	ST_3_FLOAT s3FV1;
	ST_3_FLOAT s3FV2;
	ST_3_FLOAT s3FV3;

}ST_RECT,*PST_RECT;

typedef struct _ST_PLANE
{
	unsigned int unNO;
	ST_3_FLOAT s3FV1;
	float fRadus;

}ST_PLANE,*PST_PLANE;

typedef struct _ST_SPHERE
{
	unsigned int unNO;
	ST_3_FLOAT s3FV1;
	float fRadus;

}ST_SPHERE,*PST_SPHERE;

typedef struct _ST_BOX
{
	unsigned int unNO;
	ST_3_FLOAT s3FV1;
	ST_3_FLOAT s3FV2;

}ST_BOX,*PST_BOX;

typedef struct _ST_DISK
{
	unsigned int unNO;
	ST_3_FLOAT s3FV1;
	ST_3_FLOAT s3FV2;
	float fMin;
	float fMax;

}ST_DISK,*PST_DISK;

typedef struct _ST_EFFECT_OPITION_DOMIAN
{
	bool bPoint;
	ST_POINT sPoint;

	bool bLine;
	ST_LINE sLine;

	bool bTri;
	ST_TRIANGLE sTri;

	bool bRect;
	ST_RECT sRect;

	bool bPlane;
	ST_PLANE sPlane;

	bool bSphere;
	ST_SPHERE sSphere;

	bool bBox;
	ST_BOX stBox;

	bool bDisk;
	ST_DISK sDisk;

}ST_EFFECT_OPITION_DOMIAN,*PST_EFFECT_OPITION_DOMIAN;

typedef struct _ST_ACCEL
{
	ST_3_FLOAT s3F;
	float fGravity;

}ST_ACCEL,*PST_ACCEL;

typedef struct _ST_ANGULAR_ACCEL
{
	ST_3_FLOAT s3F;

}ST_ANGULAR_ACCEL,*PST_ANGULAR_ACCEL;

typedef struct _ST_ANGULAR_DAMPING
{
	ST_3_FLOAT s3F;
	float fLow;
	float fHeight;

}ST_ANGULAR_DAMPING,*PST_ANGULAR_DAMPING;

typedef struct _ST_DAMPING
{
	ST_3_FLOAT s3F;
	float fLow;
	float fHeight;

}ST_DAMPING,*PST_DAMPING;

typedef struct _ST_EXPLOSION
{
	ST_3_FLOAT s3F;
	float fRadius;
	float fMagnitude;
	float fExpsilon;
	float fSigma;

}ST_EXPLOSION,*PST_EXPLOSION;

typedef struct _ST_FLUID_FRICTION
{
	bool bFliud;
	ST_3_FLOAT s3F;
	float fDensity;
	float fSpeed;
	float fRadius;

}ST_FLUID_FRICTION,*PST_FLUID_FRICTION;

typedef struct _ST_FORCE
{
	ST_3_FLOAT s3F;

}ST_FORCE,*PST_FORCE;

typedef struct _ST_ORBIT
{
	ST_3_FLOAT s3F;
	float fMagnitude;
	float fExpsilon;
	float fRadius;

}ST_ORBIT,*PST_ORBIT;


typedef struct _ST_BOUNCE
{
	float fResilience;
	float fFriction;
	float fCutOff;
	unsigned int uiCount;
	ST_EFFECT_OPITION_DOMIAN sDomian;

}ST_BOUNCE,*PST_BOUNCE;

typedef struct _ST_SINK
{
	bool bStrategyOutSide;
	unsigned int uiCount;
	ST_EFFECT_OPITION_DOMIAN sDomian;

}ST_SINK,*PST_SINK;

typedef struct _ST_EFFECT_OPITION
{
	bool bAccel;
	ST_ACCEL sAccel;

	bool bAngularAceel;
	ST_ANGULAR_ACCEL sAngularAceel;

	bool bAngularAccelDamping;
	ST_ANGULAR_DAMPING sAngularDamping;

	bool bDamping;
	ST_DAMPING sDamping;

	bool bExplosion;
	ST_EXPLOSION sExplosion;

	bool bFluidFriction;
	ST_FLUID_FRICTION sFluid;

	bool bForce;
	ST_FORCE sForce;

	bool bOrbit;
	ST_ORBIT sOrbit;

	bool bBounce;
	ST_BOUNCE sBounce;

	bool bSink;
	ST_SINK stSink;

}ST_EFFECT_OPITION,*PST_EFFECT_OPITION;

typedef struct _ST_PARTICLE_INFO
{
	unsigned int unID;                                 //粒子ID
	char szTexture[FREE_MAX_PATH] ;                    //纹理
	ST_PARTICLE_OPITION  sParticleOpt;
	ST_COLOR_OPITION     sColorOpt;
	ST_SHOOT_OPITION     sShootOpt;
	ST_EFFECT_OPITION    sEffectOpt; 
}ST_PARTICLE_INFO,*PST_PARTICLE_INFO;


typedef struct _ST_PARTICLE_TEMPLATE_INFO
{
	unsigned int uiParticleID;                         //粒子模板ID
	char szName[PARTICLE_NAME_LENGHT];  
	unsigned int unNum;                                //纹理数
	ST_PARTICLE_INFO asParticleInfo[MAX_TEXTURE_NUM];

}ST_PARTICLE_TEMPLATE_INFO,*PST_PARTICLE_TEMPLATE_INFO;

#endif