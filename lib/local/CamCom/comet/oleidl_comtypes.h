/** \file
 * comtype<> definitions for interfaces in the COM header file OLEIDL.H.
 * \author Gabriel Barta
 */
/*
 * Copyright © 2002 Gabriel Barta
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

#ifndef COMET_OLEIDL_COMTYPES_H
#define COMET_OLEIDL_COMTYPES_H

#include <comet/interface.h>

namespace comet {
    template<> struct comtype< ::IOleAdviseHolder>
        : public uuid_comtype< ::IOleAdviseHolder,&IID_IOleAdviseHolder, ::IUnknown> {} ;
    template<> struct comtype< ::IOleCache>
        : public uuid_comtype< ::IOleCache,&IID_IOleCache, ::IUnknown> {} ;
    template<> struct comtype< ::IOleCache2>
        : public uuid_comtype< ::IOleCache2,&IID_IOleCache2, ::IOleCache> {} ;
    template<> struct comtype< ::IOleCacheControl>
        : public uuid_comtype< ::IOleCacheControl,&IID_IOleCacheControl, ::IUnknown> {} ;
    template<> struct comtype< ::IParseDisplayName>
        : public uuid_comtype< ::IParseDisplayName,&IID_IParseDisplayName, ::IUnknown> {} ;
    template<> struct comtype< ::IOleContainer>
        : public uuid_comtype< ::IOleContainer,&IID_IOleContainer, ::IParseDisplayName> {} ;
    template<> struct comtype< ::IOleClientSite>
        : public uuid_comtype< ::IOleClientSite,&IID_IOleClientSite, ::IUnknown> {} ;
    template<> struct comtype< ::IOleObject>
        : public uuid_comtype< ::IOleObject,&IID_IOleObject, ::IUnknown> {} ;
    template<> struct comtype< ::IOleWindow>
        : public uuid_comtype< ::IOleWindow,&IID_IOleWindow, ::IUnknown> {} ;
    template<> struct comtype< ::IOleLink>
        : public uuid_comtype< ::IOleLink,&IID_IOleLink, ::IUnknown> {} ;
    template<> struct comtype< ::IOleItemContainer>
        : public uuid_comtype< ::IOleItemContainer,&IID_IOleItemContainer, ::IOleContainer> {} ;
    template<> struct comtype< ::IOleInPlaceUIWindow>
        : public uuid_comtype< ::IOleInPlaceUIWindow,&IID_IOleInPlaceUIWindow, ::IOleWindow> {} ;
    template<> struct comtype< ::IOleInPlaceActiveObject>
        : public uuid_comtype< ::IOleInPlaceActiveObject,&IID_IOleInPlaceActiveObject, ::IOleWindow> {} ;
    template<> struct comtype< ::IOleInPlaceFrame>
        : public uuid_comtype< ::IOleInPlaceFrame,&IID_IOleInPlaceFrame, ::IOleInPlaceUIWindow> {} ;
    template<> struct comtype< ::IOleInPlaceObject>
        : public uuid_comtype< ::IOleInPlaceObject,&IID_IOleInPlaceObject, ::IOleWindow> {} ;
    template<> struct comtype< ::IOleInPlaceSite>
        : public uuid_comtype< ::IOleInPlaceSite,&IID_IOleInPlaceSite, ::IOleWindow> {} ;
    template<> struct comtype< ::IContinue>
        : public uuid_comtype< ::IContinue,&IID_IContinue, ::IUnknown> {} ;
    template<> struct comtype< ::IViewObject>
        : public uuid_comtype< ::IViewObject,&IID_IViewObject, ::IUnknown> {} ;
    template<> struct comtype< ::IViewObject2>
        : public uuid_comtype< ::IViewObject2,&IID_IViewObject2, ::IViewObject> {} ;
    template<> struct comtype< ::IDropSource>
        : public uuid_comtype< ::IDropSource,&IID_IDropSource, ::IUnknown> {} ;
    template<> struct comtype< ::IDropTarget>
        : public uuid_comtype< ::IDropTarget,&IID_IDropTarget, ::IUnknown> {} ;
    template<> struct comtype< ::IEnumOLEVERB>
        : public uuid_comtype< ::IEnumOLEVERB,&IID_IEnumOLEVERB, ::IUnknown> {} ;
};

#endif //COMET_OLEIDL_COMTYPES_H
