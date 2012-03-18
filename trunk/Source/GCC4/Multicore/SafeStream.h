 #pragma once
//========================================================================
// SafeStream.h : Defines a template for thread safe streams
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/gamecode4/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// This cool streaming code was found on....
// http://articles.techrepublic.com.com/5100-10878_11-5072104.html
//
//========================================================================

 #include <ostream>
 #include <sstream>
 #include <memory>

#include "CriticalSection.h"

 // forward declaration(s)
 template< class char_type, class traits_type = std::char_traits< char_type> >    
	class basic_message_handler_log; 

 //    represents the stream buffer for a message_handler_log (see below)
 //    Note: NOT thread-safe

 template< class char_type , class traits_type = std::char_traits< char_type> >    
	class basic_message_handler_log_streambuf 
		: public std::basic_streambuf< char_type, traits_type>
 {
 private:    
	 friend class basic_message_handler_log< char_type, traits_type>;    
	 typedef std::basic_streambuf< char_type, traits_type> streambuf_type;    
	 typedef basic_message_handler_log< char_type, traits_type> ostream_type;    
	 // base class    
	 typedef std::basic_streambuf< char_type, traits_type> base_class; 

 #ifndef __GNUC__    
	 using typename base_class::int_type;    
	 using typename base_class::pos_type;    
	 using typename base_class::off_type;
 #endif     
	 
	 enum    { _BADOFF = -1 /* bad offset - for positioning functions */ };
 protected:    
	 // input, not allowed    
	 virtual int_type pbackfail(int_type = traits_type::eof())    
	 {        
		 // only for output, not for input        
		 GCC_ASSERT( false);        
		 return (traits_type::eof());    
	 }    
	 
	 virtual int showmanyc()    
	 {        
		 // only for output, not for input        
		 GCC_ASSERT( false);        
		 return 0;    
	 }    
	 
	 virtual int_type underflow()    
	 {        
		 // only for output, not for input        
		 GCC_ASSERT( false);        
		 return (traits_type::eof());    
	 }    
	 
	 virtual int_type uflow()    
	 {        
		 // only for output, not for input        
		 GCC_ASSERT( false);        
		 return (traits_type::eof());    
	 }       
	 
	 virtual std::streamsize xsgetn(char_type *, std::streamsize)    
	 {        
		 // only for output, not for input        
		 GCC_ASSERT( false);        
		 return 0;    
	 }       
	 
	 // positioning, not allowed - we're a log    
	 virtual pos_type seekoff(off_type, std::ios_base::seekdir,        
		 std::ios_base::openmode = std::ios_base::in | std::ios_base::out)    
	 {        
		 // we don't allow positioning        
		 GCC_ASSERT( false);        
		 return (std::streampos( _BADOFF));    
	 }    

	 virtual pos_type seekpos(pos_type,        
		 std::ios_base::openmode = std::ios_base::in | std::ios_base::out)    
	 {        
		 // we don't allow positioning        
		 GCC_ASSERT( false);        
		 return (std::streampos( _BADOFF));    
	 }       
	 
	 // output functions 

	 // called to write out from the internal    
	 // buffer, into the external buffer    
	 virtual int sync()    
	 {       
		 m_pOwnerStream->on_new_message( get_stream_buffer().str() );        
		 m_pStreamBuffer = std::auto_ptr< string_stream_type>( new string_stream_type);        
		 return 0;    
	 }    
	 
	 virtual streambuf_type *setbuf( char_type * buffer, std::streamsize n)    
	 {        
		 // ... note: this function MUST be called        
		 //    before working with this stream buffer        
		 // we don't use a buffer - we forward everything        
		 GCC_ASSERT( buffer == NULL && n == 0);        
		 this->setp( NULL, NULL);        
		 return this;    
	 }    
	 
	 // write the characters from the buffer    
	 // to their real destination    
	 virtual int_type overflow(int_type nChar = traits_type::eof())    
	 {        
		 if ( traits_type::not_eof( nChar))            
			 get_stream_buffer() << ( char_type)nChar;        
		 return traits_type::not_eof( nChar);    
	 }       
	 
	 virtual std::streamsize xsputn(const char_type *S, std::streamsize N)    
	 {        
		 get_stream_buffer().write( S, N);        
		 return N;    
	 } 

 public:    
	 basic_message_handler_log_streambuf()        
		 : m_pStreamBuffer( new string_stream_type) {}

 private:    
	 typedef std::basic_ostringstream< char_type> string_stream_type;    
	 string_stream_type & get_stream_buffer()    
		{ return *m_pStreamBuffer; }

 private:    
	 // holds the Message, until it's flushed    
	 std::auto_ptr< string_stream_type> m_pStreamBuffer;    
	 // the Message Handler Log - where we write into    
	 ostream_type * m_pOwnerStream;
 }; 

 // derive your class from this and implement the PROTECTED on_new_message function
 template< class char_type, class traits_type >    
	class basic_message_handler_log        
		: public std::basic_ostream< char_type, traits_type>
 {    
	 typedef basic_message_handler_log_streambuf< char_type, traits_type> handler_streambuf_type;    
	 friend class basic_message_handler_log_streambuf< char_type, traits_type>;    
	 typedef std::basic_ostream< char_type, traits_type> base_class;    
	 typedef std::basic_ostringstream< char_type> string_stream_type; 

 protected:    
	 typedef std::basic_string< char_type> string_type;     
	 basic_message_handler_log()       
		 : m_StreamBuf(),          
		 base_class( NULL)    
	 {        
		 m_StreamBuf.m_pOwnerStream = this;        
		 this->init( &m_StreamBuf);        
		 m_StreamBuf.pubsetbuf( NULL, 0);    
	 }
	 
	 basic_message_handler_log( const basic_message_handler_log< char_type, traits_type> & from)        
		 : m_StreamBuf(),          
		 base_class( NULL)    
	 {        
		 m_StreamBuf.m_pOwnerStream = this;        
		 this->init( &m_StreamBuf);        
		 m_StreamBuf.pubsetbuf( NULL, 0);    
	 }     
	 
	 virtual ~basic_message_handler_log() {}

 protected:    
	 virtual void on_new_message( const string_type & new_message_log) = 0; 
 
 public:    
	 // our stream buffer    
	 handler_streambuf_type m_StreamBuf;
 }; 

 typedef basic_message_handler_log< char> message_handler_log;
 typedef basic_message_handler_log< wchar_t> wmessage_handler_log; 



// the object to be started - on a given thread
struct win32_thread_obj
{
    virtual ~win32_thread_obj() {}
    virtual void operator()() = 0;
};
 
 
struct win32_thread_manager
{
    typedef win32_thread_obj thread_obj_base;
    static void sleep( int nMillisecs) { Sleep( nMillisecs); }
    static void create_thread( win32_thread_obj & obj)
    {
        DWORD dwThreadID;
        CreateThread( 0, 0,
            win32_thread_manager::ThreadProc, &obj, 0, &dwThreadID);
    }
 
 
private:
    static DWORD WINAPI ThreadProc( LPVOID lpData)
    {
        win32_thread_obj * pThread = ( win32_thread_obj *)lpData;
        ( *pThread)();
        return 0;
    }
};
 

// allows thread-safe writing
template< class char_type, class traits_type = std::char_traits< char_type> >
    class thread_safe_stream_writer
{
    typedef thread_safe_stream_writer< char_type, traits_type> this_class;
    typedef std::basic_ostream< char_type, traits_type> ostream_type;
    typedef std::basic_string< char_type, traits_type> string_type;
 
 
    // forward declaration
    struct thread_info;
    friend struct thread_info;
 
 
    // thread-related definitions
    typedef win32_thread_manager thread_manager;
    typedef typename thread_manager::thread_obj_base thread_obj_base;

    // so that from our thread we know the object we're manipulating
    struct thread_info : public thread_obj_base
    {
        thread_info()
            : m_bHasFinished( false),
              m_pThis( NULL)
        {}
 
 
        /* virtual */ void operator()()
        {
            while ( true)
            {
                std::string * pstr = NULL;
                {
                    ScopedCriticalSection locker( m_pThis->m_cs);
                    // get the string
                    if ( m_pThis->m_astrMessages.size() > 0)
                    {
                        pstr = m_pThis->m_astrMessages.front();
                        m_pThis->m_astrMessages.pop();
                    }
                    // ... only when there are no more messages,
                    //    will we ask if we should be destructed
                    else if ( m_pThis->m_bShouldBeDestructed)
                    {
                        // signal to the other thread we've finished
                        m_bHasFinished = true;
                        return;
                    }
                }
                // write the string
                if ( pstr)
                {
                    m_pThis->m_underlyingLog << *pstr;
                    delete pstr;
                }
                else
                    // nothing to write - wait
                    thread_manager::sleep( 1);              
            }
        }
 
 
        this_class * m_pThis;
        volatile bool m_bHasFinished;
    };
 
 
public:
    void add_message( const string_type & str)
    {
        ScopedCriticalSection locker( m_cs);
        m_astrMessages.push( new string_type( str));
    }
 
 
    thread_safe_stream_writer( ostream_type & underlyingLog)
        : m_underlyingLog( underlyingLog),
          m_bShouldBeDestructed( false)
    {
        m_info.m_pThis = this;
        thread_manager::create_thread( m_info);
    }
 
 
    ~thread_safe_stream_writer()
    {
        // signal to the other thread we're about to be
        // destructed
        {
            ScopedCriticalSection locker( m_cs);
            m_bShouldBeDestructed = true;
        }
        // wait while the other thread writes all messages
        while ( true)
        {
            ScopedCriticalSection locker( m_cs);
            if ( m_info.m_bHasFinished)
                // the other thread has finished
                break;
        }
    }
 
 
    CriticalSection & cs() const { return m_cs; }
private:
    // the critical section used for thread-safe locking
    mutable CriticalSection m_cs;
 
 
    // needed to create the other thread
    thread_info m_info;
    volatile bool m_bShouldBeDestructed;
 
 
    ostream_type & m_underlyingLog;
    std::queue< string_type*> m_astrMessages;
};
 
// forward declaration
template< class char_type, class traits_type = std::char_traits< char_type> >
    class basic_thread_safe_stream;
 
  
template< class char_type, class traits_type = std::char_traits< char_type> >
    class basic_internal_thread_safe_stream
{
    typedef std::basic_ostream< char_type, traits_type> ostream_type;
    friend class basic_thread_safe_stream< char_type, traits_type>;
 
     // non-copyiable
    typedef basic_internal_thread_safe_stream< char_type, traits_type> this_class;
    basic_internal_thread_safe_stream( const this_class &);
    this_class & operator=( this_class &);
  
public:
    basic_internal_thread_safe_stream( ostream_type & underlyingLog)
        : m_underlyingLog( underlyingLog),
          m_writer( underlyingLog)
    {}
 
 
    ~basic_internal_thread_safe_stream()
    {}
 
 
    void write_message( const std::basic_string< char_type, traits_type> & str)
    { m_writer.add_message( str); }
 
 
    void copy_state_to( ostream_type & dest) const
    {
        ScopedCriticalSection locker( m_writer.cs());
       dest.copyfmt( m_underlyingLog);
        dest.setstate( m_underlyingLog.rdstate());
    }
 
 
    void copy_state_from( const ostream_type & src)
    {
        ScopedCriticalSection locker( m_writer.cs());
        m_underlyingLog.copyfmt( src);
        m_underlyingLog.setstate( m_underlyingLog.rdstate());
    }
 
 
private:
    ostream_type & m_underlyingLog;
    thread_safe_stream_writer< char_type, traits_type> m_writer;
};
 
 
typedef basic_internal_thread_safe_stream< char> internal_thread_safe_stream;
typedef basic_internal_thread_safe_stream< wchar_t> winternal_thread_safe_stream;
 

template< class char_type, class traits_type>
    class basic_thread_safe_stream
    // *** protected, not public !!!
    : protected basic_message_handler_log< char_type, traits_type>
{
    typedef std::basic_ostream< char_type, traits_type> ostream_type;
    typedef basic_internal_thread_safe_stream< char_type, traits_type> internal_type;
public:
    basic_thread_safe_stream( internal_type & tsLog)
        : m_tsLog( tsLog)
    {
        // get underlying stream state
        tsLog.copy_state_to( ts() );
    }
  
    basic_thread_safe_stream( const basic_thread_safe_stream< char_type, traits_type> & from)
        : m_tsLog( from.m_tsLog),
          // ... on some platforms, a std::ostream base copy-constructor
          //    might be defined as private...
          basic_message_handler_log< char_type, traits_type>()
    {
        // get underlying stream state
        m_tsLog.copy_state_to( ts() );
    }
 
 
    ~basic_thread_safe_stream()
    {
        // copy state to underlying stream
        m_tsLog.copy_state_from( ts() );
    }
 
 
    // get base class - to which we can write
    std::basic_ostream< char_type, traits_type> & ts()
    { return *this; }
 
 
protected:
    virtual void on_new_message( const string_type & str)
    { m_tsLog.write_message( str); }
 
 
private:
    internal_type  & m_tsLog;
};
  
 
typedef basic_thread_safe_stream< char> thread_safe_stream;
typedef basic_thread_safe_stream< wchar_t> wthread_safe_stream;
 
 

 
 
//////////////////////////////////////////////////////////
// Test
 
 
//#include <iostream>
//#include <fstream>
//#include <iomanip>
 
 
//  This cool streaming code was found on....
//  http://articles.techrepublic.com.com/5100-10878_11-5072104.html

// Example on using the thread safe stream.

thread_safe_stream get_log()
{
    static std::ofstream out( "out.txt");
    static internal_thread_safe_stream log( out);
    return thread_safe_stream( log);
}

DWORD WINAPI WriteToLog( LPVOID lpData)
{
	get_log();

	//GCC_ASSERT(g_Out && "No stream!");
	//if (!g_Out)
	//	return FALSE;

    int *pnThreadID = ( int *)lpData;
    // wait for all threads to be created, so that
    // we write at about the same time (stress it ;-))
    Sleep( 500);
 
 
    for ( int idx = 0; idx < WRITES_PER_THREAD; idx++)
    {
        get_log().ts() << "writing double: " << 5.23 << std::endl;
        get_log().ts() << "message " << idx << " from thread " << *pnThreadID << std::endl;
        // ... get other threads a chance to write
        Sleep( 1);
 
 
        if ( ( idx == 10) && ( *pnThreadID == 10))
        {
            // from now on, '5.23' will be written as '5,23'
            // (german locale)
            std::locale loc = std::locale( "german");
            get_log().ts().imbue( loc);
        }
  
    }
  
    InterlockedDecrement( &nRemainingThreads);
	get_log().ts().flush();
    delete pnThreadID;
    return 0;
}
 
 
void testStreaming(CProcessManager *procMgr)
{
	get_log();	// initialize....
    for ( int idx = 0; idx < THREADS_COUNT; ++idx)
    {
        DWORD dwThreadID;
        CreateThread( 0, 0, WriteToLog, new int( idx), 0, &dwThreadID);
    }
 
 
    // wait for all threads to end
    while ( true)
    {
        InterlockedIncrement( &nRemainingThreads);
        if ( InterlockedDecrement( &nRemainingThreads) == 0)
            break;
        Sleep( 100);
    }
}

