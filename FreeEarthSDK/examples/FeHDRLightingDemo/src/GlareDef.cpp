﻿// File provided by Masaki Kawase 
// http://www.daionet.gr.jp/~masa/rthdribl/

// GlareDefD3D.cpp : Glare information definition
//
#include <GlareDef.h>

using namespace FeHDRLighting;

// Static star library information
static STARDEF s_aLibStarDef[NUM_BASESTARLIBTYPES] =
{
    //  star name               lines   passes  length  attn    rotate          bRotate
    {   ( "Disable" ),        0,      0,      0.0f,   0.0f,   osg::DegreesToRadians( 00.0f ),    false,  },  // STLT_DISABLE

	{   ( "Cross" ),          4,      3,      1.00f,   0.85f,  osg::DegreesToRadians( 0.0f ),     true,   },  // STLT_CROSS
	{   ( "CrossFilter" ),    4,      3,      1.00f,   0.95f,  osg::DegreesToRadians( 0.0f ),     true,   },  // STLT_CROSS
	{   ( "snowCross" ),      6,      3,      1.00f,   0.96f,  osg::DegreesToRadians( 20.0f ),    true,   },  // STLT_SNOWCROSS
	{   ( "Vertical" ),       2,      3,      1.00f,   0.96f,  osg::DegreesToRadians( 00.0f ),    false,  },  // STLT_VERTICAL
};
static int                      s_nLibStarDefs = sizeof( s_aLibStarDef ) / sizeof( STARDEF );


// Static glare library information
static GLAREDEF s_aLibGlareDef[NUM_GLARELIBTYPES] =
{
    //  glare name                              glare   bloom   ghost   distort star    star type
    //  rotate          C.A     current after   ai lum
    {   ( "Disable" ),                        0.0f,   0.0f,   0.0f,   0.01f,  0.0f,   STLT_DISABLE,
        osg::DegreesToRadians( 0.0f ),     0.5f,   0.00f,  0.00f,  0.0f,  },   // GLT_DISABLE

    {   ( "Camera" ),                         1.5f,   1.2f,   1.0f,   0.00f,  1.0f,   STLT_CROSS,
        osg::DegreesToRadians( 00.0f ),    0.5f,   0.25f,  0.90f,  1.0f,  },   // GLT_CAMERA
    {   ( "Natural Bloom" ),                  1.5f,   1.2f,   0.0f,   0.00f,  0.0f,   STLT_DISABLE,
        osg::DegreesToRadians( 00.0f ),    0.0f,   0.40f,  0.85f,  0.5f,  },   // GLT_NATURAL
    {   ( "Cheap Lens Camera" ),              1.25f,  2.0f,   1.5f,   0.05f,  2.0f,   STLT_CROSS,
        osg::DegreesToRadians( 00.0f ),    0.5f,   0.18f,  0.95f,  1.0f,  },   // GLT_CHEAPLENS
    /*  
      {   TEXT("Afterimage"),                     1.5f,   1.2f,   0.5f,   0.00f,  0.7f,   STLT_CROSS,
      _Rad(00.0f),    0.5f,   0.1f,   0.98f,  2.0f,  },   // GLT_AFTERIMAGE
     */
    {   ( "Cross Screen Filter" ),            1.0f,   2.0f,   1.7f,   0.00f,  1.5f,   STLT_CROSSFILTER,
        osg::DegreesToRadians( 25.0f ),    0.5f,   0.20f,  0.93f,  1.0f,  },   // GLT_FILTER_CROSSSCREEN
    {   ( "Spectral Cross Filter" ),          1.0f,   2.0f,   1.7f,   0.00f,  1.8f,   STLT_CROSSFILTER,
        osg::DegreesToRadians( 70.0f ),    1.5f,   0.20f,  0.93f,  1.0f,  },   // GLT_FILTER_CROSSSCREEN_SPECTRAL
    {   ( "Snow Cross Filter" ),              1.0f,   2.0f,   1.7f,   0.00f,  1.5f,   STLT_SNOWCROSS,
        osg::DegreesToRadians( 10.0f ),    0.5f,   0.20f,  0.93f,  1.0f,  },   // GLT_FILTER_SNOWCROSS
    {   ( "Spectral Snow Cross" ),            1.0f,   2.0f,   1.7f,   0.00f,  1.8f,   STLT_SNOWCROSS,
        osg::DegreesToRadians( 40.0f ),    1.5f,   0.20f,  0.93f,  1.0f,  },   // GLT_FILTER_SNOWCROSS_SPECTRAL
    {   ( "Sunny Cross Filter" ),             1.0f,   2.0f,   1.7f,   0.00f,  1.5f,   STLT_SUNNYCROSS,
        osg::DegreesToRadians( 00.0f ),    0.5f,   0.20f,  0.93f,  1.0f,  },   // GLT_FILTER_SUNNYCROSS
    {   ( "Spectral Sunny Cross" ),           1.0f,   2.0f,   1.7f,   0.00f,  1.8f,   STLT_SUNNYCROSS,
        osg::DegreesToRadians( 45.0f ),    1.5f,   0.20f,  0.93f,  1.0f,  },   // GLT_FILTER_SUNNYCROSS_SPECTRAL
    {   ( "Cine Camera Vertical Slits" ),     1.0f,   2.0f,   1.5f,   0.00f,  1.0f,   STLT_VERTICAL,
        osg::DegreesToRadians( 90.0f ),    0.5f,   0.20f,  0.93f,  1.0f,  },   // GLT_CINECAM_VERTICALSLIT
    {   ( "Cine Camera Horizontal Slits" ),   1.0f,   2.0f,   1.5f,   0.00f,  1.0f,   STLT_VERTICAL,
        osg::DegreesToRadians( 00.0f ),    0.5f,   0.20f,  0.93f,  1.0f,  },   // GLT_CINECAM_HORIZONTALSLIT
};
static int                      s_nLibGlareDefs = sizeof( s_aLibGlareDef ) / sizeof( GLAREDEF );



//----------------------------------------------------------
// Information object for star generation 

CStarDef*                       CStarDef::ms_pStarLib = NULL;
osg::Vec4 CStarDef::ms_avChromaticAberrationColor[];


CStarDef::CStarDef()
{
    Construct();
}

CStarDef::CStarDef( const CStarDef& src )
{
    Construct();
    Initialize( src );
}

CStarDef::~CStarDef()
{
    Destruct();
}


bool CStarDef::Construct()
{
    m_nStarLines = 0;
    m_pStarLine = NULL;
    m_fInclination = 0.0f;

    m_bRotation = false;

    return true;
}

void CStarDef::Destruct()
{
    Release();
}

void CStarDef::Release()
{
    SAFE_DELETE_ARRAY( m_pStarLine );
    m_nStarLines = 0;
}


bool CStarDef::Initialize( const CStarDef& src )
{
    if( &src == this )
    {
        return true;
    }

    // Release the data
    Release();

    // Copy the data from source
    m_strStarName = src.m_strStarName;
    m_nStarLines = src.m_nStarLines;
    m_fInclination = src.m_fInclination;
    m_bRotation = src.m_bRotation;

    m_pStarLine = new STARLINE[m_nStarLines];
    if( m_pStarLine == NULL )
        return false;

    for( int i = 0; i < m_nStarLines; i ++ )
    {
        m_pStarLine[i] = src.m_pStarLine[i];
    }

    return true;
}


/// generic simple star generation
bool CStarDef::Initialize( const std::string szStarName,
                              int nStarLines,
                              int nPasses,
                              float fSampleLength,
                              float fAttenuation,
                              float fInclination,
                              bool bRotation )
{
    // Release the data
    Release();

    // Copy from parameters
    m_strStarName = szStarName;
    m_nStarLines = nStarLines;
    m_fInclination = fInclination;
    m_bRotation = bRotation;

    m_pStarLine = new STARLINE[m_nStarLines];
    if( m_pStarLine == NULL )
        return false;

    float fInc = osg::DegreesToRadians( 360.0f / ( float )m_nStarLines );
    for( int i = 0; i < m_nStarLines; i ++ )
    {
        m_pStarLine[i].nPasses = nPasses;
        m_pStarLine[i].fSampleLength = fSampleLength;
        m_pStarLine[i].fAttenuation = fAttenuation;
        m_pStarLine[i].fInclination = fInc * ( float )i;
    }

    return true;
}


/// Specific start generation
//  Sunny cross filter
bool CStarDef::Initialize_SunnyCrossFilter( const std::string szStarName,
                                               float fSampleLength,
                                               float fAttenuation,
                                               float fLongAttenuation,
                                               float fInclination )
{
    // Release the data
    Release();

    // Create parameters
    m_strStarName = szStarName;
    m_nStarLines = 8;
    m_fInclination = fInclination;
    //  m_bRotation     = true ;
    m_bRotation = false;

    m_pStarLine = new STARLINE[m_nStarLines];
    if( m_pStarLine == NULL )
        return false;

    float fInc = osg::DegreesToRadians( 360.0f / ( float )m_nStarLines );
    for( int i = 0; i < m_nStarLines; i ++ )
    {
        m_pStarLine[i].fSampleLength = fSampleLength;
        m_pStarLine[i].fInclination = fInc * ( float )i + osg::DegreesToRadians( 0.0f );

        if( 0 == ( i % 2 ) )
        {
            m_pStarLine[i].nPasses = 3;
            m_pStarLine[i].fAttenuation = fLongAttenuation;    // long
        }
        else
        {
            m_pStarLine[i].nPasses = 3;
            m_pStarLine[i].fAttenuation = fAttenuation;
        }
    }

    return true;
}

bool CStarDef::InitializeStaticStarLibs()
{
    if( ms_pStarLib )
    {
        return true;
    }

    ms_pStarLib = new CStarDef[NUM_STARLIBTYPES];
    if( ms_pStarLib == NULL )
        return false;

    // Create basic form
    for( int i = 0; i < NUM_BASESTARLIBTYPES; i ++ )
    {
        ms_pStarLib[i].Initialize( s_aLibStarDef[i] );
    }

    // Create special form
    // Sunny cross filter
    ms_pStarLib[STLT_SUNNYCROSS].Initialize_SunnyCrossFilter();

    // Initialize color aberration table
    /*
      {
      D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f),
      D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f),
      D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f),
      D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f),
      } ;
     */
    osg::Vec4 avColor[8] =
    {
        /*
          D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f),
          D3DXCOLOR(0.3f, 0.3f, 0.8f,  0.0f),
          D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f),
          D3DXCOLOR(0.2f, 0.4f, 0.5f,  0.0f),
          D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f),
          D3DXCOLOR(0.5f, 0.4f, 0.2f,  0.0f),
          D3DXCOLOR(0.7f, 0.3f, 0.2f,  0.0f),
          D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f),
         */

        osg::Vec4( 0.5f, 0.5f, 0.5f, 0.0f ), // w
        osg::Vec4( 0.8f, 0.3f, 0.3f, 0.0f ),
        osg::Vec4( 1.0f, 0.2f, 0.2f, 0.0f ), // r
        osg::Vec4( 0.5f, 0.2f, 0.6f, 0.0f ),
        osg::Vec4( 0.2f, 0.2f, 1.0f, 0.0f ), // b
        osg::Vec4( 0.2f, 0.3f, 0.7f, 0.0f ),
        osg::Vec4( 0.2f, 0.6f, 0.2f, 0.0f ), // g
        osg::Vec4( 0.3f, 0.5f, 0.3f, 0.0f ),
    };

    memcpy( ms_avChromaticAberrationColor, avColor, sizeof( osg::Vec4 ) * 8 );
    /*
      ms_avChromaticAberrationColor[0] = D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f) ;
      ms_avChromaticAberrationColor[1] = D3DXCOLOR(0.7f, 0.3f, 0.3f,  0.0f) ;
      ms_avChromaticAberrationColor[2] = D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f) ;
      ms_avChromaticAberrationColor[3] = D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f) ;
      ms_avChromaticAberrationColor[4] = D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f) ;
      ms_avChromaticAberrationColor[5] = D3DXCOLOR(0.2f, 0.4f, 0.5f,  0.0f) ;
      ms_avChromaticAberrationColor[6] = D3DXCOLOR(0.2f, 0.3f, 0.8f,  0.0f) ;
      ms_avChromaticAberrationColor[7] = D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f) ;
     */
    return true;
}

bool CStarDef::DeleteStaticStarLibs()
{
    // Delete all libraries
    SAFE_DELETE_ARRAY( ms_pStarLib );

    return true;
}



//----------------------------------------------------------
// Glare definition

CGlareDef*                      CGlareDef::ms_pGlareLib = NULL;

CGlareDef::CGlareDef()
{
    Construct();
}

CGlareDef::CGlareDef( const CGlareDef& src )
{
    Construct();
    Initialize( src );
}

CGlareDef::~CGlareDef()
{
    Destruct();
}


bool CGlareDef::Construct()
{
    m_fGlareLuminance = 0.0f;
    m_fBloomLuminance = 0.0f;
    m_fGhostLuminance = 0.0f;
    m_fStarLuminance = 0.0f;
    m_fStarInclination = 0.0f;
    m_fChromaticAberration = 0.0f;

    m_fAfterimageSensitivity = 0.0f;
    m_fAfterimageRatio = 0.0f;
    m_fAfterimageLuminance = 0.0f;

    return true;
}

void CGlareDef::Destruct()
{
    m_starDef.Release();
}

void CGlareDef::Release()
{
}

bool CGlareDef::Initialize( const CGlareDef& src )
{
    if( &src == this )
    {
        return true;
    }

    // Release the data
    Release();

    // Copy data from source
    m_strGlareName = src.m_strGlareName;
    m_fGlareLuminance = src.m_fGlareLuminance;

    m_fBloomLuminance = src.m_fBloomLuminance;
    m_fGhostLuminance = src.m_fGhostLuminance;
    m_fGhostDistortion = src.m_fGhostDistortion;
    m_fStarLuminance = src.m_fStarLuminance;
    m_fStarLuminance = src.m_fStarLuminance;
    m_fStarInclination = src.m_fStarInclination;
    m_fChromaticAberration = src.m_fChromaticAberration;

    m_fAfterimageSensitivity = src.m_fStarLuminance;
    m_fAfterimageRatio = src.m_fStarLuminance;
    m_fAfterimageLuminance = src.m_fStarLuminance;

    m_starDef = src.m_starDef;

    return true;
}


bool CGlareDef::Initialize( const std::string szStarName,
                               float fGlareLuminance,
                               float fBloomLuminance,
                               float fGhostLuminance,
                               float fGhostDistortion,
                               float fStarLuminance,
                               ESTARLIBTYPE eStarType,
                               float fStarInclination,
                               float fChromaticAberration,
                               float fAfterimageSensitivity, // Current weight
                               float fAfterimageRatio,       // Afterimage weight
                               float fAfterimageLuminance )
{
    // Release the data
    Release();

    // Create parameters
    m_strGlareName = szStarName;
    m_fGlareLuminance = fGlareLuminance;

    m_fBloomLuminance = fBloomLuminance;
    m_fGhostLuminance = fGhostLuminance;
    m_fGhostDistortion = fGhostDistortion;
    m_fStarLuminance = fStarLuminance;
    m_fStarInclination = fStarInclination;
    m_fChromaticAberration = fChromaticAberration;

    m_fAfterimageSensitivity = fAfterimageSensitivity;
    m_fAfterimageRatio = fAfterimageRatio;
    m_fAfterimageLuminance = fAfterimageLuminance;

    // Create star form data
    m_starDef = CStarDef::GetLib( eStarType );

    return true;
}


bool CGlareDef::InitializeStaticGlareLibs()
{
    if( ms_pGlareLib )
    {
        return true;
    }

    CStarDef::InitializeStaticStarLibs();
    ms_pGlareLib = new CGlareDef[NUM_GLARELIBTYPES];
    if( ms_pGlareLib == NULL )
        return false;

    // Create glare form
    for( int i = 0; i < NUM_GLARELIBTYPES; i ++ )
    {
        ms_pGlareLib[i].Initialize( s_aLibGlareDef[i] );
    }

    return true;
}


bool CGlareDef::DeleteStaticGlareLibs()
{
    // Delete all libraries
    SAFE_DELETE_ARRAY( ms_pGlareLib );

    return true;
}


//----------------------------------------------------------

// Dummy to generate static object for glare
__CGlare_GenerateStaticObjects  __CGlare_GenerateStaticObjects::ms_staticObject;
