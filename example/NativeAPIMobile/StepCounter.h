
#ifndef __NATIVEAPISTEPCOUNTER_H__
#define __NATIVEAPISTEPCOUNTER_H__

#include "ConversionWchar.h"
#include <time.h>
#include <string>
#include "../../include/mobile.h"

#if defined(__OBJC__)

@class CStepCounterRecorder;

#endif //__OBJC__

class IAddInDefBaseEx;

class IMemoryManager;

#if defined(__APPLE__) && !defined(BUILD_DYNAMIC_LIBRARY)

namespace COM_1C_STEP_COUNTER 
{
    
#endif //__APPLE__ && !BUILD_DYNAMIC_LIBRARY

static const wchar_t g_kClassNames[] = L"StepCounter";
static const wchar_t g_eventChangeName[] = L"OnChange";
static const wchar_t g_paramEvent[] = L"_";
static const wchar_t g_ClassStepCounter[] = L"StepCounter";

static WcharWrapper s_kClassNames(g_kClassNames);
static WcharWrapper s_eventChangeName(g_eventChangeName);
static WcharWrapper s_paramEvent(g_paramEvent);
static WcharWrapper s_classNameStepCounter(g_ClassStepCounter);

#if defined(__APPLE__) 

#if !defined(__OBJC__)

    class CStepCounterRecorder;
    
#endif //!__OBJC__
    
#elif defined(MOBILE_PLATFORM_WINRT) //__APPLE__

    ref class CStepCounterRecorder;
    
#else //MOBILE_PLATFORM_WINRT

    class CStepCounterRecorder;
    
#endif //!__APPLE__ && !MOBILE_PLATFORM_WINRT

    enum Orientations
    {
        eOrientXOZ = 0,
        eOrientXOY,
        eOrientYOZ,
        eOrientLast      // Always last
    };
    
    enum TypesMotion
    {
        eTypeUndefined = 0,
        eTypeStationary,
        eTypeWalking,
        eTypeRunning,
        eTypeAutomotive,
        eTypeCycling,
        eTypeLast        // Always last
    };

    struct MotionData
    {
        MotionData():
            startDate(time(NULL)),
            orientXOZ(0.0), orientXOY(0.0), orientYOZ(0.0),
            typeMotion(eTypeUndefined), stepCount(0), isComplete(false) {}
        time_t      startDate;
        double      orientXOZ;
        double      orientXOY;
        double      orientYOZ;
        TypesMotion typeMotion;
        long        stepCount;
        bool        isComplete;        
    };

    class CStepCounter
    {
 
    public:
        CStepCounter();
        ~CStepCounter();
    
        void startStepCounterUpdatesFromDate();
        void stopStepCounterUpdates();
        long stepCount();
        TypesMotion typeMotion();
        double orientation(const Orientations plane);
        void setIConnect(IAddInDefBaseEx* piConnect);
        bool getRunningStepCounterUpdates();
        std::wstring getLastErrorDescr();
    private:
        MotionData          sharedData;
        IAddInDefBaseEx*    piCnt;
        bool                isRun;
        std::wstring        lastErrorDescr;
        
#if defined(MOBILE_PLATFORM_WINRT)

        CStepCounterRecorder^ motionRecorder;
        
#else //MOBILE_PLATFORM_WINRT
    
        CStepCounterRecorder* motionRecorder;
        
#endif //!MOBILE_PLATFORM_WINRT

    };
    
#if defined(__APPLE__) && !defined(BUILD_DYNAMIC_LIBRARY)

};

#endif //__APPLE__ && !BUILD_DYNAMIC_LIBRARY
    
#endif //__NATIVEAPISTEPCOUNTER_H__

