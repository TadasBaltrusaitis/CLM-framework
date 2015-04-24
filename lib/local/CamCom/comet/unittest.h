/** \file
 * Unit-test wrapper.
 */
/*
 * Copyright © 2000, 2001 Paul Hollingsworth
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

#ifndef COMET_UNITTEST_H
#define COMET_UNITTEST_H

#include <exception>
#include <iostream>

namespace comet {
    //! Define a unit test \a n.
    template<int n>
    struct test
    {
        void run() {}
    };

    int current_test = 0;

    //! Run \a n tests.
    /** \sa test
     */
    template<int n>
    struct tester
    {
        inline static void runtests()
        {
            tester<n-1>::runtests();
            current_test = n;
            test<n>().run();
        }
    };

    template<>
    struct tester<0>
    {
        static void runtests() {}
    };
} // namespace comet

//! Implement Main function for testing.
template<typename T>
struct main_t
{
    /// Call to run tests.
    static int call(int argc, const char * const argv[])
    {
        using std::cout;
        using std::cerr;
        using std::endl;
        try {
            comet::tester<64>::runtests();
        } catch (const std::exception &e) {
            std::cerr << "Test #" << comet::current_test << " failed - error message: <" << e.what() << ">" << endl;
            return 1;
        }
        catch(...) {
            std::cerr << "Test #" << comet::current_test << " failed with an unrecognized exception" << endl;
            return 2;
        }
        std::cout << "Ran all tests successfully" <<endl;
        return 0;
    }
};
#define COMET_TEST_MAIN \
int main(int argc, const char * const argv[]) \
{\
    return main_t<void>::call(argc,argv);\
}

#endif
