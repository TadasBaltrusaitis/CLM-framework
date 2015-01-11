/** \file
  * IStream adapters.
  */
/*
 * Copyright (C) 2013 Alexander Lamaison <alexander.lamaison@gmail.com>
 *
 * This material is provided "as is", with absolutely no warranty
 * expressed or implied. Any use is at your own risk. Permission to
 * use or copy this software for any purpose is hereby granted without
 * fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is
 * granted, provided the above notices are retained, and a notice that
 * the code was modified is included with the above copyright notice.
 *
 * This header is part of Comet version 2.
 * https://github.com/alamaison/comet
 */

#ifndef COMET_STREAM_H
#define COMET_STREAM_H

#include <comet/config.h>

#include <comet/bstr.h> // bstr_t
#include <comet/error.h> // com_error
#include <comet/handle_except.h> // COMET_CATCH_CLASS_INTERFACE_BOUNDARY
#include <comet/server.h> // simple_object

#include <cassert> // assert
#include <istream>
#include <limits> // numeric_limits
#include <ostream>

#include <strsafe.h> // StringCbCopyW

namespace comet {

namespace impl {

    static const size_t COPY_CHUNK_SIZE = 512;

    /**
     * Used to clear stream failure on exiting a scope.
     *
     * Useful when trying an operation and converting any failure to an
     * exception.  In that case there is no need for the stream object to
     * also retain evidence of the failure.
     *
     * Note, only clears `failbit`.  `eofbit` is left alone as it is a
     * valid consequence of operations performed by the wrapper.  `badbit` is
     * also left as these errors are unrecoverable and therefore the
     * stream is as good as dead; a caller of the unwrapped stream needs
     * to know that.
     */
    template<typename Stream>
    class stream_failure_cleanser
    {
    public:
        explicit stream_failure_cleanser(Stream& stream)
            : m_stream(stream)
        {}

        ~stream_failure_cleanser() throw()
        {
            try
            {
                m_stream.clear(m_stream.rdstate() & ~std::ios_base::failbit);
            }
            catch (const std::exception&)
            {}
        }

    private:
        stream_failure_cleanser(const stream_failure_cleanser&);
        stream_failure_cleanser& operator=(const stream_failure_cleanser&);

        Stream& m_stream;
    };

    // These do_read/write helper functions dispatch the stream-type specific code
    // so that we only need one class to implement all the stream adapters

    inline void do_istream_read(
        std::istream& stream, void* buffer, ULONG buffer_size_in_bytes,
        ULONG& bytes_read_out)
    {
        typedef std::istream stream_type;

        //
        // IMPORTANT: The bytes_read_out count must be  correct even in the
        // error case, EVEN if that error is an exception, because the caller
        // can treat an error as EOF (see Read method docs).
        //
        // However, in the error case it is acceptable to make the read count
        // smaller than the actual number of bytes read (e.g. 0) because the
        // caller is losing the unread part of the stream anyway.  Losing a few
        // extra bytes that are already in the buffer but not declared valid
        // by the byte count will not make the situation worse.

        bytes_read_out = 0U;

        stream.read(
            reinterpret_cast<char*>(buffer),
            buffer_size_in_bytes / sizeof(std::istream::char_type));

        // Any failure not caused by eof is a failure.
        // Badbit is always a failure because, even if we are at eof, the stream
        // encountered a problem that needs reporting.
        if (stream.bad() || (stream.fail() && !stream.eof()))
        {
            throw std::runtime_error("Reading from stream failed");
        }
        else
        {
            if (stream.gcount() >= 0 &&
                // temp conversion to unsigned larger than ULONG to catch
                // values larger than ULONG
                static_cast<ULONGLONG>(stream.gcount()) <
                    (std::numeric_limits<ULONG>::max)() /
                    sizeof(std::istream::char_type))
            {
                bytes_read_out = static_cast<ULONG>(
                    stream.gcount() * sizeof(std::istream::char_type));
            }

            assert(stream.eof() || bytes_read_out == buffer_size_in_bytes);
        }
    }

    inline void do_ostream_write(
        std::ostream& stream, const void* buffer, ULONG buffer_size_in_bytes,
        ULONG& bytes_written_out)
    {
        // If there is an error we cannot get a reliable write count, even
        // if we were to use stream.rdbuf()->sputn, because the failure
        // might well happen during the flush operation which doesn't let
        // us find out how much was flushed before failing.

        typedef std::ostream stream_type;

        bytes_written_out = 0U;

        stream.write(
            reinterpret_cast<const char*>(buffer),
            buffer_size_in_bytes / sizeof(std::ostream::char_type));

        try
        {
            stream.flush();
        }
        catch (const std::exception& e)
        {
            throw com_error(e.what(), STG_E_MEDIUMFULL);
        }

        if (stream.good())
        {
            bytes_written_out = buffer_size_in_bytes;
        }
        else
        {
            throw com_error("Writing to stream failed", STG_E_MEDIUMFULL);
        }
    }

    /** Ensure read position updated even in case of exception */
    template<typename Stream>
    class read_position_finaliser
    {
    public:
        read_position_finaliser(
            Stream& stream, std::streampos& new_position_out)
            : m_stream(stream), m_new_position_out(new_position_out)
        {}

        ~read_position_finaliser() throw()
        {
            try
            {
                // we still want the read position if previous op failed
                m_stream.clear();

                std::streampos new_position = m_stream.tellg();
                if (m_stream)
                {
                    m_new_position_out = new_position;
                }
                else
                {
                    m_new_position_out = std::streampos();
                }
            }
            catch (...)
            {
                // We tried.  Nothing else we can do
                //
                // We likely ended up here in the exception unwinding of a
                // failed seek because a non-seekable stream chose to let us
                // know by throwing an exception from the streambuf
                // (e.g. Boost.IOStream).  Non-seekable is also non-tellable so
                // telgp does the same thing.
                //
                // We have prevent this exception propagating else we get an
                // thrown while unwinding another exception causing terminate()
                m_new_position_out = std::streampos(-1);
            }
        }

    private:

        read_position_finaliser(const read_position_finaliser&);
        read_position_finaliser& operator=(const read_position_finaliser&);

        Stream& m_stream;
        std::streampos& m_new_position_out;
    };

    /** Ensure write position updated even in case of exception */
    template<typename Stream>
    class write_position_finaliser
    {
    public:
        write_position_finaliser(
            Stream& stream, std::streampos& new_position_out)
            : m_stream(stream), m_new_position_out(new_position_out)
        {}

        ~write_position_finaliser() throw()
        {
            try
            {
                // we still want the write position if previous op failed
                m_stream.clear();

                std::streampos new_position = m_stream.tellp();
                if (m_stream)
                {
                    m_new_position_out = new_position;
                }
                else
                {
                    m_new_position_out = std::streampos();
                }
            }
            catch (...)
            {
                // We tried.  Nothing else we can do
                //
                // We likely ended up here in the exception unwinding of a
                // failed seek because a non-seekable stream chose to let us
                // know by throwing an exception from the streambuf
                // (e.g. Boost.IOStream).  Non-seekable is also non-tellable so
                // tellp does the same thing.
                //
                // We have prevent this exception propagating else we get an
                // thrown while unwinding another exception causing terminate()
                m_new_position_out = std::streampos(-1);
            }
        }

    private:
        write_position_finaliser(const write_position_finaliser&);
        write_position_finaliser& operator=(const write_position_finaliser&);

        Stream& m_stream;
        std::streampos& m_new_position_out;
    };

    /**
     * Copies `streampos` out-parameter to ULARGE_INTEGER out parameter
     * in the face of exceptions.
     *
     * Using this class means you can concentrate on keeping the `streampos`
     * variable updated, confident in the knowledge the the COM-interface
     * ULARGE_INTEGER parameter will eventually be updated to match it,
     * no matter what.
     *
     * Assumes that the source out parameter is guaranteed to be valid at
     * all times.  Valid doesn't necessarily mean correct but that it always
     * contains the value we want to set the out parameter to, even if that is
     * wrong (for instance failure while calculating correct position).
     */
    class position_out_converter
    {
    public:

        position_out_converter(
            std::streampos& source_out_parameter,
            ULARGE_INTEGER* destination_out_parameter)
            :
        m_source(source_out_parameter),
        m_destination(destination_out_parameter) {}

        ~position_out_converter()
        {
            if (m_destination)
            {
                try
                {
                    // Convert to streamoff because streampos may not
                    // be an integer.  streamoff is guaranteed to be.
                    std::streamoff offset_from_beginning = m_source;
                    if (offset_from_beginning < 0)
                    {
                        // Invalid offset, for example seeking not supported
                        // The error itself is dealt with right after seeking
                        // so this class just has to convert it to something
                        // reasonable for an unsigned value
                        m_destination->QuadPart = 0U;
                    }
                    else
                    {
                        m_destination->QuadPart = offset_from_beginning;
                    }
                }
                catch(const std::exception&)
                {
                    // Only way this can happen is if streampos refuses to
                    // convert to streamoff in which case we really are screwed.
                    m_destination->QuadPart = 0U;
                }
            }
        }

    private:
        position_out_converter(const position_out_converter&);
        position_out_converter& operator=(const position_out_converter&);

        std::streampos& m_source;
        ULARGE_INTEGER* m_destination;
    };

    /**
     * Increments a total counter with an increment are end of scope,
     * regardless of how that scope is ended.
     *
     * Assumes the increment parameter is valid at all times.
     */
    class byte_count_incrementer
    {
    public:
        byte_count_incrementer(
            ULARGE_INTEGER* total, ULONG& increment)
            : m_total(total), m_increment(increment) {}

        ~byte_count_incrementer()
        {
            if (m_total)
            {
                m_total->QuadPart += m_increment;
            }
        }

    private:

        byte_count_incrementer(const byte_count_incrementer&);
        byte_count_incrementer& operator=(const byte_count_incrementer&);

        ULARGE_INTEGER* m_total;
        ULONG& m_increment;
    };

    template<typename StreamTraits>
    class position_resetter
    {
    public:
        position_resetter(StreamTraits& traits, std::streampos position)
            :
        m_traits(traits), m_position(position) {}

        ~position_resetter()
        {
            try
            {
                std::streampos new_position;
                m_traits.do_seek(
                    m_position, std::ios_base::beg, new_position);

                assert(new_position == m_position);
            }
            catch (const std::exception&)
            {}
        }

    private:
        position_resetter(const position_resetter&);
        position_resetter& operator=(const position_resetter&);

        StreamTraits& m_traits;
        std::streampos m_position;
    };

    /**
     * Calculate offset of end of stream from start.
     */
    template<typename StreamTraits>
    inline std::streamoff stream_size(StreamTraits& traits)
    {
        position_resetter<StreamTraits>(traits, traits.do_tell());

        std::streampos new_position;
        traits.do_seek(0, std::ios_base::end, new_position);

        return new_position;
    }

    template<typename Stream, bool is_istream, bool is_ostream>
    class stream_traits;

    template<typename Stream>
    class stream_traits<Stream, true, false>
    {
    public:
        explicit stream_traits(Stream& stream) : m_stream(stream) {}

        void do_read(
            void* buffer, ULONG buffer_size_in_bytes,
            ULONG& bytes_read_out)
        {
            do_istream_read(
                m_stream, buffer, buffer_size_in_bytes, bytes_read_out);
        }

        void do_write(
            const void* /*buffer*/,
            ULONG /*buffer_size_in_bytes*/, ULONG& bytes_written_out)
        {
            bytes_written_out = 0U;
            throw com_error(
                "std::istream does not support writing", STG_E_ACCESSDENIED);
        }

        void do_seek(
            std::streamoff offset, std::ios_base::seekdir way,
            std::streampos& new_position_out)
        {
            read_position_finaliser<Stream> position_out_updater(
                m_stream, new_position_out);

            if (!m_stream.seekg(offset, way))
            {
                throw std::runtime_error("Unable to change read position");
            }
        }

        std::streampos do_tell()
        {
            std::streampos pos = m_stream.tellg();
            if (!m_stream)
            {
                throw std::runtime_error("Stream position unavailable");
            }

            return pos;
        }

        void do_flush()
        {
            throw com_error(
                "std::istream does not support committing data",
                STG_E_ACCESSDENIED);
        }

    private:
        stream_traits(const stream_traits&);
        stream_traits& operator=(const stream_traits&);

        Stream& m_stream;
    };

    template<typename Stream>
    class stream_traits<Stream, false, true>
    {
    public:
        explicit stream_traits(Stream& stream) : m_stream(stream) {}

        void do_read(
            void* /*buffer*/,
            ULONG /*buffer_size_in_bytes*/, ULONG& bytes_read_out)
        {
            bytes_read_out = 0U;
            throw com_error(
                "std::ostream does not support reading", STG_E_ACCESSDENIED);
        }

        void do_write(
            const void* buffer,
            ULONG buffer_size_in_bytes, ULONG& bytes_written_out)
        {
            do_ostream_write(
                m_stream, buffer, buffer_size_in_bytes, bytes_written_out);
        }

        void do_seek(
            std::streamoff offset, std::ios_base::seekdir way,
            std::streampos& new_position_out)
        {
            write_position_finaliser<Stream> position_out_updater(
                m_stream, new_position_out);

            if (!m_stream.seekp(offset, way))
            {
                throw std::runtime_error("Unable to change write position");
            }
        }

        std::streampos do_tell()
        {
            std::streampos pos = m_stream.tellp();
            if (!m_stream)
            {
                throw std::runtime_error("Stream position unavailable");
            }

            return pos;
        }

        void do_flush()
        {
            if (!m_stream.flush())
            {
                throw std::runtime_error(
                    "Unable to flush buffer to output sequence");
            }
        }

    private:
        stream_traits(const stream_traits&);
        stream_traits& operator=(const stream_traits&);

        Stream& m_stream;
    };

    template<typename Stream>
    class stream_traits<Stream, true, true>
    {
    private:

        enum last_stream_operation
        {
            read,
            write
        };

    public:
        explicit stream_traits(Stream& stream)
            : m_stream(stream), m_last_op(read) {}

        void do_read(
            void* buffer, ULONG buffer_size_in_bytes, ULONG& bytes_read_out)
        {
            bytes_read_out = 0U;

            // sync reading position with writing position, which was the last
            // one used and is allowed to be different in C++ streams but
            // not COM IStreams
            if (m_last_op == write)
            {
                m_stream.seekg(m_stream.tellp());
                // We ignore errors syncing the positions as even iostreams may
                // not be seekable at all

                m_last_op = read;
            }
            assert(m_last_op == read);

            do_istream_read(
                m_stream, buffer, buffer_size_in_bytes, bytes_read_out);
        }

        void do_write(
            const void* buffer, ULONG buffer_size_in_bytes,
            ULONG& bytes_written_out)
        {
            bytes_written_out = 0U;

            // sync writing position with reading position, which was the last
            // one used and is allowed to be different in C++ streams but
            // not COM IStreams
            if (m_last_op == read)
            {
                m_stream.seekp(m_stream.tellg());
                // We ignore errors syncing the positions as even iostreams may
                // not be seekable at all

                m_last_op = write;
            }
            assert(m_last_op == write);

            do_ostream_write(
                m_stream, buffer, buffer_size_in_bytes, bytes_written_out);
        }

        void do_seek(
            std::streamoff offset, std::ios_base::seekdir way,
            std::streampos& new_position_out)
        {

            // Unlike with do_read/do_write, we do not ignore errors when
            // trying to sync the read/write positions as, if the
            // first seek succeeded, we know the stream supports
            // seeking.  Therefore a later error is really an error.

            if (m_last_op == read)
            {
                read_position_finaliser<Stream> position_out_updater(
                    m_stream, new_position_out);

                if (!m_stream.seekg(offset, way))
                {
                    throw std::runtime_error("Unable to change read position");
                }
                else
                {
                    if (!m_stream.seekp(m_stream.tellg()))
                    {
                        throw std::runtime_error(
                            "Unable to synchronise write position");
                    }
                }
            }
            else
            {
                write_position_finaliser<Stream> position_out_updater(
                    m_stream, new_position_out);

                if (!m_stream.seekp(offset, way))
                {
                    throw std::runtime_error("Unable to change write position");
                }
                else
                {
                    if (!m_stream.seekg(m_stream.tellp()))
                    {
                        throw std::runtime_error(
                            "Unable to synchronise read position");
                    }
                }
            }
        }

        std::streampos do_tell()
        {
            std::streampos pos;
            if (m_last_op == read)
            {
                pos = m_stream.tellg();
            }
            else
            {
                pos = m_stream.tellp();
            }

            if (!m_stream)
            {
                throw std::runtime_error("Stream position unavailable");
            }

            return pos;
        }

        void do_flush()
        {
            if (!m_stream.flush())
            {
                throw std::runtime_error(
                    "Unable to flush buffer to output sequence");
            }
        }

    private:
        stream_traits(const stream_traits&);
        stream_traits& operator=(const stream_traits&);

        Stream& m_stream;
        last_stream_operation m_last_op;
    };

    /**
     * Wrap COM IStream interface around C++ IOStream.
     *
     * Unlike C++ streams which may have separate read and write positions that
     * move independently, COM IStreams assume a single combined read/write head.
     * Therefore this wrapper always starts the next read or write operation
     * from the where the last operation finished, regardless of whether that
     * operation was a call to `Read` or `Write`.
     *
     * @note This only applies for as long as the read/write positions are
     *       modified only via this wrapper.  If the positions are modified by
     *       directly on the underlying IOStream, it is undefined whether the
     *       starting point for the next call to `Read`/`Write` is syncronised
     *       with the end of the previous operation.
     *
     * If operations on the inner stream results in failure (the `failbit`
     * is set), this is communicated via the COM-interface return code.  The
     * `failbit` is cleared before the call returns.  This allows further
     * wrapper methods to be called without having the clear the bit directly
     * on the underlying stream.  Fatal errors (`badbit`) and end-of-file
     * (`eofbit`) are left unchanged and remain visible in the underlying
     * stream.
     */
    template<typename Stream>
    class adapted_stream : public simple_object<IStream>
    {
    private:

        typedef impl::stream_traits<
            Stream,
            type_traits::super_sub_class<std::istream, Stream>::result,
            type_traits::super_sub_class<std::ostream, Stream>::result
        > stream_traits_type;

    public:

        adapted_stream(Stream& stream, const bstr_t& optional_name)
            : m_stream(stream), m_traits(stream),
            m_optional_name(optional_name) {}

        /**
         * Fill the given buffer with data read from the wrapped C++ stream.
         *
         * @param [in] buffer
         *     Destination of bytes to be read.
         *
         * @param [in] buffer_size
         *     Size of `buffer` and, therefore, the maximum number of bytes
         *     this method will read.  The method may read fewer than this
         *     number of bytes if the request cannot be fulfilled
         *     (e.g. end-of-file, error), in which case only the portion of
         *     `buffer` up to `*read_count_out` bytes from the start
         *     may be considered valid.
         *
         * @param [out] read_count_out
         *     Number of bytes actually read into `buffer`.  In other words,
         *     the extent of the valid bytes in `buffer` once this method
         *     returns.  This value is correct whether the method returns
         *     success or an error code.
         * 
         * @returns
         *     Error code indicating the success or failure of the method.
         * @retval `S_OK`
         *     If `buffer` was completely filled.
         * @retval `S_FALSE` if EOF reached before `buffer` was filled.
         *     `*read_count_out` gives the number of bytes that were read.
         * @retval a COM error code
         *     If there was a read error.
         *
         * Error behaviour rationale
         * -------------------------
         *
         * MSDN says that, if `read_count_out` < `buffer_size`, callers 
         * should treat both error codes and success codes as meaning
         * the end-of-file was reached.  However, doing so means they will be
         * unable to distinguish EOF from a genuine read error.  Therefore,
         * we make the stronger guarantee that only `S_FALSE` means EOF
         * while an error code always indicates a read error.
         * To accommodate callers who follow the documentation on MSDN,
         * we ensure that the read-count out-variable is correct even if the
         * wrapped stream encountered an error (`failbit`, `badbit`) or
         * threw an exception, because the caller may use the byte-count to
         * decide which bytes of the partial read are valid.
         *
         * In the error case, `*read_count_out` may be less than the number
         * of bytes actually read from the wrapped stream.  This is for
         * performance reasons: unless we read from the wrapped stream one
         * byte at a time, there is no way to calculate the the exact number
         * of bytes read if an exception is thrown part way through reading.
         * Byte-by-byte reading is hopelessly slow for some applications, such
         * as an unbuffered stream over a network, so our behaviour is a
         * pragmatic compromise. We believe it is within the (vaguely)
         * documented behaviour of `IStream` to treat the out-count in this way
         * for error cases as the count still delimits a valid region of bytes.
         */
        virtual HRESULT STDMETHODCALLTYPE Read( 
            void* buffer, ULONG buffer_size, ULONG* read_count_out)
        {
            impl::stream_failure_cleanser<Stream> state_resetter(m_stream);

            if (read_count_out)
            {
                *read_count_out = 0U;
            }

            try
            {
                if (!buffer)
                {
                    throw com_error("No buffer given", STG_E_INVALIDPOINTER);
                }

                // We use a dummy read count location if one is not given so
                // that do_read can keep it updated correctly even if
                // do_read throws an exception.
                ULONG dummy_read_count = 0U;
                if (!read_count_out)
                {
                    read_count_out = &dummy_read_count;
                }

                m_traits.do_read(buffer, buffer_size, *read_count_out);

                if (*read_count_out < buffer_size)
                {
                    assert(m_stream.eof());

                    return S_FALSE;
                }
                else
                {
                    assert(*read_count_out == buffer_size);
                    assert(m_stream.good());
                    return S_OK;
                }
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("Read", "adapted_stream");
        }

        /**
         * Write the given data to the wrapped C++ stream's controlled sequence.
         *
         * The wrapped stream is flushed before this method returns to ensure
         * the data is written to the controlled sequence (and any associated
         * errors are detected) rather than just to the stream's buffer.
         * Therefore, best performance is obtained if this method is called with
         * as much data as possible for the fewest number of flushes.
         *
         * @param [in] data
         *     Bytes to write to the controlled sequence.
         *
         * @param [in] data_size
         *     Size of `data` and the number of bytes to write.
         *
         * @param [out] written_count_out
         *     Number of bytes guaranteed to be written to the controlled
         *     sequence.  This will be `data_size` if writing succeeded.  If
         *     an error occurred, this may be fewer than the actual number of
         *     bytes written to the sequence.
         * 
         * @returns
         *     Error code indicating the success or failure of the method.
         * @retval `S_OK`
         *     If `data` was completely written to the wrapped stream's
         *     controlled sequence.
         * @retval a COM error code
         *     If there was a write error.
         *
         * Error behaviour rationale
         * -------------------------
         *
         * If writing succeeds, `*written_count_out` equals `data_size`.  If
         * writing encountered an error `*written_count_out` will be set to zero,
         * even if some bytes were written out.  We do this for performance
         * reasons: unless we write the data one byte at a time to the wrapped
         * stream _and flush the stream after each byte_, the C++ streams don't
         * provide a way to count the exact number of bytes written to the
         * stream's controlled sequence.  Byte-by-byte writing is hopelessly
         * slow for some applications, such as a stream over a network, so our
         * behaviour is a pragmatic compromise.
         * We believe it is within the (vaguely) documented behaviour of
         * `IStream` to treat the out-count in this way for error cases.
         * It may seem strange that this method even has an out-count if it
         * was no intended to be meaningful in the error case (the count would
         * be equal to `data_size` in the success case), however we believe
         * it was only intended to be meaningful for non-blocking writes to
         * asynchronous stream (`E_PENDING` would be returned) which this
         * wrapper is not (see http://bit.ly/1hZGy1L).
         */
        virtual HRESULT STDMETHODCALLTYPE Write(
            const void* data, ULONG data_size, ULONG* written_count_out)
        {
            stream_failure_cleanser<Stream> state_resetter(m_stream);

            if (written_count_out)
            {
                *written_count_out = 0U;
            }

            try
            {
                if (!data)
                {
                    throw com_error("Buffer not given", STG_E_INVALIDPOINTER);
                }

                // We use a dummy written count out location if one is not
                // given so that do_write can keep it updated correctly even if
                // do_write throws an exception.
                ULONG dummy_written_count = 0U;
                if (!written_count_out)
                {
                    written_count_out = &dummy_written_count;
                }

                m_traits.do_write(data, data_size, *written_count_out);

                if (*written_count_out < data_size)
                {
                    throw com_error(
                        "Unable to complete write operation", STG_E_MEDIUMFULL);
                }
                else
                {
                    assert(*written_count_out == data_size);
                    return S_OK;
                }
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("Write", "adapted_stream");
        }

        virtual HRESULT STDMETHODCALLTYPE Seek(
            LARGE_INTEGER offset, DWORD origin,
            ULARGE_INTEGER* new_position_out)
        {
            stream_failure_cleanser<Stream> state_resetter(m_stream);

            std::streampos new_stream_position_out = std::streampos();
            impl::position_out_converter out_param_guard(
                new_stream_position_out, new_position_out);

            try
            {
                std::ios_base::seekdir way;
                if (origin == STREAM_SEEK_CUR)
                {
                    way = std::ios_base::cur;
                }
                else if (origin == STREAM_SEEK_SET)
                {
                    way = std::ios_base::beg;
                }
                else if (origin == STREAM_SEEK_END)
                {
                    way = std::ios_base::end;
                }
                else
                {
                    throw com_error(
                        "Unrecognised stream seek origin",
                        STG_E_INVALIDFUNCTION);
                }

                if (offset.QuadPart > 
                    (std::numeric_limits<std::streamoff>::max)())
                {
                    throw com_error(
                        "Seek offset too large", STG_E_INVALIDFUNCTION);
                }
                else if (offset.QuadPart <
                    (std::numeric_limits<std::streamoff>::min)())
                {
                    throw com_error(
                        "Seek offset too small", STG_E_INVALIDFUNCTION);
                }
                else
                {
                    try
                    {
                        m_traits.do_seek(
                            static_cast<std::streamoff>(offset.QuadPart),
                            way, new_stream_position_out);
                    }
                    // Translate logic_errors (and subtypes) as they
                    // correspond (very roughly) to the kinds of errors
                    // for which IStream::Seek is documented to return
                    // STG_E_INVALIDFUNCTION
                    catch(const std::logic_error& e)
                    {
                        throw com_error(e.what(), STG_E_INVALIDFUNCTION);
                    }

                    return S_OK;
                }
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("Seek", "adapted_stream");
        }

        /**
         * Expand stream to given size.
         *
         * The will only increase a stream's size if it is smaller than the
         * given size.  If the stream size is already equal or bigger, it
         * remains unchanged.
         *
         * Not all streams support changing size.  In particular, `istream`s
         * do not support this method.
         */
        virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER new_size)
        {
            try
            {
                stream_failure_cleanser<Stream> state_resetter(m_stream);

                position_resetter<stream_traits_type> resetter(
                    m_traits, m_traits.do_tell());

                if (new_size.QuadPart > 0)
                {
                    ULARGE_INTEGER new_end_position;
                    new_end_position.QuadPart = new_size.QuadPart - 1;

                    std::streamoff new_offset;
                    if ((std::numeric_limits<std::streamoff>::max)() < 0)
                    {
                        assert(!"Purely negative number!");
                        throw com_error(
                            "Seek offset too large", STG_E_INVALIDFUNCTION);
                    }
                    else if (new_end_position.QuadPart > 
                        static_cast<ULONGLONG>(
                            (std::numeric_limits<std::streamoff>::max)()))
                    {
                        throw com_error(
                            "Seek offset too large", STG_E_INVALIDFUNCTION);
                    }
                    else
                    {
                        new_offset = static_cast<std::streamoff>(
                            new_end_position.QuadPart);
                    }

                    std::streamoff existing_end = stream_size(m_traits);

                    if (new_offset > existing_end)
                    {
                        std::streampos new_position;
                        m_traits.do_seek(
                            new_offset, std::ios_base::beg, new_position);

                        assert(std::streamoff(new_position) == new_offset);

                        // Force the stream to expand by writing NUL at
                        // new extent
                        ULONG bytes_written;
                        m_traits.do_write("\0", 1, bytes_written);
                        assert(bytes_written == 1);
                    }
                }

                return S_OK;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("SetSize", "adapted_stream");
        }

        virtual HRESULT STDMETHODCALLTYPE CopyTo( 
            IStream* destination, ULARGE_INTEGER amount,
            ULARGE_INTEGER* bytes_read_out, ULARGE_INTEGER* bytes_written_out)
        {
            stream_failure_cleanser<Stream> state_resetter(m_stream);

            ULARGE_INTEGER dummy_read_out;
            if (!bytes_read_out)
            {
                bytes_read_out = &dummy_read_out;
            }
            bytes_read_out->QuadPart = 0U;

            ULARGE_INTEGER dummy_written_out;
            if (!bytes_written_out)
            {
                bytes_written_out = &dummy_written_out;
            }
            bytes_written_out->QuadPart = 0U;

            try
            {

                if (!destination)
                {
                    throw com_error(
                        "Destination stream not given", STG_E_INVALIDPOINTER);
                }

                std::vector<unsigned char> buffer(COPY_CHUNK_SIZE);

                // Perform copy operation in chunks COPY_CHUNK bytes big
                // The chunk must be less than the biggest ULONG in size
                // because of the limits of the Read/Write API.  Of course
                // it will be in any case as it would be insane to use more
                // memory than that, but we make sure anyway using the first
                // min comparison
                do {
                    ULONG next_chunk_size =
                        static_cast<ULONG>(
                            min(
                                (std::numeric_limits<ULONG>::max)(),
                                min(
                                    amount.QuadPart - bytes_read_out->QuadPart,
                                    buffer.size())));

                    ULONG read_this_round = 0U;
                    ULONG written_this_round = 0U;

                    // These two take care of updating the total on each pass
                    // round the loop as well as on termination, exception or
                    // natural.
                    //
                    // The means the out counts are valid even in the failure
                    // case. MSDN says they don't have to be but, as we can,
                    // we might as well
                    impl::byte_count_incrementer read_incrementer(
                        bytes_read_out, read_this_round);

                    impl::byte_count_incrementer write_incrementer(
                        bytes_written_out, written_this_round);

                    m_traits.do_read(
                        &buffer[0], next_chunk_size, read_this_round);
                    HRESULT hr = destination->Write(
                        &buffer[0], read_this_round, &written_this_round);
                    if (FAILED(hr))
                    {
                        com_error_from_interface(destination, hr);
                    }

                    if (read_this_round < next_chunk_size)
                    {
                        // EOF
                        break;
                    }

                } while (amount.QuadPart > bytes_read_out->QuadPart);

                return S_OK;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("CopyTo", "adapted_stream");
        }

        /**
         * Flush data in the buffer to the controlled output sequence.
         *
         * This implementation doesn't support transactions so ignores
         * the commit flags.
         *
         * Fails if called with an istream.
         */
        virtual HRESULT STDMETHODCALLTYPE Commit(DWORD /*commit_flags*/)
        {
            stream_failure_cleanser<Stream> state_resetter(m_stream);

            try
            {
                m_traits.do_flush();
                return S_OK;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("Commit", "adapted_stream");
        }

        virtual HRESULT STDMETHODCALLTYPE Revert()
        {
            try
            {
                throw com_error("Transactions not supported", E_NOTIMPL);
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("Revert", "adapted_stream");
        }

        virtual HRESULT STDMETHODCALLTYPE LockRegion(
            ULARGE_INTEGER /*offset*/, ULARGE_INTEGER /*extent*/,
            DWORD /*lock_type*/)
        {
            try
            {
                throw com_error(
                    "Locking not supported", STG_E_INVALIDFUNCTION);
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("LockRegion", "adapted_stream");
        }

        virtual HRESULT STDMETHODCALLTYPE UnlockRegion(
            ULARGE_INTEGER /*offset*/, ULARGE_INTEGER /*extent*/,
            DWORD /*lock_type*/)
        {
            try
            {
                throw com_error(
                    "Locking not supported", STG_E_INVALIDFUNCTION);
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("UnlockRegion", "adapted_stream");
        }

        /**
         * Get some metadata about the stream.
         *
         * The name returned (if requested) is the name optionally given
         * in the constructor.  If not given in the constructor, an
         * empty string is returned.
         *
         * Some fields, such as the date fields, are not valid as that data
         * is not available for IOStreams.
         */
        virtual HRESULT STDMETHODCALLTYPE Stat( 
            STATSTG* attributes_out, DWORD stat_flag)
        {
            try
            {
                if (!attributes_out)
                {
                    throw com_error("STATSTG not given", STG_E_INVALIDPOINTER);
                }

                *attributes_out = STATSTG();

                attributes_out->type = STGTY_STREAM;
                
                try
                {
                    attributes_out->cbSize.QuadPart =
                        impl::stream_size(m_traits);
                }
                catch (const std::exception&)
                {
                    // Swallow non-bad errors as many stream are not seekable and
                    // therefore not sizeable
                    if (m_stream.bad())
                        throw;
                }

                // Must be last as, after we detach, any failure will leak
                // memory
                if (!(stat_flag & STATFLAG_NONAME))
                {
                    // pwcsName is NOT a BSTR.  It's a null-terminated OLESTR
                    // managed by the COM memory allocator

                    size_t buffer_size = m_optional_name.size() + 1;
                    size_t buffer_size_in_bytes =
                        buffer_size * sizeof(wchar_t);
                    
                    attributes_out->pwcsName = static_cast<LPOLESTR>(
                        ::CoTaskMemAlloc(buffer_size_in_bytes));
                    if (!attributes_out->pwcsName)
                    {
                        throw com_error(
                            "Unable to allocate memory for stream name",
                            STG_E_INSUFFICIENTMEMORY);
                    }

                    try
                    {
                        HRESULT hr = ::StringCbCopyW(
                            attributes_out->pwcsName, buffer_size_in_bytes,
                            m_optional_name.c_str());
                        if (FAILED(hr))
                        {
                            throw com_error(
                                "Unable to copy stream name to STATSTG", hr);
                        }
                    }
                    catch(...)
                    {
                        ::CoTaskMemFree(attributes_out->pwcsName);
                        throw;
                    }
                }

                return S_OK;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("Stat", "adapted_stream");
        }

        /**
         * Cloning not supported for IOStreams as they are not copyable.
         */
        virtual HRESULT STDMETHODCALLTYPE Clone(IStream** stream_out)
        {
            if (stream_out)
            {
                *stream_out = NULL;
            }

            try
            {
                throw com_error(
                    "Cloning not supported", STG_E_INVALIDFUNCTION);
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY("Clone", "adapted_stream");
        }

    private:

        Stream& m_stream;
        stream_traits_type m_traits;
        bstr_t m_optional_name;
    };

    template<typename StreamPtr>
    class adapted_stream_pointer : public simple_object<IStream>
    {
    public:

        adapted_stream_pointer(StreamPtr stream, const bstr_t& optional_name)
            : m_stream(stream), m_inner(adapt_stream(*m_stream, optional_name))
        {}

        // The forwarded methods must return their HRESULT by throwing and
        // catching in order to propagate the IErrorInfo upwards.

        virtual HRESULT STDMETHODCALLTYPE Read( 
            void* buffer, ULONG buffer_size, ULONG* read_count_out)
        {
            if (read_count_out)
            {
                *read_count_out = 0U;
            }

            try
            {
                HRESULT hr = m_inner->Read(buffer, buffer_size, read_count_out);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);
                
                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "Read", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE Write(
            const void* buffer, ULONG buffer_size, ULONG* written_count_out)
        {
            if (written_count_out)
            {
                *written_count_out = 0U;
            }

            try
            {
                HRESULT hr = m_inner->Write(
                    buffer, buffer_size, written_count_out);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);

                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "Write", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE Seek(
            LARGE_INTEGER offset, DWORD origin,
            ULARGE_INTEGER* new_position_out)
        {
            if (new_position_out)
            {
                new_position_out->QuadPart = 0U;
            }

            try
            {
                HRESULT hr = m_inner->Seek(offset, origin, new_position_out);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);
                
                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "Seek", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER new_size)
        {
            try
            {
                HRESULT hr = m_inner->SetSize(new_size);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);

                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "SetSize", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE CopyTo( 
            IStream* destination, ULARGE_INTEGER amount,
            ULARGE_INTEGER* bytes_read_out, ULARGE_INTEGER* bytes_written_out)
        {
            if (bytes_read_out)
            {
                bytes_read_out->QuadPart = 0U;
            }

            if (bytes_written_out)
            {
                bytes_written_out->QuadPart = 0U;
            }

            try
            {
                HRESULT hr = m_inner->CopyTo(
                    destination, amount, bytes_read_out, bytes_written_out);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);

                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "CopyTo", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE Commit(DWORD commit_flags)
        {
            try
            {
                HRESULT hr = m_inner->Commit(commit_flags);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);

                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "Commit", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE Revert()
        {
            try
            {
                HRESULT hr = m_inner->Revert();
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);

                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "Revert", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE LockRegion(
            ULARGE_INTEGER offset, ULARGE_INTEGER extent, DWORD lock_type)
        {
            try
            {
                HRESULT hr = m_inner->LockRegion(offset, extent, lock_type);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);

                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "LockRegion", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE UnlockRegion(
            ULARGE_INTEGER offset, ULARGE_INTEGER extent, DWORD lock_type)
        {
            try
            {
                HRESULT hr = m_inner->UnlockRegion(offset, extent, lock_type);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);

                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "UnlockRegion", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE Stat( 
            STATSTG* attributes_out, DWORD stat_flag)
        {
            if (attributes_out)
            {
                *attributes_out = STATSTG();
            }

            try
            {
                HRESULT hr = m_inner->Stat(attributes_out, stat_flag);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);

                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "Stat", "adapted_stream_pointer");
        }

        virtual HRESULT STDMETHODCALLTYPE Clone(IStream** stream_out)
        {
            if (stream_out)
            {
                *stream_out = NULL;
            }

            try
            {
                HRESULT hr = m_inner->Clone(stream_out);
                if (FAILED(hr))
                    throw com_error_from_interface(m_inner, hr);

                return hr;
            }
            COMET_CATCH_CLASS_INTERFACE_BOUNDARY(
                "Clone", "adapted_stream_pointer");
        }

    private:

        StreamPtr m_stream;
        com_ptr<IStream> m_inner;
    };

}

/**
 * Wrap COM IStream interface around C++ IOStream.
 *
 * The caller must ensure that the C++ IOStream remains valid until the
 * last reference to the returned wrapper is released.
 *
 * Unlike C++ streams which may have separate read and write positions that
 * move independently, COM IStreams assume a single combined read/write head.
 * Therefore this wrapper always starts the next read or write operation
 * from the where the last operation finished, regardless of whether that
 * operation was a call to `Read` or `Write`.
 *
 * @note This only applies for as long as the read/write positions are
 *       modified only via this wrapper.  If the positions are modified by
 *       directly on the underlying IOStream, it is undefined whether the
 *       starting point for the next call to `Read`/`Write` is syncronised
 *       with the end of the previous operation.
 *
 * If operations on the inner stream results in failure (the `failbit`
 * is set), this is communicated via the COM-interface return code.  The
 * `failbit` is cleared before the call returns.  This allows further
 * wrapper methods to be called without having the clear the bit directly
 * on the underlying stream.  Fatal errors (`badbit`) and end-of-file
 * (`eofbit`) are left unchanged and remain visible in the underlying
 * stream.
 */
template<typename Stream>
inline com_ptr<IStream> adapt_stream(
    Stream& stream, const bstr_t& optional_name=bstr_t())
{
    return new impl::adapted_stream<Stream>(stream, optional_name);
}

/**
 * Wrap COM IStream interface around pointer (usually smart) to C++ IOStream.
 *
 * If the pointer type is a smart pointer, the caller need not must ensure
 * the lifetime of the C++ IOStream exceeds that of the adapter; the smart
 * pointer takes care of ensuring this.
 *
 * The main reason for having this function in addition to `adapt_stream` is
 * support the common case where a COM stream is intended to be the sole owner
 * of the C++ stream with which it is created.  This allows the caller to
 * create the IStream and forget about the C++ stream.  Using `adapt_stream`
 * they would have to manage the lifetime of both.
 *
 * When we support movable types (C++11 or Boost.Move emulation), this method
 * would become unnecessary for this purpose as the C++ stream could simply
 * be moved into the adapter
 */
template<typename StreamPtr>
inline com_ptr<IStream> adapt_stream_pointer(
    StreamPtr stream_pointer, const bstr_t& optional_name=bstr_t())
{
    return new impl::adapted_stream_pointer<StreamPtr>(
        stream_pointer, optional_name);
}

}

#endif
