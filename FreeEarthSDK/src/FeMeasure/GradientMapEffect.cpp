#include <FeMeasure/GradientMapEffect.h>

#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarth/VirtualProgram>
#include <osgEarth/TerrainEngineNode>
#include <osgDB/ConvertUTF>
#include <osgDB/Registry>
#include <FeUtils/PathRegistry.h>
#include <osgEarth/GeoMath>
#include <time.h>
#include <stdlib.h>
#include <osgDB/WriteFile>
#include <osgUtil/PerlinNoise>
#include <FeUtils/CoordConverter.h>
//#include <FeUtils/FlightMotionAlgorithm.h>

#define LC "[CGradientMapEffect] "

using namespace osgEarth;
using namespace osgEarth::Util;

namespace FeMeasure
{
	osg::Image* Create3DNoiseImage(int texSize)
	{
		osg::Image* image = new osg::Image;
		image->setImage(texSize, texSize, texSize,
			4, GL_RGBA, GL_UNSIGNED_BYTE,
			new unsigned char[4 * texSize * texSize * texSize],
			osg::Image::USE_NEW_DELETE);

		const int startFrequency = 4;
		const int numOctaves = 4;

		int f, i, j, k, inc;
		double ni[3];
		double inci, incj, inck;
		int frequency = startFrequency;
		GLubyte *ptr;
		double amp = 0.5;

		osg::notify(osg::INFO) << "creating 3D noise texture... ";

		osgUtil::PerlinNoise pn;
		for (f = 0, inc = 0; f < numOctaves; ++f, frequency *= 2, ++inc, amp *= 0.5)
		{
			pn.SetNoiseFrequency(frequency);
			//SetNoiseFrequency(frequency);
			ptr = image->data();
			ni[0] = ni[1] = ni[2] = 0;

			inci = 1.0 / (texSize / frequency);
			for (i = 0; i < texSize; ++i, ni[0] += inci)
			{
				incj = 1.0 / (texSize / frequency);
				for (j = 0; j < texSize; ++j, ni[1] += incj)
				{
					inck = 1.0 / (texSize / frequency);
					for (k = 0; k < texSize; ++k, ni[2] += inck, ptr += 4)
					{
						*(ptr + inc) = (GLubyte)(((pn.noise3(ni) + 1.0) * amp) * 128.0);
					}
				}
			}
		}

		osg::notify(osg::INFO) << "DONE" << std::endl;
		return image;
	}

	osg::Texture3D* Create3DNoiseTexture(int texSize)
	{
		osg::Texture3D* noiseTexture = new osg::Texture3D;
		noiseTexture->setFilter(osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR);
		noiseTexture->setFilter(osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR);
		noiseTexture->setWrap(osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT);
		noiseTexture->setWrap(osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT);
		noiseTexture->setWrap(osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT);
		noiseTexture->setImage(Create3DNoiseImage(texSize));
		return noiseTexture;
	}

}

namespace FeMeasure
{
	CGradientMapEffect::CGradientMapEffect(osgEarth::MapNode* pMapNode) :
		osgEarth::TerrainEffect(),
		m_opMapNode(pMapNode)
	{
		Init();
		//SetViewCamera(pCamera);

		std::string filesPath = FeUtil::CFileReg::Instance()->GetFullPath("shaders/GradientMap");
		osgDB::Registry::instance()->getDataFilePathList().push_back(filesPath);
		m_shaderPackage.add("FragmentLighting.glsl", "gradient_map_fragment_lighting");
		m_shaderPackage.add("VertexView.glsl", "gradient_map_vertex_view");
		m_shaderPackage.add("CalculateXYZWidthLLDegree.glsl", "fe_xyz_to_lonlatdegree");
		m_shaderPackage.add("ConvertHSV2RGB.glsl", "ConvertHSV2RGB");
	}

	CGradientMapEffect::~CGradientMapEffect()
	{
		//nop
	}

	void CGradientMapEffect::onInstall(osgEarth::TerrainEngineNode* engine)
	{
		if (m_bOnInstall)
		{
			return;
		}

		if (engine)
		{
			osg::StateSet* stateset = engine->getOrCreateStateSet();

			stateset->setTextureAttribute(1, m_rpNoise3DTexture);
			stateset->addUniform(m_rpUniformNoise);

			VirtualProgram* vp = VirtualProgram::getOrCreate(stateset);

			if (m_opLayer.valid())
			{
#ifdef _DEBUG
#ifdef _WIN32
				std::string str = osgDB::convertStringFromUTF8toCurrentCodePage(m_opLayer->getName());
				OE_NOTICE << LC << "Installing layer " << str << " as normal map" << std::endl;
#endif
#ifndef _WIN32
				OE_NOTICE << LC << "Installing layer " << m_opLayer->getName() << " as normal map" << std::endl;
#endif
#endif // _DEBUG
				int unit = *m_opLayer->shareImageUnit();
				m_rpSamplerUniform = stateset->getOrCreateUniform("oe_nmap_normalTex", osg::Uniform::SAMPLER_2D);
				m_rpSamplerUniform->set(unit);
				//OSG_NOTICE << "unti " << unit << std::endl;
			}

			// these special (built-in) function names are for the main lighting shaders.
			// using them here will override the default lighting.
			m_shaderPackage.load(vp, "FragmentLighting.glsl");
			m_shaderPackage.load(vp, "VertexView.glsl");
			if (m_rpLongituteRange.valid())
			{
				stateset->addUniform(m_rpLongituteRange);
				stateset->addUniform(m_rpLatitudeRange);
				stateset->addUniform(m_rpLonLatWidth);
				stateset->addUniform(m_rpArrowSize);
				stateset->addUniform(m_rpBorderLineSize);
			}
		}
		m_bOnInstall = true;
	}

	void CGradientMapEffect::onUninstall(osgEarth::TerrainEngineNode* engine)
	{
		if (!m_bOnInstall)
		{
			return;
		}

		if (engine)
		{
			osg::StateSet* stateset = engine->getStateSet();
			if (stateset)
			{

				stateset->removeTextureAttribute(1, m_rpNoise3DTexture);
				stateset->removeUniform(m_rpUniformNoise);

				VirtualProgram* vp = VirtualProgram::get(stateset);
				if (vp)
				{
					stateset->removeUniform(m_rpSamplerUniform.get());
					if (m_rpLongituteRange.valid())
					{
						stateset->removeUniform(m_rpLongituteRange);
						stateset->removeUniform(m_rpLatitudeRange);
						stateset->removeUniform(m_rpLonLatWidth);
						stateset->removeUniform(m_rpArrowSize);
						stateset->removeUniform(m_rpBorderLineSize);
					}

					m_shaderPackage.unload(vp, "VertexView.glsl");
					m_shaderPackage.unload(vp, "FragmentLighting.glsl");
				}
			}
		}
		m_bOnInstall = false;
	}

	void CGradientMapEffect::Init()
	{
		m_bOnInstall = false;

		m_rpLongituteRange = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "longituteRange");
		m_longituteRange = osg::Vec3f(120.117255, 122.059630, 1.0);
		m_rpLongituteRange->set(m_longituteRange);

		m_rpLatitudeRange = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "latitudeRange");
		m_latitudeRange = osg::Vec3f(21.732352, 25.422586, 1.0);
		//m_latitudeRange = osg::Vec2f(70.732352, 90.0);
		m_rpLatitudeRange->set(m_latitudeRange);

		m_rpLonLatWidth = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "lonLatWidth");
		//m_lonLatWidth = osg::Vec2f(5000.0, 5000.0);
		m_lonLatWidth = osg::Vec2f(50000.0, 50000.0);
		m_rpLonLatWidth->set(m_lonLatWidth);

		m_rpArrowSize = new osg::Uniform(osg::Uniform::FLOAT, "fArrowSize");
		m_fArrowSize = 5000.0;
		m_rpArrowSize->set(m_fArrowSize);

		m_rpBorderLineSize = new osg::Uniform(osg::Uniform::FLOAT, "fBorderLineSize");
		m_rpBorderLineSize->set(1.0f);

		// 噪声纹理
		//m_rpNoise3DTexture = osgUtil::create3DNoiseTexture(32 /*128*/);
		m_rpNoise3DTexture = FeMeasure::Create3DNoiseTexture(32 /*128*/);
		m_rpUniformNoise = new osg::Uniform("NoiseTex", 1);
	}

	void CGradientMapEffect::SetNormalMapConfig(const osgEarth::Config& conf, osgEarth::Map* map)
	{
		conf.getIfSet("layer", m_strLayerName);

		if (map && m_strLayerName.isSet())
		{
			m_opLayer = (map->getImageLayerByName(*m_strLayerName));
		}
	}

	void CGradientMapEffect::SetLonLatRange(osg::Vec3f lonRange, osg::Vec3f latRange)
	{
		if (m_rpLongituteRange.valid() && m_rpLatitudeRange.valid())
		{
			m_longituteRange = lonRange;
			m_rpLongituteRange->set(m_longituteRange);
			m_latitudeRange = latRange;
			m_rpLatitudeRange->set(m_latitudeRange);
		}
	}

	void CGradientMapEffect::SetLonLatWidth(osg::Vec2f lonLatWidth)
	{
		m_lonLatWidth = lonLatWidth;
		m_rpLonLatWidth->set(m_lonLatWidth);
	}

	void CGradientMapEffect::SetArrowSize(float arrowSize)
	{
		m_fArrowSize = arrowSize;
		m_rpArrowSize->set(m_fArrowSize);
	}

	void CGradientMapEffect::GetLonLatRange(osg::Vec3f& lonRange, osg::Vec3f& latRange)
	{
		lonRange = m_longituteRange;
		latRange = m_latitudeRange;
	}

	void CGradientMapEffect::CalculateGradientInfo(const osg::Vec2d llDegree, osg::Vec2d &gradientInfo)
	{
		// 计算当前点的高度
		if (!m_opMapNode.valid()) { return; }

		double dHeight = 0.0;
		m_opMapNode->getTerrain()->getHeight(m_opMapNode->getMapSRS(), llDegree.x(), llDegree.y(), &dHeight);

		double degreeDelta = 15.0;
		int pointsNum = int(360.0 / degreeDelta);
		double dDistance = 500.0;
		
		float maxSlope = 0.0;
		osg::Vec2d llRadian = osg::Vec2d(osg::DegreesToRadians(llDegree.x()), osg::DegreesToRadians(llDegree.y()));
		for (int i = 0; i < pointsNum; ++i)
		{
			// 计算经纬度
			//double curDegree = i*degreeDelta;
			double curDegree = double(rand()) / (double)(RAND_MAX);
			curDegree *= 360.0;
			osg::Vec2d curRadians;
			osgEarth::GeoMath::destination(llRadian.y(), llRadian.x(), osg::DegreesToRadians(curDegree), dDistance, curRadians.y(), curRadians.x());
			double curHeight = 0.0;
			m_opMapNode->getTerrain()->getHeight(m_opMapNode->getMapSRS(), osg::RadiansToDegrees(curRadians.x()), osg::RadiansToDegrees(curRadians.y()), &curHeight);
			if (fabs(dHeight - curHeight) > maxSlope)
			{
				maxSlope = fabs(dHeight - curHeight);
				gradientInfo.y() = curDegree;
                osg::Vec3d a(llRadian, dHeight);
                osg::Vec3d b(curRadians, curHeight);
                gradientInfo.x() = CalculatePitchDegree(a, b);
			}
		}
	}

	double CGradientMapEffect::CalculatePitchDegree(osg::Vec3d& firstRadian, osg::Vec3d& secondRadian)
	{
		if (!m_opMapNode.valid())
		{
			return 0.0;
		}

		// 记录第一个点转换为世界坐标后的值
		osg::Vec3d firstXYZd;
		m_opMapNode->getMapSRS()->getEllipsoid()->convertLatLongHeightToXYZ(
			firstRadian.y(), firstRadian.x(), firstRadian.z(), firstXYZd.x(), firstXYZd.y(), firstXYZd.z());
		// 记录第二个点转换为世界坐标后的值
		osg::Vec3d secondXYZd;
		m_opMapNode->getMapSRS()->getEllipsoid()->convertLatLongHeightToXYZ(
			secondRadian.y(), secondRadian.x(), secondRadian.z(), secondXYZd.x(), secondXYZd.y(), secondXYZd.z());

		// 记录第二点下面一点的位置
		osg::Vec3d secondBottom(secondRadian.x(), secondRadian.y(), firstRadian.z()), secondBottomXYZd;
		m_opMapNode->getMapSRS()->getEllipsoid()->convertLatLongHeightToXYZ(
			secondBottom.y(), secondBottom.x(), secondBottom.z(), secondBottomXYZd.x(), secondBottomXYZd.y(), secondBottomXYZd.z());

		if (fabs(firstRadian.z() - secondRadian.z()) <= DBL_EPSILON)
		{
			return 0.0;
		}
		else
		{
			long double distance = sqrtl((firstXYZd-secondXYZd).length2());
			long double detal = sqrtl((secondXYZd-secondBottomXYZd).length2());
			double pitchRadian = 0.0;
			if (detal >= distance)
			{
				if (detal > 0.0) {
					return 90.0;
				}
				else {
					return -90.0;
				}
			}
			else
			{
				pitchRadian = asinl(detal / distance);
			}
			if (secondRadian.z() < firstRadian.z() && pitchRadian > 0.0)
			{
				pitchRadian = -pitchRadian;
			}
			return osg::RadiansToDegrees(pitchRadian);
		}
		return 0.0;
	}

	void CGradientMapEffect::GetLonLatWidth(osg::Vec2f& lonLatWidth)
	{
		lonLatWidth = m_lonLatWidth;
	}

}
