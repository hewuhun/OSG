// Copyright (c) 2004-2008  Sundog Software, LLC All rights reserved worldwide

#include <FeSilverliningLib/VertexBuffer.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Atmosphere.h>

using namespace SilverLining;

VertexBuffer::VertexBuffer(int nVerts)
{
    internalHandle = Renderer::GetInstance()->AllocateVertexBuffer(nVerts);
    numVerts = nVerts;
}

VertexBuffer::~VertexBuffer()
{
    if (internalHandle) {
        Renderer::GetInstance()->ReleaseVertexBuffer(internalHandle);
        internalHandle = 0;
    }
}

bool VertexBuffer::DrawIndexedStrip(const IndexBuffer& ib, bool useVertColors)
{
    return Renderer::GetInstance()->DrawStrip(internalHandle, ib.internalHandle, 0, ib.GetNumIndices(),
            numVerts, useVertColors);
}

bool VertexBuffer::LockBuffer()
{
    return Renderer::GetInstance()->LockVertexBuffer(internalHandle);
}

Vertex *VertexBuffer::GetVertices() const
{
    return Renderer::GetInstance()->GetVertices(internalHandle);
}

bool VertexBuffer::UnlockBuffer()
{
    return Renderer::GetInstance()->UnlockVertexBuffer(internalHandle);
}

bool VertexBuffer::Update(int offset, Vertex *verts, int nVerts)
{
    return Renderer::GetInstance()->UpdateVertexBuffer(internalHandle, offset, verts, nVerts);
}

bool VertexBuffer::Get(int offset, Vertex *verts, int nVerts)
{
    return Renderer::GetInstance()->GetVertexBuffer(internalHandle, offset, verts, nVerts);
}

void Vertex::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
#ifdef FLOATING_POINT_COLOR
    Vertex::r = (float)r / 255.0f;
    Vertex::g = (float)g / 255.0f;
    Vertex::b = (float)b / 255.0f;
    Vertex::a = (float)a / 255.0f;
#else
    switch (Renderer::GetInstance()->GetType()) {
        // OpenGL can ONLY represent RGBA and DirectX can ONLY represent ARGB... argh
    case Atmosphere::DIRECTX9: {
        color = (a << 24) | (r << 16) | (g << 8) | b;
    }
    break;

    default: {
        color = (a << 24) | (b << 16) | (g << 8) | r;
    }
    break;
    }
#endif
}

void Vertex::SetUV(float pu, float pv)
{
    u = pu;
    v = pv;
    t = 0;
    s = 0;
}

void Vertex::SetColor(const Color& c)
{
#ifdef FLOATING_POINT_COLOR
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
#else
    unsigned char r, g, b, a;
    r = (unsigned char)(c.r * 255.0);
    g = (unsigned char)(c.g * 255.0);
    b = (unsigned char)(c.b * 255.0);
    a = (unsigned char)(c.a * 255.0);

    SetColor(r, g, b, a);
#endif
}
