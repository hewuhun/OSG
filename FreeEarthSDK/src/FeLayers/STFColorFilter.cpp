#include <FeLayers/STFColorFilter.h>

#include <OpenThreads/Atomic>
#include <osg/Program>
#include <osgEarthUtil/RGBColorFilter>
#include <osgEarth/VirtualProgram>
#include <osgEarth/StringUtils>
#include <osgEarth/ThreadingUtils>

using namespace osgEarth;
using namespace osgEarth::Util;

namespace FeLayers
{
	namespace
	{
		static OpenThreads::Atomic s_uniformNameGen;

		static const char* s_localShaderSource =
			"#version 110\n"
			"uniform vec3 __UNIFORM_NAME__;\n"
			"void colorBlend(in float scale, in float dest, in float src, out float outcolor)\n"
			"{\n"
			"outcolor =  (scale * dest + (1.0 - scale) * src);\n"
			"}\n"
			"float myNoise()\n"
			"{\n"
			"return 1.0;\n"
			"}\n"
			"void __ENTRY_POINT__(inout vec4 color)\n"
			"{\n"
			"    color.rgb = clamp(color.rgb + __UNIFORM_NAME__.rgb, 0.0, 1.0); \n"
			"float fr = color.r * 0.393  + color.g * 0.769 + color.b * 0.189;\n"
			"float fg = color.r * 0.349 + color.g * 0.686 + color.b * 0.168;\n"
			"float fb = color.r * 0.272 + color.g * 0.534 + color.b * 0.132;\n"
			"float or;\n"
			"float og;\n"
			"float ob;\n"
			"colorBlend(myNoise(), fr, color.r, or);\n"
			"colorBlend(myNoise(), fg, color.g, og);\n"
			"colorBlend(myNoise(), fb, color.b, ob);\n"
			"color.rgb = vec3(or, og, ob);\n"
			"} \n";
	}


#define FUNCTION_PREFIX "osgearthutil_rgbColorFilter_"
#define UNIFORM_PREFIX  "osgearthutil_u_rgb_"

	CSTFColorFilter::CSTFColorFilter(void)
	{
		init();
	}

	CSTFColorFilter::CSTFColorFilter(const Config& conf)
	{
		init();

		osg::Vec3f val;
		val[0] = conf.value("r", 0.0);
		val[1] = conf.value("g", 0.0);
		val[2] = conf.value("b", 0.0);
		setRGBOffset( val );
	}

	void CSTFColorFilter::init()
	{
		m_instanceId = (++s_uniformNameGen) - 1;
		m_rgb = new osg::Uniform(osg::Uniform::FLOAT_VEC3, (osgEarth::Stringify() << UNIFORM_PREFIX << m_instanceId));
		m_rgb->set(osg::Vec3f(0.0f, 0.0f, 0.0f));
	}

	void CSTFColorFilter::setRGBOffset(const osg::Vec3f& value)
	{
		m_rgb->set(value);
	}

	osg::Vec3f CSTFColorFilter::getRGBOffset(void) const
	{
		osg::Vec3f value;
		m_rgb->get(value);
		return (value);
	}

	std::string CSTFColorFilter::getEntryPointFunctionName(void) const
	{
		return (osgEarth::Stringify() << FUNCTION_PREFIX << m_instanceId);
	}

	void CSTFColorFilter::install(osg::StateSet* stateSet) const
	{
		stateSet->addUniform(m_rgb.get());

		osgEarth::VirtualProgram* vp = dynamic_cast<osgEarth::VirtualProgram*>(stateSet->getAttribute(VirtualProgram::SA_TYPE));
		if (vp)
		{
			std::string entryPoint = osgEarth::Stringify() << FUNCTION_PREFIX << m_instanceId;
			std::string code = s_localShaderSource;
			osgEarth::replaceIn(code, "__UNIFORM_NAME__", m_rgb->getName());
			osgEarth::replaceIn(code, "__ENTRY_POINT__", entryPoint);

			osg::Shader* main = new osg::Shader(osg::Shader::FRAGMENT, code);
			//main->setName(entryPoint);
			vp->setShader(entryPoint, main);
		}
	}

	Config CSTFColorFilter::getConfig() const
	{
		osg::Vec3f val = getRGBOffset();
		Config conf("rgb");
		conf.add( "r", val[0] );
		conf.add( "g", val[1] );
		conf.add( "b", val[2] );
		return conf;
	}

	OSGEARTH_REGISTER_COLORFILTER( stf, osgEarth::Util::RGBColorFilter );
}