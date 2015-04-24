/** \file
  * _NewEnum style COM enumerator backed by a smart pointer to items.
  */
/*
 * Copyright © 2010 Alexander Lamaison
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

#ifndef COMET_SMART_ENUM_H
#define COMET_SMART_ENUM_H

#include <comet/config.h>

#include <comet/enum_common.h>
#include <comet/server.h>

namespace comet {

    namespace impl {

        template<typename SmartPtr>
        class smart_enum_source
        {
        public:
            typedef typename SmartPtr::element_type::const_iterator
                const_iterator;

            explicit smart_enum_source(SmartPtr source)
                : source_(source), it_(begin()) {}

            const_iterator begin()
            {
                return source_->begin();
            }

            const_iterator end()
            {
                return source_->end();
            }

            const_iterator& current()
            {
                return it_;
            }

        private:
            SmartPtr source_;
            const_iterator it_;
        };

    }

    /**
     * Implements _NewEnum style COM object on top of smart pointer to
     * a collection.
     * \param Itf Enumeration Interface.
     * \param C STL Style container.
     * \param T Iteration Element type
     * \param CONVERTER Converts container element to \p T type. (std::identity<C::value_type>)
     * \sa stl_enumeration create_enum
     */
    template<
        typename Itf, typename SmartPtr, typename T,
        typename CONVERTER=std::identity<
            COMET_STRICT_TYPENAME SmartPtr::element_type::value_type> >
    class smart_enumeration :
        public impl::enumeration<
            Itf, T, CONVERTER, impl::smart_enum_source<SmartPtr> >
    {
    public:
        smart_enumeration(
            SmartPtr container, const CONVERTER& converter=CONVERTER())
            : enumeration(
                impl::smart_enum_source<SmartPtr>(container), converter) {}

    private:
        smart_enumeration(const smart_enumeration&);
        smart_enumeration& operator=(const smart_enumeration&);
    };

    /**
     * Smart Enumeration creation helper.
     *
     * Creates the enumeration with the element type specified by the
     * enumerated_type_of policy.  To specify the element type explicitly, use
     * smart_enumeration directly.
     *
     * \tparam ET        Enumeration Type e.g. IEnumUnknown.
     * \tparam SmartPtr  Smart pointer (inferred from @a container parameter).
     *
     * \param container  Smart pointer to an STL collection
     *                   e.g. auto_ptr<vector>.
     */
    template<typename ET, typename SmartPtr>
    inline com_ptr<ET> make_smart_enumeration(SmartPtr container)
    {
        typedef typename enumerated_type_of<ET>::is T;
        typedef std::identity<
            COMET_STRICT_TYPENAME SmartPtr::element_type::value_type>
            CONVERTER;
        return new smart_enumeration<ET, SmartPtr, T, CONVERTER>(container);
    }

    /**
     * Smart Enumeration creation helper with custom converter.
     *
     * Creates the enumeration with the element type specified by the
     * enumerated_type_of policy.  To specify the element type explicitly, use
     * smart_enumeration directly.
     *
     * \tparam ET        Enumeration Type e.g. IEnumUnknown.
     * \tparam SmartPtr  Smart pointer (inferred from @a container parameter).
     * \tparam CONVERTER Converter type (inferred from @a converter).
     *
     * \param container  Smart pointer to an STL collection
     *                   e.g. auto_ptr<vector>.
     * \param converter  Custom converter.
     */
    template<typename ET, typename SmartPtr, typename CONVERTER>
    inline com_ptr<ET> make_smart_enumeration(
        SmartPtr container, const CONVERTER& converter)
    {
        typedef typename enumerated_type_of<ET>::is T;
        return new smart_enumeration<ET, SmartPtr, T, CONVERTER>(
            container, converter);
    }
}

#endif
