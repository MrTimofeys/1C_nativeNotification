
#include "StepCounter.h"
#include "ConversionWchar.h"
#include "../../include/AddInDefBase.h"

#if defined(MOBILE_PLATFORM_WINRT)

#include <vector>
#include <math.h>
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::Devices::Sensors;

#elif defined(__ANDROID__) //MOBILE_PLATFORM_WINRT

#include "../include/IAndroidComponentHelper.h"
#include "jnienv.h"
#include <jni.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#endif //__ANDROID__   

#if defined(__ANDROID__)

std::wstring jstring2wstring(JNIEnv* jenv, jstring aStr)
{
    std::wstring result;

    if (aStr)
    {        
        const jchar *pCh = jenv->GetStringChars(aStr, 0);
        jsize len = jenv->GetStringLength(aStr);
        const jchar *temp = pCh;
        while (len > 0)
        {
            result += *(temp++);
            --len;
        }
        jenv->ReleaseStringChars(aStr, pCh);
    }
    return result;
}

class CStepCounterRecorder
{
private:
    IAndroidComponentHelper*    helper;
    jclass                      cc;
    jobject                     obj;
    IAddInDefBaseEx*            cnn;
    MotionData*                 shrdData;
    bool                        isChangedData;
    jmethodID   methodID_resetCounter;
    jmethodID   methodID_getStepCount;
    jmethodID   methodID_getOrientXOZ;
    jmethodID   methodID_getOrientXOY;
    jmethodID   methodID_getOrientYOZ;
    jmethodID   methodID_clearObjectsReference;
    jmethodID   methodID_getErrorDescr;
public:

    CStepCounterRecorder(IAddInDefBaseEx *m_iConnect, MotionData* sharedData): cc(0), obj(0), shrdData(sharedData), isChangedData(false)
    {
        shrdData->isComplete = false;
        shrdData->stepCount = 0;
        shrdData->typeMotion = eTypeUndefined;
        shrdData->startDate = time(NULL);
        
        cnn = m_iConnect;
        cnn->SetEventBufferDepth(10);
        helper = (IAndroidComponentHelper*)cnn->GetInterface(eIAndroidComponentHelper);
        if (helper)
        {
            WCHAR_T* className = 0;
            convToShortWchar(&className, L"com/stepCounterPackage/stepCounterLib/StepCounterClass");
            jclass ccloc = helper->FindClass(className);
            delete[] className;
            className = 0;

            if (ccloc)
            {
                JNIEnv* jenv = getJniEnv();
                cc = static_cast<jclass>(jenv->NewGlobalRef(ccloc));
                jenv->DeleteLocalRef(ccloc);
                jobject activity = helper->GetActivity();
                jmethodID ctorID = jenv->GetMethodID(cc, "<init>", "(Landroid/app/Activity;J)V");
                jobject objloc = jenv->NewObject(cc, ctorID, activity, (jlong)this);
                if (objloc)
                {
                    obj = jenv->NewGlobalRef(objloc);
                    jenv->DeleteLocalRef(objloc);
                }
                jenv->DeleteLocalRef(activity);
                
                // Run native
                jmethodID methodID_show = jenv->GetMethodID(cc, "show", "()V");
                jenv->CallVoidMethod(obj, methodID_show);

                // Methods
                methodID_resetCounter = jenv->GetMethodID(cc, "resetCounter", "()V");
                methodID_getStepCount = jenv->GetMethodID(cc, "getStepCount", "()I");
                methodID_getOrientXOZ = jenv->GetMethodID(cc, "getOrientXOZ", "()F");
                methodID_getOrientXOY = jenv->GetMethodID(cc, "getOrientXOY", "()F");
                methodID_getOrientYOZ = jenv->GetMethodID(cc, "getOrientYOZ", "()F");
                methodID_clearObjectsReference = jenv->GetMethodID(cc, "clearObjectsReference", "()V");
                methodID_getErrorDescr = jenv->GetMethodID(cc, "getErrorDescr", "()Ljava/lang/String;");
            }
        }
    }

    ~CStepCounterRecorder()
    {
        JNIEnv* jenv = getJniEnv();
        jenv->CallVoidMethod(obj, methodID_clearObjectsReference);
        stopStepCounterUpdates();
        jenv->DeleteGlobalRef(obj);
        jenv->DeleteGlobalRef(cc);
    }

    void updateAllData(double orientXOZ, double orientXOY, double orientYOZ,
                       long stepCount)
    {
        shrdData->orientXOZ = orientXOZ;
        shrdData->orientXOY = orientXOY;
        shrdData->orientYOZ = orientYOZ;
        if (stepCount != shrdData->stepCount)
            isChangedData = true;
        shrdData->stepCount = stepCount;
        shrdData->isComplete = true;

        if (isChangedData)
        {
            // Send message
            cnn->ExternalEvent(s_classNameStepCounter, s_eventChangeName, s_paramEvent);
            isChangedData = false;
        }
    }

    void updateData()
    {
        JNIEnv* jenv = getJniEnv();
        jfloat val = jenv->CallFloatMethod(obj, methodID_getOrientXOZ);
        double orientXOZ = (double)val;
        val = jenv->CallFloatMethod(obj, methodID_getOrientXOY);
        double orientXOY = (double)val;
        val = jenv->CallFloatMethod(obj, methodID_getOrientYOZ);
        double orientYOZ = (double)val;
        jint valSC = jenv->CallIntMethod(obj, methodID_getStepCount);
        long newStepCount = (long)valSC;

        updateAllData(orientXOZ, orientXOY, orientYOZ, newStepCount);
    }

    void startStepCounterUpdates()
    {
        JNIEnv* jenv = getJniEnv();
        shrdData->isComplete = false;
        shrdData->startDate = time(NULL);
        shrdData->stepCount = 0;
        jenv->CallVoidMethod(obj, methodID_resetCounter);
        // Register sensor
        jmethodID methodID_registerListener = jenv->GetMethodID(cc, "registerListener", "()V");
        jenv->CallVoidMethod(obj, methodID_registerListener);
    }

    void stopStepCounterUpdates()
    {
        JNIEnv* jenv = getJniEnv();
        // Register sensor
        jmethodID methodID_unregisterListener = jenv->GetMethodID(cc, "unregisterListener", "()V");
        jenv->CallVoidMethod(obj, methodID_unregisterListener);
    }

    std::wstring getLastErrorDescr()
    {
        JNIEnv* jenv = getJniEnv();
        jstring err = (jstring)(jenv->CallObjectMethod(obj, methodID_getErrorDescr));
        return jstring2wstring(jenv, err);
    }
};

extern "C" JNIEXPORT void JNICALL Java_com_stepCounterPackage_stepCounterLib_StepCounterClass_OnRefreshData(JNIEnv* jenv, jclass aClass,
    jlong pObject, jdouble orientXOZ, jdouble orientXOY, jdouble orientYOZ, jlong stepCount)
{
    // Bringing value to pointer to the object
    CStepCounterRecorder* step_counterObj = (CStepCounterRecorder*)pObject;
    if (step_counterObj)
        step_counterObj->updateAllData(orientXOZ, orientXOY, orientYOZ, stepCount);
}

#elif defined(MOBILE_PLATFORM_WINRT)

ref class CStepCounterRecorder sealed
{
private:
    IAddInDefBaseEx* cnn;
    Accelerometer^ accels;
    Windows::Foundation::EventRegistrationToken cookie;
    bool isChangedData;
    MotionData* shrdData;
    String^ lastError;

    const double       THRESHOLD_LEVEL_MAX = 0.4;            // [rad]
    const double       THRESHOLD_LEVEL_MIN = 0.001;          // [rad]
    const int          ORDER_FILTER = 3;                     // []
    const unsigned int REFRESH_INTERVAL_SENSOR_DATA = 100;   // [ms]
    const int          LENGTH_HISTORY_SENSOR_DATA = 5;       // [s]
    const int          MINIMAL_LONG_STEP = 20;               // [100ms]
    int                TAKT_COUNT_NOT_CALC;                  // [100ms]
    
    int sizeVector;
    std::vector<double> listAngleSmooth;
    std::vector<double> listAngleMetering;
    std::vector<double> listAngleMin;
    std::vector<double> listInStep;
    size_t itData;
    int nTakt;

    enum TypeValues { METERING, SMOOTH, MINSTAT, INSTEP };
    
    void setNextElement(TypeValues typeVal, double newValue)
    {
        // Shift iterator
        if (itData >= static_cast<size_t>(sizeVector - 1))
            itData = 0;
        else
            ++itData;
        // Memory a new value
        switch (typeVal)
        {
            case METERING: listAngleMetering.at(itData) = newValue; break;
            case SMOOTH:   listAngleSmooth.at(itData) = newValue; break;
            case MINSTAT:  listAngleMin.at(itData) = newValue; break;
            case INSTEP:   listInStep.at(itData) = newValue; break;
            default: ;
        }
    }
    
    void setElement(int itElem, TypeValues typeVal, double newValue)
    {
        if (itElem < 0)
            return;
        if (itElem >= sizeVector)
            return;
        int itCurr = itData - (sizeVector - itElem - 1);
        // Calculate iterator
        if (itCurr < 0)
            itCurr = sizeVector + itCurr;
        // Memory a new value
        switch (typeVal)
        {
            case METERING: listAngleMetering.at(itCurr) = newValue; break;
            case SMOOTH:   listAngleSmooth.at(itCurr) = newValue; break;
            case MINSTAT:  listAngleMin.at(itCurr) = newValue; break;
            case INSTEP:   listInStep.at(itCurr) = newValue; break;
            default: ;
        }
    }
    
    double getElement(int itElem, TypeValues typeVal)
    {
        if (itElem < 0)
            return 0.0;
        if (itElem >= sizeVector)
            return 0.0;
        int itCurr = itData - (sizeVector - itElem - 1);
        // Calculate iterator
        if (itCurr < 0)
            itCurr = sizeVector + itCurr;
        // Get value from memory
        double result = 0.0;
        switch (typeVal)
        {
            case METERING: result = listAngleMetering.at(itCurr); break;
            case SMOOTH:   result = listAngleSmooth.at(itCurr); break;
            case MINSTAT:  result = listAngleMin.at(itCurr); break;
            case INSTEP:   result = listInStep.at(itCurr); break;
            default: ;
        }
        return result;
    }

internal:

    CStepCounterRecorder(IAddInDefBaseEx* m_iConnect, MotionData* sharedData) : shrdData(sharedData)
    {
        isChangedData = false;
        shrdData->isComplete = false;
        shrdData->stepCount = 0;
        shrdData->typeMotion = eTypeUndefined;
        shrdData->startDate = time(NULL);
        lastError = L"";

        // Create the vector for sensor's data
        sizeVector = (int)ceil(LENGTH_HISTORY_SENSOR_DATA /
                     (REFRESH_INTERVAL_SENSOR_DATA / 1000.0));
        TAKT_COUNT_NOT_CALC = (int)(2.0 * sizeVector / LENGTH_HISTORY_SENSOR_DATA);   // []  2 - count of seconds

        listAngleMetering.reserve(sizeVector);
        listAngleSmooth.reserve(sizeVector);
        listAngleMin.reserve(sizeVector);
        listInStep.reserve(sizeVector);
        for (int i = 0; i < sizeVector; ++i) 
        {
            listAngleMetering.push_back(0.0);
            listAngleSmooth.push_back(0.0);
            listAngleMin.push_back(0.0);
            listInStep.push_back(0.0);
        }

        // Iterator sensor's data
        itData = 0;

        // Initial values
        nTakt = 0;

        cnn = m_iConnect;
        cnn->SetEventBufferDepth(10);

        accels = Accelerometer::GetDefault();
    }

public:

    virtual ~CStepCounterRecorder()
    {
        stopStepCounterUpdates();
    }

    void updateData() {}

    void startStepCounterUpdates()
    {
        if (accels)
        {
            accels->ReportInterval = max(accels->MinimumReportInterval, REFRESH_INTERVAL_SENSOR_DATA);
            cookie = accels->ReadingChanged += ref new TypedEventHandler<Accelerometer^, AccelerometerReadingChangedEventArgs^>(this, &CStepCounterRecorder::ReadingChanged);
        }
        else
        {
            lastError = L"Error accelerometer's access";
        }
        shrdData->isComplete = false;
        shrdData->startDate = time(NULL);
        shrdData->stepCount = 0;
    }

    void stopStepCounterUpdates()
    {
        if (accels)
            accels->ReadingChanged -= cookie;
    }
    
    String^ getLastErrorDescr()
    {
        return lastError;
    }

    void ReadingChanged(Accelerometer^ sender, AccelerometerReadingChangedEventArgs^ e)
    {
        double accelOX = 0.0;
        double accelOY = 0.0;
        double accelOZ = 0.0;
        {
            AccelerometerReading^ reading = e->Reading;

            accelOX = reading->AccelerationX;
            accelOY = reading->AccelerationY;
            accelOZ = reading->AccelerationZ;

            shrdData->orientXOZ = atan2(accelOY, accelOZ);
            shrdData->orientXOY = atan2(accelOX, accelOY);
            shrdData->orientYOZ = atan2(accelOX, accelOZ);
        }

        // Orientation vector
        double sizeAccel = sqrt(pow(accelOX, 2) + pow(accelOY, 2) + pow(accelOZ, 2));
        double cosAngle = accelOX / sizeAccel;
        double angleAccel = acos(cosAngle);

        // Memory new value
        setNextElement(METERING, angleAccel);

        if ((++nTakt) % TAKT_COUNT_NOT_CALC == 0) 
        {
            // Execute filter
            for (int i = sizeVector - TAKT_COUNT_NOT_CALC; i < sizeVector; ++i)
            {
                double s = 0.0;
                for (int j = i - ORDER_FILTER + 1; j <= i; ++j)
                    s += getElement(j, METERING);
                setElement(i, SMOOTH, s / ORDER_FILTER);
            }
        
            // Calculate minimum values
            for (int i = sizeVector - TAKT_COUNT_NOT_CALC; i < sizeVector; ++i)
            {
                setElement(i, MINSTAT, 10000.0); // Obviously large number
                int minPos = i - TAKT_COUNT_NOT_CALC * 2 + 1;
                if (minPos < 0)
                    minPos = 0;
                for (int j = minPos; j <= i; ++j)
                    setElement(i, MINSTAT, getElement(i, MINSTAT) < getElement(j, SMOOTH) ? getElement(i, MINSTAT) : getElement(j, SMOOTH));
            }

            // Calculation step's phases
            for (int i = sizeVector - TAKT_COUNT_NOT_CALC; i < sizeVector; ++i)
            {
                setElement(i, INSTEP, 0.0);
                if ((abs(getElement(i - 1, MINSTAT) - getElement(i, MINSTAT)) < THRESHOLD_LEVEL_MIN)
                   && (getElement(i - 1, MINSTAT) > THRESHOLD_LEVEL_MAX))
                {
                    int longPredValue = 1;
                    int j = i - 2;
                    while ((j >= 0) && (abs(getElement(i - 1, MINSTAT) - getElement(j, MINSTAT)) < THRESHOLD_LEVEL_MIN))
                    {
                        ++longPredValue;
                        --j;
                    }
                    if (longPredValue > MINIMAL_LONG_STEP)
                        setElement(i, INSTEP, 1.0);
                }
            }

            // Count steps
            bool isUp = false;
            for (int i = sizeVector - TAKT_COUNT_NOT_CALC - 1; i < sizeVector; ++i)
            {
                if (isUp && (getElement(i, INSTEP) < 0.5))
                {
                    // Every cycle is two steps
                    shrdData->stepCount += 2;
                    isChangedData = true;
                }
                isUp = (getElement(i, INSTEP) > 0.5);
            }
        }

        shrdData->isComplete = true;
        if (isChangedData)
        {
            cnn->ExternalEvent(s_classNameStepCounter, s_eventChangeName, s_paramEvent);
            isChangedData = false;
        }
    }
};

std::wstring pfstr(Platform::String^ str)
{
    return str ? (const wchar_t*)str->Data() : L"";
}

#endif //!__ANDROID__ && !MOBILE_PLATFORM_WINRT   


//----------------------------------------------------------------------------//

CStepCounter::CStepCounter() : piCnt(NULL), isRun(false), lastErrorDescr(L""),

#if defined(MOBILE_PLATFORM_WINRT)

    motionRecorder(nullptr)

#else //MOBILE_PLATFORM_WINRT

    motionRecorder(NULL)

#endif //!MOBILE_PLATFORM_WINRT

{
}

CStepCounter::~CStepCounter()
{
    if (motionRecorder) 
    {
        delete motionRecorder;
        
#if defined(MOBILE_PLATFORM_WINRT)

        motionRecorder = nullptr;
        
#else //MOBILE_PLATFORM_WINRT
    
        motionRecorder = NULL;
        
#endif //!MOBILE_PLATFORM_WINRT

    }       
}

void CStepCounter::setIConnect(IAddInDefBaseEx* piConnect)
{
    piCnt = piConnect;
    if (piCnt) 
    {
            
#if defined(MOBILE_PLATFORM_WINRT)

        motionRecorder = ref new CStepCounterRecorder(piCnt, &sharedData);
            
#else //MOBILE_PLATFORM_WINRT
    
        motionRecorder = new CStepCounterRecorder(piCnt, &sharedData);
            
#endif //!MOBILE_PLATFORM_WINRT

    }
}

void CStepCounter::startStepCounterUpdatesFromDate()
{
    // Set current date&time
    sharedData.startDate = time(NULL);
    if (motionRecorder) 
    {
        motionRecorder->startStepCounterUpdates();
        motionRecorder->updateData();
        isRun = true;
    }
}

void CStepCounter::stopStepCounterUpdates()
{
    isRun = false;
    if (motionRecorder)
        motionRecorder->stopStepCounterUpdates();
}

long CStepCounter::stepCount()
{
    if (sharedData.isComplete)
        return sharedData.stepCount;
    return 0;
}

TypesMotion CStepCounter::typeMotion()
{
    TypesMotion buf = eTypeUndefined;
    if (sharedData.isComplete) 
    {
        buf = sharedData.typeMotion;
        if ((buf < 0) || (buf >= eTypeLast))
            buf = eTypeUndefined;
    }
    return buf;
}

double CStepCounter::orientation(const Orientations plane)
{
    double res = 0.0;
    if (sharedData.isComplete) 
    {
        switch (plane) 
        {
            case eOrientXOZ:
                res = sharedData.orientXOZ;
                break;
            case eOrientXOY:
                res = sharedData.orientXOY;
                break;
            case eOrientYOZ:
                res = sharedData.orientYOZ;
                break;
        default:
            ;    
        }
    }
    return res;
}

bool CStepCounter::getRunningStepCounterUpdates()
{
    return isRun;
}

std::wstring CStepCounter::getLastErrorDescr()
{
    if (motionRecorder)
#if defined(MOBILE_PLATFORM_WINRT)
        lastErrorDescr = pfstr(motionRecorder->getLastErrorDescr());
#else //MOBILE_PLATFORM_WINRT
        lastErrorDescr = motionRecorder->getLastErrorDescr();
#endif //!MOBILE_PLATFORM_WINRT
    return lastErrorDescr;
}
