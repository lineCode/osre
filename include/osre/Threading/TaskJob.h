/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015 OSRE ( Open Source Render Engine ) by Kim Kulling

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

#include <osre/Common/Types.h>
#include <cassert>

namespace OSRE {

namespace Common {
    class Event;
    class EventData;
}

namespace Threading {

//-------------------------------------------------------------------------------------------------
///	@class		::ZFXCE2::Threading::TaskJob
///	@ingroup	Infrastructure
///
///	@brief This class implements a simple container for events as ids and the assigned data, which 
///	are implemented as subclasses from the Common::EventData class ( @see Common::EventData ).
//-------------------------------------------------------------------------------------------------
class TaskJob {
public:
    ///	@brief	The class constructor with the event and the event data.
    ///	@param	pEvent		[in] A pointer showing to the event.
    ///	@param	pEventData	[in] A pointer showing to the event data.
    TaskJob( const Common::Event *pEvent, const Common::EventData *pEventData );

    ///	@brief	The class destructor.
    ~TaskJob();

    ///	@brief	Returns a pointer showing to the assigned event.
    ///	@return	The event 
    const Common::Event *getEvent() const;

    ///	@brief	Returns a pointer showing to the assigned event data.
    ///	@return	The event data.
    const Common::EventData *getEventData() const;

    ///	@brief	Set new data.
    ///	@param	pEvent		A pointer showing to the event.
    ///	@param	pEventData	A pointer showing to the event data.
    void set( const Common::Event *pEvent, const Common::EventData *pEventData );

    ///	@brief	Clears the TaskJob-instance.
    void clear();

private:
    TaskJob();
    TaskJob( const TaskJob & );
    TaskJob &operator = ( const TaskJob & );

private:
    const Common::Event *m_pEvent;
    const Common::EventData *m_pEventData;
};

//-------------------------------------------------------------------------------------------------
inline
TaskJob::TaskJob( const Common::Event *pEvent, const Common::EventData *pEventData ) 
: m_pEvent( pEvent )
, m_pEventData( pEventData ) {
    assert( nullptr != pEvent );
}

//-------------------------------------------------------------------------------------------------
inline
TaskJob::~TaskJob() {
    clear();
}

//-------------------------------------------------------------------------------------------------
inline
const Common::Event *TaskJob::getEvent() const {
    return m_pEvent;
}

//-------------------------------------------------------------------------------------------------
inline
const Common::EventData *TaskJob::getEventData() const {
    return m_pEventData;
}

//-------------------------------------------------------------------------------------------------
inline
void TaskJob::set(  const Common::Event *pEvent, const Common::EventData *pEventData ) {
    m_pEvent = pEvent;
    m_pEventData = pEventData;
}

//-------------------------------------------------------------------------------------------------
inline
void TaskJob::clear() {
    m_pEvent = nullptr;
    m_pEventData = nullptr;
}

//-------------------------------------------------------------------------------------------------

} // Namespace Threading
} // Namespace OSRE