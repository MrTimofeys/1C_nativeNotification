
#include "StepCounter.h"
#include "../../include/AddInDefBase.h"

#import <Foundation/Foundation.h>
#import <CoreMotion/CoreMotion.h>

namespace COM_1C_STEP_COUNTER
{

const double UPDATE_INTERVAL_DATA = 1.0f / 10.0f;

};

using namespace COM_1C_STEP_COUNTER;

@interface CStepCounterRecorder: NSObject
{
    CMMotionManager         *motionManager;
    CMMotionActivityManager *motionActivityManager;
    NSOperationQueue        *theQueue;
    CMPedometer             *stepCounterManager;
    IAddInDefBaseEx         *m_iConn;
    
    MotionData              *shrdData;
    bool                     isChangedData;
}

-(id)initWithSharedData:(IAddInDefBaseEx*)piCnt : (MotionData*)sharedData;
-(void)dealloc;
-(void)startStepCounterUpdates;
-(void)stopStepCounterUpdates;

@end;

//----------------------------------------------------------------------------//


@implementation CStepCounterRecorder


- (id)initWithSharedData:(IAddInDefBaseEx*)piCnt : (MotionData*)sharedData
{
    self = [super init];
    if (self)
    {
        m_iConn = NULL;
        motionManager = NULL;
        motionActivityManager = NULL;
        theQueue = NULL;
        stepCounterManager = NULL;

        shrdData = sharedData;
        shrdData->isComplete = false;
        shrdData->stepCount = 0;
        shrdData->orientXOZ = 0.0;
        shrdData->orientXOY = 0.0;
        shrdData->orientYOZ = 0.0;
        shrdData->typeMotion = eTypeUndefined;
        shrdData->startDate = time(NULL);
        
        m_iConn = (IAddInDefBaseEx*)piCnt;
        if (m_iConn)
            m_iConn->SetEventBufferDepth(10);

        motionManager = [[CMMotionManager alloc]init];
        
        motionActivityManager = [[CMMotionActivityManager alloc]init];
        
        theQueue = [[NSOperationQueue alloc] init];
   
        stepCounterManager = [[CMPedometer alloc]init];
    }
    return self;
}

-(void)dealloc
{
    if (stepCounterManager)
        [stepCounterManager release];
    if (theQueue)
        [theQueue release];
    if (motionActivityManager)
        [motionActivityManager release];
    if (motionManager)
        [motionManager release];
    m_iConn = NULL;
    shrdData = NULL;
    [super dealloc];
}

- (void)startStepCounterUpdates
{
    shrdData->isComplete = false;
    shrdData->startDate = time(NULL);
    shrdData->stepCount = 0;
    
    if (!m_iConn)
        return;
    
    // ***** Orientation
    if (motionManager)
    {
        motionManager.accelerometerUpdateInterval = UPDATE_INTERVAL_DATA;
        [motionManager startAccelerometerUpdatesToQueue:theQueue withHandler:^(CMAccelerometerData *accelerometerData, NSError *error)
        {
            if (accelerometerData)
            {
                shrdData->orientXOZ = atan2(accelerometerData.acceleration.y, accelerometerData.acceleration.z);
                shrdData->orientXOY = atan2(accelerometerData.acceleration.x, accelerometerData.acceleration.y);
                shrdData->orientYOZ = atan2(accelerometerData.acceleration.x, accelerometerData.acceleration.z);
            }
        }];
    }

    // ***** Type motion
    if (motionActivityManager)
        [motionActivityManager startActivityUpdatesToQueue:[NSOperationQueue mainQueue] withHandler:^(CMMotionActivity *activity)
         {
             if (activity) 
             {
                 TypesMotion res = eTypeUndefined;
                 if (activity.stationary)
                     res = eTypeStationary;
                 if (activity.walking)
                     res = eTypeWalking;
                 if (activity.running)
                     res = eTypeRunning;
                 if (activity.automotive)
                     res = eTypeAutomotive;
                 if (activity.cycling)
                     res = eTypeCycling;
                 self->shrdData->typeMotion = res;
             }
         }];
    
    // ***** Step count
    if (stepCounterManager) 
    {
        [stepCounterManager startPedometerUpdatesFromDate:[NSDate dateWithTimeIntervalSince1970:shrdData->startDate]
                                            withHandler:^(CMPedometerData * stepCounterData, NSError * error)
             {
                 dispatch_async (dispatch_get_main_queue(),
                               ^{
                                   if (stepCounterData)
                                   {
                                       self->isChangedData = false;
                                       long buf = stepCounterData.numberOfSteps.longValue;
                                       if (self->shrdData->stepCount != buf)
                                           self->isChangedData = true;
                                       self->shrdData->stepCount = buf;
                                       shrdData->isComplete = true;
                                   }
                                   
                                   if (self->isChangedData)
                                   {
                                       // Send message
                                       if (m_iConn)
                                           m_iConn->ExternalEvent(s_classNameStepCounter, s_eventChangeName, s_paramEvent);

                                       self->isChangedData = false;
                                   }
                               });
             }];
    }
    shrdData->isComplete = true;
}

- (void)stopStepCounterUpdates
{
    if (stepCounterManager)
        [stepCounterManager stopPedometerUpdates];
    if (motionActivityManager)
        [motionActivityManager stopActivityUpdates];
    if (motionManager)
        [motionManager stopAccelerometerUpdates];
    if (m_iConn)
        m_iConn->CleanEventBuffer();
}

@end


//----------------------------------------------------------------------------//

#if !defined(BUILD_DYNAMIC_LIBRARY)

namespace COM_1C_STEP_COUNTER
{

#endif //!BUILD_DYNAMIC_LIBRARY

CStepCounter::CStepCounter() : piCnt(NULL), isRun(false), lastErrorDescr(L""), motionRecorder(NULL)
{
    NSBundle* mb = [NSBundle mainBundle];
    if (mb)
    {
        NSString* value = [mb objectForInfoDictionaryKey:@"Special addin key"];
        if (value && [value isEqualToString:@"88821555"])
            return;
    }
    lastErrorDescr = L"Key verification error";
}

CStepCounter::~CStepCounter()
{
    if (motionRecorder) 
    {
        [motionRecorder stopStepCounterUpdates];
        [motionRecorder release];
        motionRecorder = NULL;
    }
}

void CStepCounter::setIConnect(IAddInDefBaseEx* piConnect)
{
    piCnt = piConnect;
    if (piCnt) 
    {
        motionRecorder = [[CStepCounterRecorder alloc]initWithSharedData: piCnt: &sharedData];
    }
}

void CStepCounter::startStepCounterUpdatesFromDate()
{
    // Set date&time
    sharedData.startDate = time(NULL);
    if (motionRecorder) 
    {
        [motionRecorder startStepCounterUpdates];
        isRun = true;
    }
}

void CStepCounter::stopStepCounterUpdates()
{
    isRun = false;
    if (motionRecorder)
        [motionRecorder stopStepCounterUpdates];
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
                res = 0.0;
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
    return lastErrorDescr;
}
    
#if !defined(BUILD_DYNAMIC_LIBRARY)

}

#endif //!BUILD_DYNAMIC_LIBRARY

