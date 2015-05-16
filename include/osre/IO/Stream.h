#pragma once

#include <osre/Common/Types.h>
#include <osre/IO/Uri.h>

namespace OSRE {
namespace IO {

class Uri;

//-------------------------------------------------------------------------------------------------
///	@class		::ZFXCE2::IO::Stream
///	@ingroup	Infrastructure
///
///	@brief  This class declares the interface for a stream. 
///
/// Streams can be used to do any kind of input-output operations like reading stuff from the hard 
/// disk or writing stuff into an archive.
//-------------------------------------------------------------------------------------------------
class Stream {
public:
    typedef ui32 Position;      ///< The current position.
    typedef ui32 Offset;        ///< The offset from the current position.
    
    /// @brief  Enumerates the type of access.
    enum AccessMode	{
        ReadAccess,             ///< Read-only access.
        WriteAccess,            ///< Write-access.
        ReadAccessBinary,       ///< Read-access in binary mode.
        WriteAccessBinary,      ///< Write-access in binary mode.
        ReadWriteAccess,        ///< Read/Write-access.
        AppendAccess            ///< Append-access, stuff will be attached and the file end.
    };

    /// @brief  Enumerates the requested file position
    enum Origin	{
        Begin,      ///< Begin of the file.
        Current,    ///< Current position
        End         ///< End of the file.
    };

public:
    ///	@brief	The class default constructor.
    Stream();

    ///	@brief	The class constructor with the uri and the access type.
    ///	@param	uri				[in] Describes the location of the object to access.
    ///	@param	mode        	[in] The requested access mode.
    Stream( const Uri &uri, AccessMode mode );

    ///	@brief	The class destructor, virtual.
    virtual ~Stream();

    ///	@brief	Set the uri manually.
    ///	@param	uri		[in] Describes the location of the object to access.
    virtual void setUri( const Uri &uRI);

    ///	@brief	Returns the assigned uri.
    ///	@return	A reference showing to the uri.
    virtual const Uri &getUri() const;

    /// @brief  Returns true, if stream can used to read.
    /// @return true, if read operation is supported.
    virtual bool canRead() const;

    /// @brief  Returns true, if stream can used to write.
    /// @return true, if write operation is supported.
    virtual bool canWrite() const;
    
    /// @brief  Returns true, if stream can used to seek the position.
    /// @return true, if seek operation is supported.
    virtual bool canSeek() const;
    
    /// @brief  Returns true, if stream can used to map to a memory location.
    /// @return true, if map operation is supported.
    virtual bool canBeMapped() const;

    /// @brief  Set the current request mode.
    /// @param  accessMode      [in] The new access mode.
    virtual void setAccessMode( AccessMode accessMode );

    /// @brief  Returns the current access mode.
    /// @return The current access mode.
    virtual AccessMode getAccessMode() const;
    
    /// @brief  Opens the stream.
    /// @return true, if successful.
    virtual bool open();

    /// @brief  Closes the stream.
    /// @return true, if successful.
    virtual bool close();
    
    /// @brief  Returns the file size.
    /// @return The file size.
    virtual ui32 getSize();
    
    /// @brief  Reads a given number of bytes from the buffer.
    /// @param  pBuffer         [in] The buffer to read in.
    /// @param  size            [in] The number of bytes to read.
    /// @return The number of read bytes.
    virtual ui32 read( void *pBuffer, ui32 size );
    
    /// @brief  Writes a given number of bytes into the buffer.
    /// @param  pBuffer         [in] The buffer to write.
    /// @param  size            [in] The number of bytes to write.
    /// @return The number of written bytes.
    virtual ui32 write( const void *pBuffer, ui32 size );
    
    /// @brief  Moves to the current position.
    /// @param  offset          [in] The offset.
    /// @param  origin          [in] The origin.
    /// @return The position.
    virtual Position seek( Offset offset, Origin origin );
    
    /// @brief  Returns the current position.
    /// @return The current position.
    virtual Position tell();
    
    ///	@brief	Returns true, if the stream is open.
    ///	@return true, if file is open.
    virtual bool isOpen() const;

public:
    Uri m_Uri;
    AccessMode m_AccessMode;
};

//-------------------------------------------------------------------------------------------------

} // Namespace IO
} // Namespace OSRE