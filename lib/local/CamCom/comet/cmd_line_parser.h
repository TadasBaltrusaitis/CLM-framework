/** \file
  * Command line parser.
  */
/*
 * Copyright © 2002 Mikael Lindgren
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

#ifndef COMET_COMMAND_LINE_PARSER_H
#define COMET_COMMAND_LINE_PARSER_H

#include <comet/tstring.h>

namespace comet {

    /*! \addtogroup Misc
     */
    //@{
    class cmd_line_parser
    {
    public:
        enum kind_t {
            Name,        // Name type token, has no leading / or -
            Option,    // Option type token. Leading / or - skipped by token
            Value,    // Value for name or option. Leading : or = skipped by token
            Done        // No more tokens
        };

        explicit cmd_line_parser(const TCHAR* cmd_line): cmd_line_(cmd_line)
        {
            reset();
        }

        kind_t get_next_token(tstring& result)
        {
            static const TCHAR terminators[] = _T("=/- \t");
            static const TCHAR white_space[] = _T(" \t");

            kind_t kind;

            token_ = next_token_ + _tcsspn(next_token_, white_space);    // Skip leading whitespace
            switch (*token_)
            {
                case 0:
                    return Done;
                case _T('-'):
                case _T('/'):
                    kind = Option;
                    ++token_;
                    break;
                case _T('='):
                    kind = Value;
                    ++token_;
                    break;
                default:
                    kind = Name;
                    break;
            }
            if (kind == Option || kind == Value)
                token_ += _tcsspn(token_, white_space);    // Skip any more whitespace
            if (*token_ == _T('"'))
            {
                const TCHAR* next = _tcschr(token_+1, _T('"'));
                if ( next )
                {
                    result.assign( token_+1, next );
                    next_token_ = next+1;
                    return kind;
                }
            }
            next_token_ = token_ + _tcscspn(token_, terminators);
            result.assign(token_, next_token_);
            return kind;
        }

        void reset()
        {
            token_ = 0;
            next_token_ = cmd_line_;
        }

    private:
        const TCHAR* cmd_line_;
        const TCHAR* token_;
        const TCHAR* next_token_;
    };
    //@}

}

#endif
