/*
    Copyright 2005-2013 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks.

    Threading Building Blocks is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    Threading Building Blocks is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Threading Building Blocks; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/

#if !defined(__TBB_machine_H) || defined(__TBB_machine_gcc_itsx_H)
#error Do not #include this internal file directly; use public TBB headers instead.
#endif

#define __TBB_machine_gcc_itsx_H

#define __TBB_OP_XACQUIRE 0xF2
#define __TBB_OP_XRELEASE 0xF3
#define __TBB_OP_LOCK     0xF0

#define __TBB_STRINGIZE_INTERNAL(arg) #arg
#define __TBB_STRINGIZE(arg) __TBB_STRINGIZE_INTERNAL(arg)

#ifdef __TBB_x86_64
#define __TBB_r_out "=r"
#else
#define __TBB_r_out "=q"
#endif

inline static uint8_t __TBB_machine_try_lock_elided( volatile uint8_t* lk )
{
    uint8_t value = 1;
    __asm__ volatile (".byte " __TBB_STRINGIZE(__TBB_OP_XACQUIRE)"; lock; xchgb %0, %1;"
                      : __TBB_r_out(value), "=m"(*lk)  : "0"(value), "m"(*lk) : "memory" );
    return uint8_t(value^1);
}

inline static void __TBB_machine_try_lock_elided_cancel()
{
    // 'pause' instruction aborts HLE/RTM transactions
    __asm__ volatile ("pause\n" : : : "memory" );
}

inline static void __TBB_machine_unlock_elided( volatile uint8_t* lk )
{
    __asm__ volatile (".byte " __TBB_STRINGIZE(__TBB_OP_XRELEASE)"; movb $0, %0" 
                      : "=m"(*lk) : "m"(*lk) : "memory" );
}
