//---------------------------------------------------------------------------
// a_nullstream.h
//
// Null ostream - discards anything written to it.
// Code taken from a ng post I can no longer locate, but it is PD.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_NULLSTREAM_H
#define INC_A_NULLSTREAM_H
#include <iostream>

namespace ALib {

//----------------------------------------------------------------------------

template <class cT, class traits = std::char_traits<cT> >
class basic_nullbuf: public std::basic_streambuf<cT, traits> {
    typename traits::int_type overflow(typename traits::int_type c)
    {
        return traits::not_eof(c); // indicate success
    }
};

template <class cT, class traits = std::char_traits<cT> >
class basic_onullstream: public std::basic_ostream<cT, traits> {
    public:
        basic_onullstream():
        std::basic_ios<cT, traits>(&m_sbuf),
        std::basic_ostream<cT, traits>(&m_sbuf)
        {
                this->init(&m_sbuf);
        }

    private:
        basic_nullbuf<cT, traits> m_sbuf;
};

typedef basic_onullstream<char> NullStream;

//----------------------------------------------------------------------------

} // namespace


#endif

