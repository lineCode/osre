/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2018 OSRE ( Open Source Render Engine ) by Kim Kulling

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-----------------------------------------------------------------------------------------------*/
#pragma once

namespace OSRE {
namespace RenderBackend {

//-------------------------------------------------------------------------------------------------
///	@ingroup	Engine
///
///	@brief  This class describes the supported blending states which are currently active in the 
/// render-pass.
//-------------------------------------------------------------------------------------------------
class BlendState {
public:
    /// 
    enum class BlendFunc {
        FuncNone,
        FuncAdd,
        FuncSubstract,
        ReverseSubstract,
        Min,
        Max,
        Off
    };

public:
    /// @brief  The default constructor.
    BlendState() noexcept;

    /// @brief  The constructor with the func.
    /// @param  func    [in] The new blending function to apply.
    explicit BlendState(BlendFunc func);

    /// @brief  The class destructor.
    ~BlendState();

    /// @brief  Will set a new blending function.
    /// @param  func    [in] The new blending function to apply.
    void setBlendFunc( BlendFunc func );

    /// @brief  Returns the blending function.
    /// return  The blending function.
    BlendFunc getBlendFunc() const;

    /// The compare operators.
    bool operator == ( const BlendState &rhs ) const;
    bool operator != ( const BlendState &rhs ) const;

private:
    BlendFunc m_blendFunc;
};

inline
BlendState::BlendState() noexcept
: m_blendFunc( BlendFunc::FuncNone ) {
    // empty
}

inline 
BlendState::BlendState(BlendFunc func)
: m_blendFunc( func ) {
    // empty
}

inline
BlendState::~BlendState() {
    // empty
}

inline
void BlendState::setBlendFunc( BlendFunc func ) {
    if ( func != m_blendFunc ) {
        m_blendFunc = func;
    }
}

inline
BlendState::BlendFunc BlendState::getBlendFunc() const {
    return m_blendFunc;
}

inline
bool BlendState::operator == ( const BlendState &rhs ) const {
    return m_blendFunc == rhs.m_blendFunc;
}

inline
bool BlendState::operator != ( const BlendState &rhs ) const {
    return !( *this == rhs );
}

} // Namespace RenderBackend
} // Namespace OSRE
