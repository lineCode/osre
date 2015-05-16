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

#include <osre/Threading/AbstractTask.h>
#include <osre/Threading/TAsyncQueue.h>

namespace OSRE {

namespace Common {
    class Event;
}

namespace Threading {

class TaskManager;
class SystemTaskThread;
class TaskJob;

//-------------------------------------------------------------------------------------------------
///	@class		::ZFXCE2::Threading::Task
///	@ingroup	Infrastructure
///
///	@brief	This class implements a system task. 
///
/// You can use a system task for a running service like an IO-task waiting in the background for any 
/// loader jobs. The system-task will run all the time after starting it and awaits jobs.
//-------------------------------------------------------------------------------------------------
class SystemTask : public AbstractTask {
    friend class TaskManager;

public:
    ///	@brief	Overwritten, @see AbstractTask for more info's.
    virtual void setWorkingMode( WorkingMode mode );

    ///	@brief	Overwritten, @see AbstractTask.
    virtual WorkingMode getWorkingMode() const;

    virtual void setBufferMode( BufferMode buffermode );
    virtual BufferMode getBufferMode() const;

    ///	@brief	Overwritten, @see AbstractTask.
    virtual bool start( Platform::AbstractThread *pThread );

    ///	@brief	Overwritten, @see AbstractTask.
    virtual bool stop();

    ///	@brief	Overwritten, @see AbstractTask.
    virtual bool isRunning() const;

    ///	@brief	Overwritten, @see AbstractTask.
    virtual bool execute();

    ///	@brief	Overwritten, @see AbstractTask.
    virtual void setThreadInstance( Platform::AbstractThread *pThreadInstance );
    
    ///	@brief	Overwritten, @see AbstractTask.
    virtual void onUpdate();
    
    ///	@brief	Overwritten, @see AbstractTask.
    virtual void await();

    ///	@brief	An event handler will be attached.
    ///	@param	pEventHandler	[in] A pointer showing to the event handler to attach.
    virtual void attachEventHandler( Common::AbstractEventHandler *pEventHandler );
    
    ///	@brief	An event handler will be detached.
    virtual void detachEventHandler();
    
    ///	@brief	A new task job will be enqueued.
    ///	@param	pEvent		[in] A pointer showing to the event, which describes the kind of job.
    ///	@param	pEventData	[in] A pointer showing to the event data.
    ///	@return	true, if the enqueue operation was successful, false if not.
    virtual bool sendEvent( const Common::Event *pEvent, const Common::EventData *pEventData );
    
    ///	@brief	Returns the number of enqueued jobs.
    ///	@return	The number of attached jobs.
    virtual ui32 getEvetQueueSize() const;
    
    ///	@brief	The factory method, creates a new instance of the system task.
    static SystemTask *create( const String &rTaskName );

protected:
    ///	@brief	The class constructor.
    ///	@param	taskName	[in] The class instance name.
    SystemTask( const String &taskName );

    ///	@brief	The class destructor, virtual.
    virtual ~SystemTask();
    
private:
    WorkingMode m_WorkingMode;
    BufferMode m_buffermode;
    SystemTaskThread *m_pTaskThread;
    typedef Threading::TAsyncQueue<const TaskJob*> TaskQueue;
    TaskQueue *m_pAsyncQueue;
};

//-------------------------------------------------------------------------------------------------

} // Namespace Threading
} // Namespace OSRE