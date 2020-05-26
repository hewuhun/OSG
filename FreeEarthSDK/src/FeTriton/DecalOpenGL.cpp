// Copyright (c) 2013-2014 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/DecalOpenGL.h>
#include <FeTriton/Environment.h>

using namespace Triton;

DecalOpenGL::DecalOpenGL(const Environment *env, TextureHandle pTexture, float pSize) :
    vboID(0), idxID(0), vertexArray(0), Decal(env, pTexture, pSize)
{

}

DecalOpenGL::~DecalOpenGL()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (vertexArray) {
				et->glDeleteVertexArrays(1, &vertexArray);
			}

			if (vboID) {
				et->glDeleteBuffers(1, &vboID);
			}

			if (idxID) {
				et->glDeleteBuffers(1, &idxID);
			}
		}
	}
}

void DecalOpenGL::BindArrays()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (environment->GetRenderer() >= OPENGL_3_2) 
			{
				et->glBindVertexArray(vertexArray);
				et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
				et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
			} 
			else
			{
				et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
				et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(4, GL_FLOAT, 0, 0);
			}
		}
	}
}

void DecalOpenGL::UnbindArrays()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (environment->GetRenderer() >= OPENGL_3_2)
			{
				et->glBindVertexArray(0);
			} 
			else
			{
				glDisableClientState(GL_VERTEX_ARRAY);
			}

			et->glBindBuffer(GL_ARRAY_BUFFER, 0);
			et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}
}

void DecalOpenGL::Draw(bool inside)
{
	if(vboID == 0 || idxID == 0)
		Setup();

	if(vboID != 0 && idxID != 0)
	{
		glDepthMask(GL_FALSE);

		if (inside) {
			glDisable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
		} else {
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, additive? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);

		BindArrays();

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glDepthMask(GL_TRUE);
	}
}

typedef struct BoxVertex_S {
    float x, y, z, w;
} BoxVertex;

void DecalOpenGL::Setup()
{
	if(!environment)
		return;

	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    BoxVertex v[8];

    Vector3 halfRight = obb.axes[0] * obb.halfDistances[0] * volumeScale;
    Vector3 halfUp = obb.axes[1] * obb.halfDistances[1];
    Vector3 halfForward = obb.axes[2] * obb.halfDistances[2] * volumeScale;
    Vector3 halfLeft = halfRight * -1;
    Vector3 halfDown = halfUp * -1;
    Vector3 halfBack = halfForward * -1;

    // Front bottom left
    Vector3 vec = halfBack + halfLeft + halfDown;
    v[0].x = (float)vec.x;
    v[0].y = (float)vec.y;
    v[0].z = (float)vec.z;
    v[0].w = 1.0f;

    // Front bottom right
    vec = halfBack + halfRight + halfDown;
    v[1].x = (float)vec.x;
    v[1].y = (float)vec.y;
    v[1].z = (float)vec.z;
    v[1].w = 1.0f;

    // Front top right
    vec = halfBack + halfRight + halfUp;
    v[2].x = (float)vec.x;
    v[2].y = (float)vec.y;
    v[2].z = (float)vec.z;
    v[2].w = 1.0f;

    // Front top left
    vec = halfBack + halfLeft + halfUp;
    v[3].x = (float)vec.x;
    v[3].y = (float)vec.y;
    v[3].z = (float)vec.z;
    v[3].w = 1.0f;

    // Back bottom left
    vec = halfForward + halfLeft + halfDown;
    v[4].x = (float)vec.x;
    v[4].y = (float)vec.y;
    v[4].z = (float)vec.z;
    v[4].w = 1.0f;

    // Back bottom right
    vec = halfForward + halfRight + halfDown;
    v[5].x = (float)vec.x;
    v[5].y = (float)vec.y;
    v[5].z = (float)vec.z;
    v[5].w = 1.0f;

    // Back top right
    vec = halfForward + halfRight + halfUp;
    v[6].x = (float)vec.x;
    v[6].y = (float)vec.y;
    v[6].z = (float)vec.z;
    v[6].w = 1.0f;

    // Back top left
    vec = halfForward + halfLeft + halfUp;
    v[7].x = (float)vec.x;
    v[7].y = (float)vec.y;
    v[7].z = (float)vec.z;
    v[7].w = 1.0f;

    GLuint i[36] = {
        0, 1, 2, 2, 3, 0, // front
        1, 5, 6, 6, 2, 1, // right side
        2, 6, 7, 7, 3, 2, // top
        0, 7, 4, 0, 3, 7, // left side
        0, 4, 5, 0, 5, 1, // bottom
        4, 7, 6, 4, 6, 5  // back
    };

    et->glGenVertexArrays(1, &vertexArray);

    // Upload both to the GPU
    et->glGenBuffers(1, &vboID);
    et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
    et->glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(BoxVertex),
                 (void *)v, GL_STATIC_DRAW_ARB);

    et->glGenBuffers(1, &idxID);
    et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
    et->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint),
                 (void *)i, GL_STATIC_DRAW);

    et->glBindBuffer(GL_ARRAY_BUFFER, 0);
    et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


