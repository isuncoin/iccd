//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2012, 2013 Ripple Labs Inc.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#ifndef RIPPLE_OVERLAY_MESSAGE_H_INCLUDED
#define RIPPLE_OVERLAY_MESSAGE_H_INCLUDED

#include "ripple.pb.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <beast/cxx14/type_traits.h> // <type_traits>

namespace ripple {

// VFALCO NOTE If we forward declare Message and write out shared_ptr
//             instead of using the in-class type alias, we can remove the entire
//             ripple.pb.h from the main headers.
//

// packaging of messages into length/type-prepended buffers
// ready for transmission.
//
// Message implements simple "packing" of protocol buffers Messages into
// a string prepended by a header specifying the message length.
// MessageType should be a Message class generated by the protobuf compiler.
//

class Message : public std::enable_shared_from_this <Message>
{
public:
    using pointer = std::shared_ptr<Message>;

public:
    /** Number of bytes in a message header.
    */
    static size_t const kHeaderBytes = 6;

    Message (::google::protobuf::Message const& message, int type);

    /** Retrieve the packed message data. */
    std::vector <uint8_t> const&
    getBuffer () const
    {
        return mBuffer;
    }

    /** Get the traffic category */
    int
    getCategory () const
    {
        return mCategory;
    }

    /** Determine bytewise equality. */
    bool operator == (Message const& other) const;

    /** Calculate the length of a packed message. */
    /** @{ */
    static unsigned getLength (std::vector <uint8_t> const& buf);

    template <class FwdIter>
    static
    std::enable_if_t<std::is_same<typename
        FwdIter::value_type, std::uint8_t>::value, std::size_t>
    size (FwdIter first, FwdIter last)
    {
        if (std::distance(first, last) <
                Message::kHeaderBytes)
            return 0;
        std::size_t n;
        n  = std::size_t{*first++} << 24;
        n += std::size_t{*first++} << 16;
        n += std::size_t{*first++} <<  8;
        n += std::size_t{*first};
        return n;
    }

    template <class BufferSequence>
    static
    std::size_t
    size (BufferSequence const& buffers)
    {
        return size(buffers_begin(buffers),
            buffers_end(buffers));
    }
    /** @} */

    /** Determine the type of a packed message. */
    /** @{ */
    static int getType (std::vector <uint8_t> const& buf);

    template <class FwdIter>
    static
    std::enable_if_t<std::is_same<typename
        FwdIter::value_type, std::uint8_t>::value, int>
    type (FwdIter first, FwdIter last)
    {
        if (std::distance(first, last) <
                Message::kHeaderBytes)
            return 0;
        return (int{*std::next(first, 4)} << 8) |
            *std::next(first, 5);
    }

    template <class BufferSequence>
    static
    int
    type (BufferSequence const& buffers)
    {
        return type(buffers_begin(buffers),
            buffers_end(buffers));
    }
    /** @} */

private:
    template <class BufferSequence, class Value = std::uint8_t>
    static
    boost::asio::buffers_iterator<BufferSequence, Value>
    buffers_begin (BufferSequence const& buffers)
    {
        return boost::asio::buffers_iterator<
            BufferSequence, Value>::begin (buffers);
    }

    template <class BufferSequence, class Value = std::uint8_t>
    static
    boost::asio::buffers_iterator<BufferSequence, Value>
    buffers_end (BufferSequence const& buffers)
    {
        return boost::asio::buffers_iterator<
            BufferSequence, Value>::end (buffers);
    }

        // Encodes the size and type into a header at the beginning of buf
    //
    void encodeHeader (unsigned size, int type);

    std::vector <uint8_t> mBuffer;

    int mCategory;
};

}

#endif
