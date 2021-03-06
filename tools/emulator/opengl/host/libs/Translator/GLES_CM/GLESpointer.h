/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef GLES_POINTER_H
#define GLES_POINTER_H

#include <GLES/gl.h>
#include "GLESbuffer.h"

class GLESpointer
{

public:
    GLESpointer();
    GLenum        getType() const;
    GLint         getSize() const;
    GLsizei       getStride() const;
    const GLvoid* getArrayData() const;
    GLvoid*       getBufferData() const;
    unsigned int  getBufferOffset() const;
    void          getBufferConversions(const RangeList& rl,RangeList& rlOut);
    bool          bufferNeedConversion(){ return !m_buffer->fullyConverted();}
    void          setArray (GLint size,GLenum type,GLsizei stride,const GLvoid* data);
    void          setBuffer(GLint size,GLenum type,GLsizei stride,GLESbuffer* buf,int offset);
    bool          isEnable() const;
    void          enable(bool b);

private:
    GLint         m_size;
    GLenum        m_type;
    GLsizei       m_stride;
    bool          m_enabled;
    const GLvoid* m_data;
    GLESbuffer*   m_buffer;
    unsigned int  m_buffOffset;
};
#endif
