/* ZFX Community Engine 2  (ZFXCE2)
---------------------------------------------------------------------------------------------------
Copyright (c) 2011-2015, ZFXCE2 Development Team
All rights reserved.

Redistribution and use of this software in source and binary forms, 
with or without modification, are permitted provided that the 
following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions 
  and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this list of conditions
  and the following disclaimer in the documentation and/or other materials provided with the 
  distribution.

* Neither the name of the ZFXCE2 team, nor the names of its contributors may be used to endorse or 
  promote products derived from this software without specific prior written permission of the 
  ZFXCE2 Development Team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-------------------------------------------------------------------------------------------------*/
#include <osre/IO/Stream.h>

namespace OSRE {
namespace IO {

//-------------------------------------------------------------------------------------------------
Stream::Stream() 
: m_Uri()
, m_AccessMode() {
	// empty
}

//-------------------------------------------------------------------------------------------------
Stream::Stream( const Uri &uri, AccessMode mode ) 
: m_Uri( uri)
, m_AccessMode( mode ) {
	// empty
}

//-------------------------------------------------------------------------------------------------
Stream::~Stream() {
	// empty
}

//-------------------------------------------------------------------------------------------------
void Stream::setUri( const Uri &rURI ) {
	m_Uri = rURI;
}

//-------------------------------------------------------------------------------------------------
const Uri &Stream::getUri() const {
	return m_Uri;
}

//-------------------------------------------------------------------------------------------------
bool Stream::canRead() const {
	return false;
}

//-------------------------------------------------------------------------------------------------
bool Stream::canWrite() const {
	return false;
}

//-------------------------------------------------------------------------------------------------
bool Stream::canSeek() const {
	return false;
}

//-------------------------------------------------------------------------------------------------
bool Stream::canBeMapped() const {
	return false;
}

//-------------------------------------------------------------------------------------------------
void Stream::setAccessMode( AccessMode accessMode ) {
	m_AccessMode = accessMode;
}

//-------------------------------------------------------------------------------------------------
Stream::AccessMode Stream::getAccessMode() const {
	return m_AccessMode;
}

//-------------------------------------------------------------------------------------------------
ui32 Stream::getSize() {
	return 0;
}

//-------------------------------------------------------------------------------------------------
ui32 Stream::read( void *pBuffer, ui32 size ) {
	return 0;
}

//-------------------------------------------------------------------------------------------------
ui32 Stream::write( const void *pBuffer, ui32 size ) {
	return 0;
}

//-------------------------------------------------------------------------------------------------
Stream::Position Stream::seek( Offset offset, Origin origin ) {
	return 0;
}

//-------------------------------------------------------------------------------------------------
Stream::Position Stream::tell() {
	return 0;
}

//-------------------------------------------------------------------------------------------------
bool Stream::isOpen() const {
	return false;
}

//-------------------------------------------------------------------------------------------------
bool Stream::open() {
	return false;
}

//-------------------------------------------------------------------------------------------------
bool Stream::close() {
	return false;
}

//-------------------------------------------------------------------------------------------------

} // Namespace IO
} // Namespace OSRE