/** \file
  * Lightweight Multiple Reader Single Writer lock.
  *
  * See \ref cometlwlock.
  *
  * The lw_lock class is heavily based on class LightweightLock written by Brad Wilson.
  * see http://www.quality.nu/dotnetguy/archive/fog0000000007.aspx
  * \author Brad Wilson
  * \author Sofus Mortensen
  */

//  Copyright (C) 1995-2002 Brad Wilson
//
//  This material is provided "as is", with absolutely no warranty
//  expressed or implied. Any use is at your own risk. Permission to
//  use or copy this software for any purpose is hereby granted without
//  fee, provided the above notices are retained on all copies.
//  Permission to modify the code and to distribute modified code is
//  granted, provided the above notices are retained, and a notice that
//  the code was modified is included with the above copyright notice.
//
////////////////////////////////////////////////////////////////////////

/** \page cometlwlock Lightweight Lock
  This lightweight lock class was adapted from samples and ideas that
  were put across the ATL mailing list. It is a non-starving, kernel-
  free lock that does not order writer requests. It is optimized for
  use with resources that can take multiple simultaneous reads,
  particularly when writing is only an occasional task.

  Multiple readers may acquire the lock without any interference with
  one another. As soon as a writer requests the lock, additional
  readers will spin. When the pre-writer readers have all given up
  control of the lock, the writer will obtain it. After the writer
  has rescinded control, the additional readers will gain access
  to the locked resource.

  This class is very lightweight. It does not use any kernel objects.
  It is designed for rapid access to resources without requiring
  code to undergo process and ring changes. Because the "spin"
  method for this lock is "Sleep(0)", it is a good idea to keep
  the lock only long enough for short operations; otherwise, CPU
  will be wasted spinning for the lock. You can change the spin
  mechanism by #define'ing COMET_LW_LOCK_SPIN before including this
  header file.

  VERY VERY IMPORTANT: If you have a lock open with read access and
  attempt to get write access as well, you will deadlock! Always
  rescind your read access before requesting write access (and,
  of course, don't rely on any read information across this).

  This lock works in a single process only. It cannot be used, as is,
  for cross-process synchronization. To do that, you should convert
  this lock to using a semaphore and mutex, or use shared memory to
  avoid kernel objects.
  */

/*
 * Copyright © 2002 Sofus Mortensen
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

#include <comet/config.h>
#include <comet/assert.h>

#include <windows.h>

namespace comet {

#ifndef COMET_LW_LOCK_SPIN
#define COMET_LW_LOCK_SPIN Sleep(0)
#endif
    /*! \addtogroup Misc
     */
    //@{

    /** Provide a lightweight lock imlementation.
     *  See \ref cometlwlock for more information and warnings.
     *  \sa auto_reader_lock auto_writer_lock
     */
    class lw_lock
    {
    //  Interface

    public:
        ///  Constructor
        lw_lock()
        {
            reader_count_ = 0;
            writer_count_ = 0;
        }

        ///  Destructor
        ~lw_lock()
        {
            COMET_ASSERT( reader_count_ == 0 );
            COMET_ASSERT( writer_count_ == 0 );
        }

        ///  Reader lock acquisition
        void enter_reader() const
        {
            for (;;)
            {
                //  If there's a writer already, spin without unnecessarily
                //  interlocking the CPUs

                if( writer_count_ != 0 )
                {
                    COMET_LW_LOCK_SPIN;
                    continue;
                }

                //  Add to the readers list

                InterlockedIncrement((long *)&reader_count_ );

                //  Check for writers again (we may have been pre-empted). If
                //  there are no writers writing or waiting, then we're done.

                if( writer_count_ == 0 )
                    break;

                //  Remove from the readers list, spin, try again

                InterlockedDecrement((long *)&reader_count_ );
                COMET_LW_LOCK_SPIN;
            }
        }

        ///  Reader lock release
        void leave_reader() const
        {
            InterlockedDecrement((long *)&reader_count_ );
        }

        /// Writer lock acquisition
        void enter_writer()
        {
            //  See if we can become the writer (expensive, because it inter-
            //  locks the CPUs, so writing should be an infrequent process)

            while( InterlockedExchange((long *)&writer_count_, 1 ) == 1 )
            {
                COMET_LW_LOCK_SPIN;
            }

            //  Now we're the writer, but there may be outstanding readers.
            //  Spin until there aren't any more; new readers will wait now
            //  that we're the writer.

            while( reader_count_ != 0 )
            {
                COMET_LW_LOCK_SPIN;
            }
        }

        ///  Writer lock release
        void leave_writer()
        {
            writer_count_ = 0;
        }

    //  Implementation

    private:
        mutable long volatile reader_count_;
        mutable long volatile writer_count_;

        // Declare class non-copyable
        lw_lock(const lw_lock&);
        lw_lock& operator=(const lw_lock&);
    };

    /** \class auto_reader_lock lw_lock.h comet/lw_lock.h
     *  Auto-release locking class for lw_lock Read acces.
     *  \sa lw_lock auto_writer_lock
     */
    class auto_reader_lock {
    public:
        explicit auto_reader_lock(const lw_lock& cs) : cs_(cs) {
            cs_.enter_reader();
        }

        ~auto_reader_lock() {
            cs_.leave_reader();
        }

    private:
        auto_reader_lock& operator=(const auto_reader_lock&);
        auto_reader_lock(const auto_reader_lock&);

        const lw_lock& cs_;
    };

    /** \class auto_writer_lock lw_lock.h comet/lw_lock.h
     *  Auto-release locking class for lw_lock write acces.
     *  \sa lw_lock auto_reader_lock
     */
    class auto_writer_lock {
    public:
        explicit auto_writer_lock(lw_lock& cs) : cs_(cs) {
            cs_.enter_writer();
        }

        ~auto_writer_lock() {
            cs_.leave_writer();
        }

    private:
        auto_writer_lock& operator=(const auto_writer_lock&);
        auto_writer_lock(const auto_writer_lock&);

        lw_lock& cs_;
    };
    //@}

}
