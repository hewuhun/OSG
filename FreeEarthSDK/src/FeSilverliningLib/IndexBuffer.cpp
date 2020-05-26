// Copyright (c) 2004-2008  Sundog Software, LLC All rights reserved worldwide

#include <FeSilverliningLib/IndexBuffer.h>
#include <FeSilverliningLib/Renderer.h>

using namespace SilverLining;

IndexBuffer::IndexBuffer(int nIndices)
{
    internalHandle = Renderer::GetInstance()->AllocateIndexBuffer(nIndices);
    numIndices = nIndices;
}

IndexBuffer::~IndexBuffer()
{
    if (internalHandle) {
        Renderer::GetInstance()->ReleaseIndexBuffer(internalHandle);
    }
}

bool IndexBuffer::LockBuffer()
{
    return Renderer::GetInstance()->LockIndexBuffer(internalHandle);
}

Index *IndexBuffer::GetIndices() const
{
    return Renderer::GetInstance()->GetIndices(internalHandle);
}

bool IndexBuffer::UnlockBuffer()
{
    return Renderer::GetInstance()->UnlockIndexBuffer(internalHandle);
}
