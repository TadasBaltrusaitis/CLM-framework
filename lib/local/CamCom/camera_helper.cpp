#include "stdafx.h"

#include "camera_helper.h"

//#include <format.h>

namespace {
    namespace internal {
        template<camera::Property property> struct property_traits {};

        template<long Id>
        struct camera_control_property_traits
        {
            typedef IAMCameraControl Interface;
            static long id() { return Id; }
        };

        template<long Id>
        struct video_proc_amp_property_traits
        {
            typedef IAMVideoProcAmp Interface;
            static long id() { return Id; }
        };

        template<> struct property_traits < camera::Exposure > : public camera_control_property_traits<CameraControl_Exposure> {};
        template<> struct property_traits < camera::Focus > : public camera_control_property_traits<CameraControl_Focus>{};
        template<> struct property_traits < camera::Zoom > : public camera_control_property_traits<CameraControl_Zoom>{};
        template<> struct property_traits < camera::Pan > : public camera_control_property_traits<CameraControl_Pan>{};
        template<> struct property_traits < camera::Tilt > : public camera_control_property_traits<CameraControl_Tilt>{};
        template<> struct property_traits < camera::Roll > : public camera_control_property_traits<CameraControl_Roll>{};
        template<> struct property_traits < camera::Iris > : public camera_control_property_traits<CameraControl_Iris>{};

        template<> struct property_traits < camera::Brightness > : public video_proc_amp_property_traits<VideoProcAmp_Brightness>{};
        template<> struct property_traits < camera::Contrast > : public video_proc_amp_property_traits<VideoProcAmp_Contrast>{};
        template<> struct property_traits < camera::Hue > : public video_proc_amp_property_traits<VideoProcAmp_Hue>{};
        template<> struct property_traits < camera::Saturation > : public video_proc_amp_property_traits<VideoProcAmp_Saturation>{};
        template<> struct property_traits < camera::Sharpness > : public video_proc_amp_property_traits<VideoProcAmp_Sharpness>{};
        template<> struct property_traits < camera::Gamma > : public video_proc_amp_property_traits<VideoProcAmp_Gamma>{};
        template<> struct property_traits < camera::ColorEnable > : public video_proc_amp_property_traits<VideoProcAmp_ColorEnable>{};
        template<> struct property_traits < camera::WhiteBalance > : public video_proc_amp_property_traits<VideoProcAmp_WhiteBalance>{};
        template<> struct property_traits < camera::BacklightCompensation > : public video_proc_amp_property_traits<VideoProcAmp_BacklightCompensation>{};
        template<> struct property_traits < camera::Gain > : public video_proc_amp_property_traits < VideoProcAmp_Gain > {};

        template<typename Interface> struct interface_traits {};
        template<> struct interface_traits < IAMCameraControl >
        {
            static long auto_flag() { return CameraControl_Flags_Auto; }
            static long manual_flag() { return CameraControl_Flags_Manual; }
        };
        template<> struct interface_traits<IAMVideoProcAmp>
        {
            static long auto_flag() { return VideoProcAmp_Flags_Auto; }
            static long manual_flag() { return VideoProcAmp_Flags_Manual; }
        };

        template<typename Interface, typename SourcePtr>
        bool has(long id, const SourcePtr& source_ptr) {
            comet::com_ptr<Interface> pInterface = comet::com_cast(source_ptr);
            if (!pInterface) return false;
            long value, flags;
            HRESULT hr = pInterface->Get(id, &value, &flags);
            return SUCCEEDED(hr);
        }

        template<typename Interface, typename SourcePtr>
        cam_prop_range get_range(long id, const SourcePtr& source_ptr) {
            comet::com_ptr<Interface> pInterface = comet::try_cast(source_ptr);
            cam_prop_range range;
            long flags;
            pInterface->GetRange(id, &range.min, &range.max, &range.step, &range.defaultValue, &flags) | comet::raise_exception;
            range.canBeAuto = (flags & interface_traits<Interface>::auto_flag()) != 0;
            range.canBeManual = (flags & interface_traits<Interface>::manual_flag()) != 0;
            return range;
        }

        template<typename Interface, typename SourcePtr>
        cam_prop_value get(long id, const SourcePtr& source_ptr) {
            comet::com_ptr<Interface> pInterface = comet::try_cast(source_ptr);
            cam_prop_value value;
            long flags;
            pInterface->Get(id, &value.value, &flags) | comet::raise_exception;
            value.isAuto = (flags & interface_traits<Interface>::auto_flag()) != 0 || (flags & interface_traits<Interface>::manual_flag()) == 0;
            return value;
        }

        template<typename Interface, typename SourcePtr>
        void set(long id, const cam_prop_value& value, const SourcePtr& source_ptr) {
            comet::com_ptr<Interface> pInterface = comet::try_cast(source_ptr);
            long flags = value.isAuto ? interface_traits<Interface>::auto_flag() : interface_traits<Interface>::manual_flag();
            pInterface->Set(id, value.value, flags) | comet::raise_exception;
        }
    }
}

#define MAP_OVER_CAMERA_PROPERTIES(FUNC) \
    FUNC(::camera::Exposure, Exposure) \
    FUNC(::camera::Focus, Focus) \
    FUNC(::camera::Zoom, Zoom) \
    FUNC(::camera::Pan, Pan) \
    FUNC(::camera::Tilt, Tilt) \
    FUNC(::camera::Roll, Roll) \
    FUNC(::camera::Iris, Iris) \
    FUNC(::camera::Brightness, Brightness) \
    FUNC(::camera::Contrast, Contrast) \
    FUNC(::camera::Hue, Hue) \
    FUNC(::camera::Saturation, Saturation) \
    FUNC(::camera::Sharpness, Sharpness) \
    FUNC(::camera::Gamma, Gamma) \
    FUNC(::camera::ColorEnable, Color Enable) \
    FUNC(::camera::WhiteBalance, White Balance) \
    FUNC(::camera::BacklightCompensation, Backlight Compensation) \
    FUNC(::camera::Gain, Gain)


bool camera::has(Property property) const
{
    switch (property) {
#define CASE(PROP, NAME) case PROP: return ::internal::has<::internal::property_traits<PROP>::Interface>(::internal::property_traits<PROP>::id(), source_ptr);
        MAP_OVER_CAMERA_PROPERTIES(CASE)
#undef CASE
    default:
        return false;
    }
}
cam_prop_range camera::get_range(Property property) const
{
    switch (property) {
#define CASE(PROP, NAME) case PROP: return ::internal::get_range<::internal::property_traits<PROP>::Interface>(::internal::property_traits<PROP>::id(), source_ptr);
        MAP_OVER_CAMERA_PROPERTIES(CASE)
#undef CASE
    default:
        throw std::runtime_error("No such property: " + std::string(camera::property_name(property)));
    }
}
cam_prop_value camera::get(Property property) const
{
    switch (property) {
#define CASE(PROP, NAME) case PROP: return ::internal::get<::internal::property_traits<PROP>::Interface>(::internal::property_traits<PROP>::id(), source_ptr);
        MAP_OVER_CAMERA_PROPERTIES(CASE)
#undef CASE
    default:
        throw std::runtime_error("No such property: " + std::string(camera::property_name(property)));
    }
}
void camera::set(Property property, const cam_prop_value& value)
{
    switch (property) {
#define CASE(PROP, NAME) case PROP: ::internal::set<::internal::property_traits<PROP>::Interface>(::internal::property_traits<PROP>::id(), value, source_ptr); break;
        MAP_OVER_CAMERA_PROPERTIES(CASE)
#undef CASE
    }
}

std::vector<camera::Property> camera::list_properties()
{
	std::vector<camera::Property> properties;
	properties.push_back(Exposure);   // CameraControl
    properties.push_back(Focus);
    properties.push_back(Zoom);
    properties.push_back(Pan);
    properties.push_back(Tilt);
    properties.push_back(Roll);
    properties.push_back(Iris);

    properties.push_back(Brightness);
    properties.push_back(Contrast);
    properties.push_back(Hue);
    properties.push_back(Saturation);
    properties.push_back(Sharpness);
    properties.push_back(Gamma);
    properties.push_back(ColorEnable);
    properties.push_back(WhiteBalance);
    properties.push_back(BacklightCompensation);
    properties.push_back(Gain);

    return properties;
//#define PROPENTRY(PROP, NAME) PROP,
//        MAP_OVER_CAMERA_PROPERTIES(PROPENTRY)
//#undef PROPENTRY
//    };
}

const char* camera::property_name(Property property)
{
    switch (property) {
#define CASE(PROP, NAME) case PROP: return #NAME;
        MAP_OVER_CAMERA_PROPERTIES(CASE)
#undef CASE
    default:
        return "UNKNOWN";
    }
}
