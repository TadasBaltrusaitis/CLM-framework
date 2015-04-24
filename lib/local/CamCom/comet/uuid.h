/** \file
  * uuid wrapper class.
  */
/*
 * Copyright © 2001, 2002 Sofus Mortensen
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

#ifndef COMET_UUID_H
#define COMET_UUID_H

#include <comet/config.h>
#include <comet/uuid_fwd.h>
#include <comet/bstr.h>

#pragma comment( lib, "Rpcrt4" )

namespace comet {

    template<typename C>
    void uuid_t::copy_to_str(C s[36]) const throw()
    {
        const unsigned char *p = reinterpret_cast<const unsigned char*>(this);

        for (int i=0; i<20; ++i)
        {
            int j = uuid_table()[i];
            if (j >= 0)
            {
                const unsigned char byt = p[j];
                *s = hex_table()[byt >> 4];
                ++s;
                *s = hex_table()[byt & 0xf];
            }
            else *s = L'-';
            ++s;
        }
    }

    template<typename C>
    bool uuid_t::init_from_str(const C s[], size_t len) throw()
    {
        unsigned char *p = reinterpret_cast<unsigned char*>(this);

        bool has_brace;
        switch (len)
        {
            default: return false;
            case 36: has_brace = false; break;
            case 38:
                if (*s != C('{'))
                    return false;
                has_brace = true;
                ++s;
                break;
        }

        int i;
        for (i=0; i<20; ++i)
        {
            int j = uuid_table()[i];
            if (j >= 0)
            {
                int a = parse_nibble(*s);
                ++s;
                int b = parse_nibble(*s);
                p[j] = unsigned char(a << 4 | b);
            }
            else if (*s != C('-'))
                return false;
            ++s;
        }
        return (! has_brace) || (*s == C('}'));
    }

}

#endif
