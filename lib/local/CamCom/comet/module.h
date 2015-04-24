/** \file
  * Main functionality for providing a COM module.
  */
/*
 * Copyright © 2000-2002 Sofus Mortensen, Paul Hollingsworth, Michael Geddes, Mikael Lindgren
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

#ifndef COMET_MODULE_H
#define COMET_MODULE_H

#include <vector>
#include <comet/config.h>
#include <comet/tstring.h>
#include <comet/threading.h>

namespace comet {

    /*!\addtogroup Server
     */
    //@{
    namespace impl {
        /** \internal
         * 'Command' base class.
         */
        struct cmd_t
        {
            virtual void cmd() = 0;
            cmd_t() {}
        private:
            cmd_t(const cmd_t&);
            cmd_t& operator=(const cmd_t&);
        };

        /**  \internal
         * 'Command' to delete a pointer.
         *
         */
        template<typename T>
        struct itf_releaser_t : public cmd_t
        {
            void cmd() { IUnknown *p = p_; p_=NULL; p->Release(); }
            itf_releaser_t(T *&p) :p_(p) { }
        private:
            T *&p_;
            itf_releaser_t(const itf_releaser_t&);
            itf_releaser_t& operator=(const itf_releaser_t&);
        };

        template<typename T>
        cmd_t *create_itf_releaser( T *&p)
        {
            return new itf_releaser_t<T>(p);
        }

        /** 'Command' to Call object_dispose on an object.
         * \internal
         */
        template<typename T>
        struct pointer_deleter_t : public cmd_t
        {
            void cmd() { T *p = p_; p_=NULL; delete p;}
            pointer_deleter_t(T *&p) :p_(p) { }
        private:
            T *&p_;
            pointer_deleter_t(const pointer_deleter_t&);
            pointer_deleter_t& operator=(const pointer_deleter_t&);
        };

        /** 'Command' to Call object_dispose on an object.
         * \internal
         */
        template<typename T>
        struct object_disposer_t : public cmd_t
        {
            void cmd() { p_->object_dispose(); }
            object_disposer_t( T*p) : p_(p) {}
        private:
            T *p_;
            object_disposer_t(const object_disposer_t &);
            object_disposer_t &operator=(const object_disposer_t &);
        };
    }
    //! Create a pointer deleter command.
    /** A command to delete pointers, commonly used for shutdown.
     * \code
         module().add_object_to_dispose(create_pointer_deleter(new my_class_t()));
     * \endcode
     * \sa module_t::add_object_to_dispose
     */
    template<typename T>
    impl::cmd_t *create_pointer_deleter( T *&p)
    {
        return new impl::pointer_deleter_t<T>(p);
    }
    //! Create an interface releaser command.
    /** A command to release COM objects, commonly used for shutdown.
     * Used for singletons.
     * \code
         module().add_object_to_dispose(create_interface_releaser(new my_coclass()));
     * \endcode
     * \sa module_t::add_object_to_dispose
     */
    template<typename T>
    impl::cmd_t *create_interface_releaser( T *&p)
    {
        return new impl::itf_releaser_t<T>(p);
    }

    //! Create a more generic object 'disposer'.
    /** Creates a Command that calls a static object_dispose(p) method.
     * \code
         class my_class_t
        {
            object_dispose( my_class_t *val)
            {
                val->destroy_myself();
            }
        };
         module().add_object_to_dispose(create_object_disposer(new my_class_t()));
     * \endcode
     * \sa module_t::add_object_to_dispose
     */
    template<typename T>
    impl::cmd_t *create_object_disposer( T *p )
    {
        return new impl::object_disposer_t<T>(p);
    }

    /// COM module.
    struct module_t
    {
        //! \name Attributes
        //@{
        /// Return current reference count.
        long rc()
        {
            return rc_;
        }
        /// Retun the HINSTANCE of the module.
        HINSTANCE instance() const
        {
            return instance_;
        }

        /// Set the hinstance of the module.
        void instance(HINSTANCE h)
        {
            instance_ = h;
        }

        /// Return the module's critical_section.
        /** \code
              auto_cs lock( module().cs() );
            \endcode
          */
        const critical_section& cs() const
        {
            return cs_;
        }

        //@}

        //! \name Operations
        //@{

        /// Add to the module locks.
        void lock()
        {
            if (InterlockedIncrement(&rc_) == 1 && shutdown_event_ != 0)
            {
                auto_cs lock(cs_);
                if ( shutdown_event_->is_set())
                    shutdown_event_->reset();
            }
        }

        /// Decrement the module lock.
        void unlock()
        {
            if(InterlockedDecrement(&rc_)==0)
            {
                activity_ = true;
                if (shutdown_event_ != 0)
                {
                    auto_cs lock(cs_);
                    shutdown_event_->set();
                }
            }
        }

        /// Shutdown the server.
        void shutdown()
        {
            for (std::vector<impl::cmd_t*>::iterator it = objects_to_dispose_.begin(); it != objects_to_dispose_.end(); ++it)
            {
                (*it)->cmd();
                delete *it;
            }
            objects_to_dispose_.clear();
        }

        /// Set an event for shutdown.
        void set_shutdown_event(event& shutdown_event)
        {
            shutdown_event_ = &shutdown_event;
        }

        /// Remove the event for on shutdown.
        void clear_shutdown_event()
        {
            shutdown_event_ = 0;
        }

        /// Returns if there has been activity  on the module since last reset.
        bool has_activity() const
        {
            return rc_ != 0 || activity_;
        }

        /// Reset the activity marker.
        void reset_activity_flag()
        {
            activity_ = false;
        }

        /// Add an objet to be disposed on shutdown.
        void add_object_to_dispose(impl::cmd_t* p)
        {
            auto_cs lock(cs_);
            objects_to_dispose_.push_back(p);
        }
        //@}

    private:
        long rc_;
        bool activity_;
        event* shutdown_event_;
        HINSTANCE instance_;
        critical_section cs_;

        module_t() : rc_(0), activity_(false),
                     shutdown_event_(0), instance_(0)
        {}

        std::vector<impl::cmd_t*> objects_to_dispose_;

        friend module_t& module();

        // declare non-copyable
        module_t(const module_t&);
        module_t& operator=(const module_t&);
    };

    //! global module object
    inline module_t& module()
    {
        static module_t m;
        return m;
    }
    //@}
}

#endif
