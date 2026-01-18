
package com.stepCounterPackage.stepCounterLib;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorManager;
import android.hardware.SensorEventListener;
import static java.lang.Math.*;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Vector;
import java.lang.String;

@android.support.annotation.Keep
public class StepCounterClass implements Runnable, SensorEventListener
{
    Activity     m_Activity;
    long         m_CallObject;
    boolean      m_IsAllLoaded;
    
    final double THRESHOLD_LEVEL_MAX = 0.4;            // [rad]
    final double THRESHOLD_LEVEL_MIN = 0.001;          // [rad]
    final int    ORDER_FILTER = 3;                     // []
    final int    REFRESH_INTERVAL_SENSOR_DATA = 100;   // [ms]
    final int    LENGTH_HISTORY_SENSOR_DATA = 5;       // [s]
    final int    MINIMAL_LONG_STEP = 20;               // [100ms]
    int          TAKT_COUNT_NOT_CALC;                  // [100ms]
    
    private double m_CurAccelOX;
    private double m_CurAccelOY;
    private double m_CurAccelOZ;
    private int    m_CurStepCount;
    private int    m_SizeVector;
    private int    m_ItData;
    private int    m_NTakt;
    
    enum TypeValues { METERING, SMOOTH, MINSTAT, INSTEP };
    
    private Vector<Double> m_ListAngleMetering;
    private Vector<Double> m_ListAngleSmooth;
    private Vector<Double> m_ListAngleMin;
    private Vector<Double> m_ListInStep;

    private Timer         m_SensorData;
    private TimerTask     m_RefreshSensorData;
    private SensorManager m_SensorManager = null;
    private String        m_LastError = "";
    
    private static native void NativeTrace(String str);
    
    private void trace(String str)
    {
        if (m_IsAllLoaded)
            NativeTrace(str);
    }
   
    private static native void OnRefreshData(long pObject, double orientXOZ, double orientXOY, double orientYOZ, long stepCount);
    
    private void refreshData(double orientOX, double orientOY, double orientOZ, long stepCount)
    {
        if (m_IsAllLoaded && (m_CallObject != 0))
            OnRefreshData(m_CallObject, atan2(orientOY, orientOZ), atan2(orientOX, orientOY), atan2(orientOX, orientOZ), stepCount);
    }
    
    private void setNextElement(TypeValues typeVal, double newValue)
    {
        // Shift iterator
        if (m_ItData >= (m_SizeVector - 1))
            m_ItData = 0;
        else
            ++m_ItData;
        // Memory a new value
        switch (typeVal)
        {
            case METERING: m_ListAngleMetering.setElementAt(newValue, m_ItData); break;
            case SMOOTH:   m_ListAngleSmooth.setElementAt(newValue, m_ItData); break;
            case MINSTAT:  m_ListAngleMin.setElementAt(newValue, m_ItData); break;
            case INSTEP:   m_ListInStep.setElementAt(newValue, m_ItData); break;
            default: ;
        }
    }
    
    private void setElement(int itElem, TypeValues typeVal, double newValue)
    {
        if (itElem < 0)
            return;
        if (itElem >= m_SizeVector)
            return;
        int itCurr = m_ItData - (m_SizeVector - itElem - 1);
        // Calculate iterator
        if (itCurr < 0)
            itCurr = m_SizeVector + itCurr;
        // Memory a new value
        switch (typeVal)
        {
            case METERING: m_ListAngleMetering.setElementAt(newValue, itCurr); break;
            case SMOOTH:   m_ListAngleSmooth.setElementAt(newValue, itCurr); break;
            case MINSTAT:  m_ListAngleMin.setElementAt(newValue, itCurr); break;
            case INSTEP:   m_ListInStep.setElementAt(newValue, itCurr); break;
            default: ;
        }
    }
    
    private double getElement(int itElem, TypeValues typeVal)
    {
        if (itElem < 0)
            return 0.0;
        if (itElem >= m_SizeVector)
            return 0.0;
        int itCurr = m_ItData - (m_SizeVector - itElem - 1);
        // Calculate iterator
        if (itCurr < 0)
            itCurr = m_SizeVector + itCurr;
        // Get value from memory
        double result = 0.0;
        switch (typeVal)
        {
            case METERING: result = m_ListAngleMetering.get(itCurr); break;
            case SMOOTH:   result = m_ListAngleSmooth.get(itCurr); break;
            case MINSTAT:  result = m_ListAngleMin.get(itCurr); break;
            case INSTEP:   result = m_ListInStep.get(itCurr); break;
            default: ;
        }
        return result;
    }
    
    public void onAddSensorData()
    {
        // Acceleration vector              
        double sizeAccel = Math.sqrt(
                Math.pow(m_CurAccelOX, 2) +
                Math.pow(m_CurAccelOY, 2) +
                Math.pow(m_CurAccelOZ, 2));
        
        double cosAngle = m_CurAccelOX / sizeAccel;
        double angleAccel = Math.acos(cosAngle);    
            
        // Memory new value
        setNextElement(TypeValues.METERING, angleAccel);

        if ((++m_NTakt) % TAKT_COUNT_NOT_CALC == 0)
        {
            // Execute filter
            for (int i = m_SizeVector - TAKT_COUNT_NOT_CALC; i < m_SizeVector; ++i)
            {
                double s = 0.0;
                for (int j = i - ORDER_FILTER + 1; j <= i; ++j)
                    s += getElement(j, TypeValues.METERING);
                setElement(i, TypeValues.SMOOTH, s / ORDER_FILTER);
            }
        
            // Calculate minimum values
            for (int i = m_SizeVector - TAKT_COUNT_NOT_CALC; i < m_SizeVector; ++i)
            {
                setElement(i, TypeValues.MINSTAT, 10000.0); // Obviously large number
                int minPos = Math.max(0, i - TAKT_COUNT_NOT_CALC * 2 + 1);
                for (int j = minPos; j <= i; ++j)
                    setElement(i, TypeValues.MINSTAT, Math.min(getElement(i, TypeValues.MINSTAT), getElement(j, TypeValues.SMOOTH)));
            }

            // Calculation step's phases
            for (int i = m_SizeVector - TAKT_COUNT_NOT_CALC; i < m_SizeVector; ++i)
            {
                setElement(i, TypeValues.INSTEP, 0.0);
                if ((Math.abs(getElement(i - 1, TypeValues.MINSTAT) - getElement(i, TypeValues.MINSTAT)) < THRESHOLD_LEVEL_MIN)
                   && (getElement(i - 1, TypeValues.MINSTAT) > THRESHOLD_LEVEL_MAX))
                {
                    int longPredValue = 1;
                    int j = i - 2;
                    while ((j >= 0) && (Math.abs(getElement(i - 1, TypeValues.MINSTAT) - getElement(j, TypeValues.MINSTAT)) < THRESHOLD_LEVEL_MIN))
                    {
                        ++longPredValue;
                        --j;
                    }
                    if (longPredValue > MINIMAL_LONG_STEP)
                        setElement(i, TypeValues.INSTEP, 1.0);
                }
            }

            // Count steps
            boolean isUp = false;
            for (int i = m_SizeVector - TAKT_COUNT_NOT_CALC - 1; i < m_SizeVector; ++i)
            {
                if (isUp && (getElement(i, TypeValues.INSTEP) < 0.5))
                {
                    // Every cycle is two steps
                    m_CurStepCount += 2;
                    refreshData(m_CurAccelOX, m_CurAccelOY, m_CurAccelOZ, m_CurStepCount);                   
                }
                isUp = (getElement(i, TypeValues.INSTEP) > 0.5);
            }
        }
    }

    public StepCounterClass(Activity activity, long pObject)
    {
        m_Activity = activity;
        m_CallObject = pObject;
        m_IsAllLoaded = false;
        m_LastError = "";

        m_SensorManager = (SensorManager) activity.getSystemService(Context.SENSOR_SERVICE);

        // Create the vector for sensor's data
        m_SizeVector = (int) Math.round(LENGTH_HISTORY_SENSOR_DATA /
                                                (REFRESH_INTERVAL_SENSOR_DATA / 1000.0));
        TAKT_COUNT_NOT_CALC = (int)(2.0 * m_SizeVector / LENGTH_HISTORY_SENSOR_DATA);   // []  2 - count of seconds
        
        m_ListAngleMetering = new Vector<Double>(m_SizeVector);
        m_ListAngleSmooth = new Vector<Double>(m_SizeVector);
        m_ListAngleMin = new Vector<Double>(m_SizeVector);
        m_ListInStep = new Vector<Double>(m_SizeVector);
        for (int i = 0; i < m_SizeVector; ++i) 
        {
            m_ListAngleMetering.addElement(Double.valueOf(0.0));
            m_ListAngleSmooth.addElement(Double.valueOf(0.0));
            m_ListAngleMin.addElement(Double.valueOf(0.0));
            m_ListInStep.addElement(Double.valueOf(0.0));
        }
        
        // Iterator sensor's data
        m_ItData = 0;
        
        // Initial values
        m_CurAccelOX = 0.0;
        m_CurAccelOY = 0.0;
        m_CurAccelOZ = 0.0;
        m_NTakt = 0;
        m_CurStepCount = 0;
        
        // Timer for refresh sensor's data
        m_SensorData = new Timer();
        m_RefreshSensorData = new TimerTask()
        {
            public void run()
            {
                onAddSensorData();
            }
        };
        m_SensorData.schedule(m_RefreshSensorData, REFRESH_INTERVAL_SENSOR_DATA, REFRESH_INTERVAL_SENSOR_DATA);
    }

    public void registerListener()
    {
        // Set event handler accelerometer's sensor data
        m_SensorManager.registerListener(this,
                                        m_SensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                                        SensorManager.SENSOR_DELAY_NORMAL);
    }

    public void unregisterListener()
    {
        // Set event handler accelerometer's sensor data is null
        m_SensorManager.unregisterListener(this);
    }
    
    public void show()
    {
        m_Activity.runOnUiThread(this);
    }

    public void run()
    {   
        try 
        {
            System.loadLibrary("com_1c_StepCounter");
            m_IsAllLoaded = true;
        }
        catch (UnsatisfiedLinkError e) 
        {
            m_LastError = "Error loading library com_1c_StepCounter";
            e.printStackTrace();
        }
        System.out.println(this + ".run() END");
    }
    
    public void resetCounter()
    {
        m_CurStepCount = 0;
        refreshData(m_CurAccelOX, m_CurAccelOY, m_CurAccelOZ, m_CurStepCount);
    }
    
    public int getStepCount()
    {
        return m_CurStepCount;
    }
    
    public float getOrientXOZ()
    {
        return (float)atan2(m_CurAccelOY, m_CurAccelOZ);
    }
    
    public float getOrientXOY()
    {
        return (float)atan2(m_CurAccelOX, m_CurAccelOY);
    }
    
    public float getOrientYOZ()
    {
        return (float)atan2(m_CurAccelOX, m_CurAccelOZ);
    }
    
    public void clearObjectsReference()
    {
        m_CallObject = 0;
    }
    
    public String getErrorDescr()
    {
        return m_LastError;
    }

    @Override
    public void onSensorChanged(SensorEvent event) 
    {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) 
        {
            m_CurAccelOX = event.values[0];
            m_CurAccelOY = event.values[1];
            m_CurAccelOZ = event.values[2];
        }
    }
    
    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {}
    
}
