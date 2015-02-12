#ifndef camera_helper_h__
#define camera_helper_h__

#pragma once

#include <vector>

#include <cv.h>
#include <highgui.h>
#include <opencv2/imgcodecs.hpp>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include <DShow.h>

#include <comet/ptr.h>

#include <comet_task_ptr.h>
#include <comet_com_ptr_array.h>

namespace comet {

#define MAKE_COMET_COMTYPE(T, BASE) \
    template<> struct comtype< ::T > \
    { \
        static const IID& uuid() { return IID_ ## T; } \
        typedef ::BASE base; \
    }

    MAKE_COMET_COMTYPE(IMF2DBuffer, IUnknown);
    MAKE_COMET_COMTYPE(IAMVideoProcAmp, IUnknown);
    MAKE_COMET_COMTYPE(IAMCameraControl, IUnknown);
}

struct cam_prop_range
{
    long min;
    long max;
    long step;
    long defaultValue;
    bool canBeAuto;
    bool canBeManual;
};

inline std::ostream& operator<<(std::ostream& os, const cam_prop_range& val)
{
    os << "[" << val.min << ":" << val.step << ":" << val.max << "], " << val.defaultValue;
    if (val.canBeAuto && val.canBeManual)
        os << " (auto/manual)";
    else if (val.canBeAuto)
        os << " (auto)";
    else if (val.canBeManual)
        os << " (manual)";
    return os;
}

struct cam_prop_value
{
    long value;
    bool isAuto;

    cam_prop_value() : value(0), isAuto(true)
    {
    }

    cam_prop_value(long value, bool isAuto = false) : value(value), isAuto(isAuto)
    {
    }

    operator long() const
    {
        return value;
    }

    static cam_prop_value AUTO(long value = 0) {
        return cam_prop_value(value, true);
    };
    static cam_prop_value MANUAL(long value = 0) {
        return cam_prop_value(value, false);
    };
};

inline std::ostream& operator<<(std::ostream& os, const cam_prop_value& val)
{
    os << val.value;
    os << " (";
    if (val.isAuto)
        os << "auto";
    else
        os << "manual";
    os << ")";
    return os;
}

namespace MediaFormat
{
    enum e
    {
        Unknown,
        RGB32, // MFVideoFormat_RGB32
        ARGB32, // MFVideoFormat_ARGB32
        RGB24, // MFVideoFormat_RGB24
        RGB555, // MFVideoFormat_RGB555
        RGB565, // MFVideoFormat_RGB565
        RGB8, // MFVideoFormat_RGB8
        AI44, // MFVideoFormat_AI44
        AYUV, // MFVideoFormat_AYUV
        YUY2, // MFVideoFormat_YUY2
        YVYU, // MFVideoFormat_YVYU
        YVU9, // MFVideoFormat_YVU9
        UYVY, // MFVideoFormat_UYVY
        NV11, // MFVideoFormat_NV11
        NV12, // MFVideoFormat_NV12
        YV12, // MFVideoFormat_YV12
        I420, // MFVideoFormat_I420
        IYUV, // MFVideoFormat_IYUV
        Y210, // MFVideoFormat_Y210
        Y216, // MFVideoFormat_Y216
        Y410, // MFVideoFormat_Y410
        Y416, // MFVideoFormat_Y416
        Y41P, // MFVideoFormat_Y41P
        Y41T, // MFVideoFormat_Y41T
        Y42T, // MFVideoFormat_Y42T
        P210, // MFVideoFormat_P210
        P216, // MFVideoFormat_P216
        P010, // MFVideoFormat_P010
        P016, // MFVideoFormat_P016
        v210, // MFVideoFormat_v210
        v216, // MFVideoFormat_v216
        v410, // MFVideoFormat_v410
        MP43, // MFVideoFormat_MP43
        MP4S, // MFVideoFormat_MP4S
        M4S2, // MFVideoFormat_M4S2
        MP4V, // MFVideoFormat_MP4V
        WMV1, // MFVideoFormat_WMV1
        WMV2, // MFVideoFormat_WMV2
        WMV3, // MFVideoFormat_WMV3
        WVC1, // MFVideoFormat_WVC1
        MSS1, // MFVideoFormat_MSS1
        MSS2, // MFVideoFormat_MSS2
        MPG1, // MFVideoFormat_MPG1
        DVSL, // MFVideoFormat_DVSL
        DVSD, // MFVideoFormat_DVSD
        DVHD, // MFVideoFormat_DVHD
        DV25, // MFVideoFormat_DV25
        DV50, // MFVideoFormat_DV50
        DVH1, // MFVideoFormat_DVH1
        DVC, // MFVideoFormat_DVC
        H264, // MFVideoFormat_H264
        MJPG, // MFVideoFormat_MJPG
        YUV_420O, // MFVideoFormat_420O
        H263, // MFVideoFormat_H263
        H264_ES, // MFVideoFormat_H264_ES
        MPEG2, // MFVideoFormat_MPEG2
    };

    inline e fromGUID(GUID guid)
    {
        if (guid == MFVideoFormat_RGB32) return RGB32;
        else if (guid == MFVideoFormat_ARGB32) return ARGB32;
        else if (guid == MFVideoFormat_RGB24) return RGB24;
        else if (guid == MFVideoFormat_RGB555) return RGB555;
        else if (guid == MFVideoFormat_RGB565) return RGB565;
        else if (guid == MFVideoFormat_RGB8) return RGB8;
        else if (guid == MFVideoFormat_AI44) return AI44;
        else if (guid == MFVideoFormat_AYUV) return AYUV;
        else if (guid == MFVideoFormat_YUY2) return YUY2;
        else if (guid == MFVideoFormat_YVYU) return YVYU;
        else if (guid == MFVideoFormat_YVU9) return YVU9;
        else if (guid == MFVideoFormat_UYVY) return UYVY;
        else if (guid == MFVideoFormat_NV11) return NV11;
        else if (guid == MFVideoFormat_NV12) return NV12;
        else if (guid == MFVideoFormat_YV12) return YV12;
        else if (guid == MFVideoFormat_I420) return I420;
        else if (guid == MFVideoFormat_IYUV) return IYUV;
        else if (guid == MFVideoFormat_Y210) return Y210;
        else if (guid == MFVideoFormat_Y216) return Y216;
        else if (guid == MFVideoFormat_Y410) return Y410;
        else if (guid == MFVideoFormat_Y416) return Y416;
        else if (guid == MFVideoFormat_Y41P) return Y41P;
        else if (guid == MFVideoFormat_Y41T) return Y41T;
        else if (guid == MFVideoFormat_Y42T) return Y42T;
        else if (guid == MFVideoFormat_P210) return P210;
        else if (guid == MFVideoFormat_P216) return P216;
        else if (guid == MFVideoFormat_P010) return P010;
        else if (guid == MFVideoFormat_P016) return P016;
        else if (guid == MFVideoFormat_v210) return v210;
        else if (guid == MFVideoFormat_v216) return v216;
        else if (guid == MFVideoFormat_v410) return v410;
        else if (guid == MFVideoFormat_MP43) return MP43;
        else if (guid == MFVideoFormat_MP4S) return MP4S;
        else if (guid == MFVideoFormat_M4S2) return M4S2;
        else if (guid == MFVideoFormat_MP4V) return MP4V;
        else if (guid == MFVideoFormat_WMV1) return WMV1;
        else if (guid == MFVideoFormat_WMV2) return WMV2;
        else if (guid == MFVideoFormat_WMV3) return WMV3;
        else if (guid == MFVideoFormat_WVC1) return WVC1;
        else if (guid == MFVideoFormat_MSS1) return MSS1;
        else if (guid == MFVideoFormat_MSS2) return MSS2;
        else if (guid == MFVideoFormat_MPG1) return MPG1;
        else if (guid == MFVideoFormat_DVSL) return DVSL;
        else if (guid == MFVideoFormat_DVSD) return DVSD;
        else if (guid == MFVideoFormat_DVHD) return DVHD;
        else if (guid == MFVideoFormat_DV25) return DV25;
        else if (guid == MFVideoFormat_DV50) return DV50;
        else if (guid == MFVideoFormat_DVH1) return DVH1;
        else if (guid == MFVideoFormat_DVC) return DVC;
        else if (guid == MFVideoFormat_H264) return H264;
        else if (guid == MFVideoFormat_MJPG) return MJPG;
        else if (guid == MFVideoFormat_420O) return YUV_420O;
        else if (guid == MFVideoFormat_H263) return H263;
        else if (guid == MFVideoFormat_H264_ES) return H264_ES;
        else if (guid == MFVideoFormat_MPEG2) return MPEG2;
        else return Unknown;
    }

    inline const char* to_string (const MediaFormat::e& format)
    {
        switch (format)
        {
        case RGB32: return "RGB32";
        case ARGB32: return "ARGB32";
        case RGB24: return "RGB24";
        case RGB555: return "RGB555";
        case RGB565: return "RGB565";
        case RGB8: return "RGB8";
        case AI44: return "AI44";
        case AYUV: return "AYUV";
        case YUY2: return "YUY2";
        case YVYU: return "YVYU";
        case YVU9: return "YVU9";
        case UYVY: return "UYVY";
        case NV11: return "NV11";
        case NV12: return "NV12";
        case YV12: return "YV12";
        case I420: return "I420";
        case IYUV: return "IYUV";
        case Y210: return "Y210";
        case Y216: return "Y216";
        case Y410: return "Y410";
        case Y416: return "Y416";
        case Y41P: return "Y41P";
        case Y41T: return "Y41T";
        case Y42T: return "Y42T";
        case P210: return "P210";
        case P216: return "P216";
        case P010: return "P010";
        case P016: return "P016";
        case v210: return "v210";
        case v216: return "v216";
        case v410: return "v410";
        case MP43: return "MP43";
        case MP4S: return "MP4S";
        case M4S2: return "M4S2";
        case MP4V: return "MP4V";
        case WMV1: return "WMV1";
        case WMV2: return "WMV2";
        case WMV3: return "WMV3";
        case WVC1: return "WVC1";
        case MSS1: return "MSS1";
        case MSS2: return "MSS2";
        case MPG1: return "MPG1";
        case DVSL: return "DVSL";
        case DVSD: return "DVSD";
        case DVHD: return "DVHD";
        case DV25: return "DV25";
        case DV50: return "DV50";
        case DVH1: return "DVH1";
        case DVC: return "DVC";
        case H264: return "H264";
        case MJPG: return "MJPG";
        case YUV_420O: return "420O";
        case H263: return "H263";
        case H264_ES: return "H264_ES";
        case MPEG2: return "MPEG2";
        default: return "Unknown";
        }
    }

    inline std::ostream& operator<< (std::ostream& os, const MediaFormat::e& format)
    {
        return os << to_string(format);
    }
}

class media_type
{
public:
    media_type() : _ptr(NULL)
    {
    }

    media_type(IMFMediaType* ptr) : _ptr(ptr)
    {
    }

    media_type(comet::com_ptr<IMFMediaType> ptr) : _ptr(ptr)
    {
    }

    cv::Size resolution() const
    {
        UINT32 width, height;
        MFGetAttributeSize(_ptr.in(), MF_MT_FRAME_SIZE, &width, &height) | comet::raise_exception;
        return cv::Size(width, height);
    }

    double framerate() const
    {
        UINT32 num, denom;
        MFGetAttributeRatio(_ptr.in(), MF_MT_FRAME_RATE, &num, &denom) | comet::raise_exception;
        return static_cast<double>(num) / denom;
    }

    MediaFormat::e format() const
    {
        GUID subtype;
        _ptr->GetGUID(MF_MT_SUBTYPE, &subtype) | comet::raise_exception;
        return MediaFormat::fromGUID(subtype);
    }

#ifdef _TYPEDEF_BOOL_TYPE
    typedef media_type _Myt;
    _TYPEDEF_BOOL_TYPE;
    _OPERATOR_BOOL() const _NOEXCEPT
    {    // test for non-null pointer
    return (_ptr != 0 ? _CONVERTIBLE_TO_TRUE : 0);
    }
#else
    explicit operator bool() const
    { // test for non-null pointer
        return _ptr != 0;
    }
#endif

    comet::com_ptr<IMFMediaType> _ptr;
};


class camera
{
public:
    camera()
    {
    }

    camera(IMFActivate* ptr) : activate_ptr(ptr)
    {
    }

    camera(comet::com_ptr<IMFActivate> ptr) : activate_ptr(ptr)
    {
    }

    //camera(const camera& other) = delete;

    camera(camera&& other)
    {
        *this = std::move(other);
    }

    //camera& operator=(const camera& other) = delete;

    camera& operator=(camera&& other)
    {
        activate_ptr.swap(other.activate_ptr);
        source_ptr.swap(other.source_ptr);
        reader_ptr.swap(other.reader_ptr);
        return *this;
    }

    ~camera()
    {
        shutdown();
    }

    bool is_active() const
    {
        return !source_ptr.is_null();
    }

    void activate()
    {
        if (activate_ptr)
            activate_ptr->ActivateObject(IID_IMFMediaSource, reinterpret_cast<void**>(source_ptr.out())) | comet::raise_exception;
        reader_ptr = NULL;
    }

    void shutdown()
    {
        if (activate_ptr)
            activate_ptr->ShutdownObject() | comet::raise_exception;
        source_ptr = NULL;
        reader_ptr = NULL;
    }

    cv::Mat read_frame(int streamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM, int bufferIndex = 0)
    {
        if (!activate_ptr)
            return cv::Mat();

        if (!reader_ptr)
        {
            comet::com_ptr<IMFAttributes> pAttributes;
            MFCreateAttributes(pAttributes.out(), 1) | comet::raise_exception;
            pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE) | comet::raise_exception;
            pAttributes->SetUINT32(MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN, TRUE) | comet::raise_exception;

            MFCreateSourceReaderFromMediaSource(source_ptr.in(), pAttributes.in(), reader_ptr.out()) | comet::raise_exception;
        }

        comet::com_ptr<IMFSample> sample;
        DWORD actualStreamIndex, flags;
        LONGLONG timestamp;
        try
        {
            do
            {
                reader_ptr->ReadSample(
                    streamIndex, // Stream index.
                    0, // Flags.
                    &actualStreamIndex, // Receives the actual stream index.
                    &flags, // Receives status flags.
                    &timestamp, // Receives the time stamp.
                    sample.out() // Receives the sample or NULL.
                ) | comet::raise_exception;
            } while (sample == NULL && (flags & MF_SOURCE_READERF_STREAMTICK));
        }
        catch (std::exception& e)
        {
            std::cerr << "Error getting frame: " << e.what() << std::endl;
            std::cerr << "              flags: " << flags << std::endl;
            shutdown();
            activate();
            throw;
        }

        media_type cur_media_type;
        reader_ptr->GetCurrentMediaType(actualStreamIndex, cur_media_type._ptr.out()) | comet::raise_exception;

        //PrintAttributes(cur_media_type._ptr.in());

        auto format = cur_media_type.format();

        cv::Mat ret;

        DWORD bufCount;
        sample->GetBufferCount(&bufCount) | comet::raise_exception;

        DWORD bufIndex = bufferIndex >= 0 ? bufferIndex : bufCount - bufferIndex;

        if (bufCount > bufIndex)
        {
            comet::com_ptr<IMFMediaBuffer> buffer;
            sample->GetBufferByIndex(bufferIndex, buffer.out()) | comet::raise_exception;

            switch (format)
            {
            case MediaFormat::RGB24:
            case MediaFormat::ARGB32:
            case MediaFormat::RGB32:
                {
                    comet::com_ptr<IMF2DBuffer> buf2d = try_cast(buffer);

                    //DWORD length;
                    //buf2d->GetContiguousLength(&length) | comet::raise_exception;

                    //ret.create();

                    //COMET_ASSERT(ret.dataend - ret.datastart == length);

                    auto resolution = cur_media_type.resolution();

                    struct buf2d_lock
                    {
                        comet::com_ptr<IMF2DBuffer>& buf2d;

                        buf2d_lock(comet::com_ptr<IMF2DBuffer>& buf2d, BYTE*& scanline0, LONG& pitch) : buf2d(buf2d)
                        {
                            buf2d->Lock2D(&scanline0, &pitch) | comet::raise_exception;
                        }

                        ~buf2d_lock()
                        {
                            buf2d->Unlock2D() | comet::raise_exception;
                        }
                    };

                    BYTE *scanline0;
                    LONG pitch;
                    buf2d_lock buf_lock_token(buf2d, scanline0, pitch);
                    if (pitch >= 0)
                    {
                        cv::Mat buf2dmat(resolution,
                            (format == MediaFormat::RGB24) ? CV_8UC3 : CV_8UC4,
                            scanline0,
                            pitch);
                        buf2dmat.copyTo(ret);
                    }
                    else
                    {
                        cv::Mat buf2dmat(resolution,
                            (format == MediaFormat::RGB24) ? CV_8UC3 : CV_8UC4,
                            scanline0 + pitch*(resolution.height-1),
                            -pitch);
                        cv::flip(buf2dmat, ret, 0);
                    }

                    break;
                }
            case MediaFormat::MJPG:
                {
                    struct buf_lock
                    {
                        comet::com_ptr<IMFMediaBuffer>& buffer;

                        buf_lock(comet::com_ptr<IMFMediaBuffer>& buffer, BYTE*& data, DWORD& maxLength, DWORD& length) : buffer(buffer)
                        {
                            buffer->Lock(&data, &maxLength, &length) | comet::raise_exception;
                        }

                        ~buf_lock()
                        {
                            buffer->Unlock() | comet::raise_exception;
                        }
                    };

                    BYTE* data;
                    DWORD length;
                    DWORD maxLength;

                    buf_lock buf_lock_token(buffer, data, maxLength, length);

                    ret = cv::imdecode(cv::Mat(length, 1, CV_8U, data), cv::IMREAD_COLOR);

                    break;
                }
            default:
				std::stringstream sstream;
				sstream << "Unknown media format: " << format;
				throw std::runtime_error(sstream.str());
            }
        }

        return ret;
    }

    std::string name() const
    {
        return get_attr_str(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME);
    }

    std::string symlink() const
    {
        return get_attr_str(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK);
    }

	// TODO change
    //explicit operator bool() const
    //{
    //    return !activate_ptr.is_null();
    //}

    enum Property
    {
        // CameraControl
        Exposure,
        Focus,
        Zoom,
        Pan,
        Tilt,
        Roll,
        Iris,

        // VideoProcAmp
        Brightness,
        Contrast,
        Hue,
        Saturation,
        Sharpness,
        Gamma,
        ColorEnable,
        WhiteBalance,
        BacklightCompensation,
        Gain
    };

    bool has(Property property) const;
    cam_prop_range get_range(Property property) const;
    cam_prop_value get(Property property) const;
    void set(Property property, const cam_prop_value& value);

    static std::vector<Property> list_properties();
    static const char* property_name(Property);
     
    std::vector<media_type> media_types(int streamIndex = 0) const
    {
        auto pHandler = getMediaTypeHandler(streamIndex);

        DWORD cTypes = 0;
        pHandler->GetMediaTypeCount(&cTypes) | comet::raise_exception;

        std::vector<media_type> ret;
        for (DWORD i = 0; i < cTypes; i++)
        {
            comet::com_ptr<IMFMediaType> pType;
            pHandler->GetMediaTypeByIndex(i, pType.out()) | comet::raise_exception;
            ret.emplace_back(pType);
        }

        return ret;
    }

    media_type get_media_type(int streamIndex = 0)
    {
        media_type ret;
        getMediaTypeHandler(streamIndex)->GetCurrentMediaType(ret._ptr.out()) | comet::raise_exception;
        return ret;
    }

    void set_media_type(const media_type& type, int streamIndex = 0)
    {
        getMediaTypeHandler(streamIndex)->SetCurrentMediaType(type._ptr.in()) | comet::raise_exception;
        if (reader_ptr)
        {
            reader_ptr->SetCurrentMediaType(streamIndex, nullptr, type._ptr.in()) | comet::raise_exception;
        }
    }

	// TODO change
    //explicit operator bool() {
    //    return !activate_ptr.is_null();
    //}

private:
    std::string get_attr_str(REFGUID guid) const
    {
        comet::task_ptr<WCHAR> pStr;
        UINT32 strLen;
        activate_ptr->GetAllocatedString(guid, pStr.out(), &strLen) | comet::raise_exception;
        return comet::bstr_t(pStr.in(), strLen).s_str();
    }

    comet::com_ptr<IMFMediaTypeHandler> getMediaTypeHandler(int streamIndex = 0) const
    {
        comet::com_ptr<IMFPresentationDescriptor> pPD;
        source_ptr->CreatePresentationDescriptor(pPD.out()) | comet::raise_exception;

        BOOL fSelected;
        comet::com_ptr<IMFStreamDescriptor> pSD;
        pPD->GetStreamDescriptorByIndex(streamIndex, &fSelected, pSD.out()) | comet::raise_exception;

        comet::com_ptr<IMFMediaTypeHandler> pHandler;
        pSD->GetMediaTypeHandler(pHandler.out()) | comet::raise_exception;

        return pHandler;
    }

    comet::com_ptr<IMFActivate> activate_ptr;
    comet::com_ptr<IMFMediaSource> source_ptr;
    comet::com_ptr<IMFSourceReader> reader_ptr;
};

class camera_helper
{
public:
    static std::vector<camera> get_all_cameras()
    {
        comet::com_ptr<IMFAttributes> config;
        MFCreateAttributes(config.out(), 1) | comet::raise_exception;

        config->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID) | comet::raise_exception;

        comet::com_ptr_array<IMFActivate> com_ptr_array;
        MFEnumDeviceSources(config.in(), com_ptr_array.out(), com_ptr_array.out_count()) | comet::raise_exception;

        std::vector<camera> ret;
        for (size_t i = 0; i < com_ptr_array.count(); ++i)
        {
            ret.emplace_back(com_ptr_array[i]);
        }
        return ret;
    }

    static camera get_camera_by_symlink(const std::string& symlink)
    {
        // This is how you should do it, but for some reason it gives an activate pointer with no friendly name

        //         comet::com_ptr<IMFAttributes> config;
        //         MFCreateAttributes(config.out(), 1) | comet::raise_exception;
        // 
        //         config->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID) | comet::raise_exception;
        //         comet::bstr_t symlink_bstr(symlink);
        //         config->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, symlink_bstr.c_str()) | comet::raise_exception;
        // 
        //         comet::com_ptr<IMFActivate> activate_ptr;
        //         MFCreateDeviceSourceActivate(config.in(), activate_ptr.out()) | comet::raise_exception;
        // 
        //         return camera(activate_ptr);

        for(auto&& camera : get_all_cameras())
        {
            if (camera.symlink() == symlink)
                return std::move(camera);
        }

		throw std::runtime_error("No camera with symlink: " + std::string(symlink));
    }
};

#endif // camera_helper_h__
