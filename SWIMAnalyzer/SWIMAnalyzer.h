#pragma once
#ifndef SWIM_ANALYZER_H
#define SWIM_ANALYZER_H

#include <Analyzer.h>
#include "SWIMAnalyzerResults.h"
#include "SWIMSimulationDataGenerator.h"

class SWIMAnalyzerSettings;
enum FrameType
{
    EntryFrame,
    EntryPulses,
    EntryHSICalibrate,
    SRSTFrame,
    ROTFFrame,
    WOTFFrame,
    DataLenFrame,
    AddressFrame,
    DataReadFrame,
    DataWriteFrame,
    ErrorFrame,
    UnknowFrame
};

#define SRST_HEADER 0x0
#define ROTF_HEADER 0x2
#define WOTF_HEADER 0x4

class ANALYZER_EXPORT SWIMAnalyzer : public Analyzer
{
public:
    SWIMAnalyzer();
    virtual ~SWIMAnalyzer();
    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels);
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();

protected: //functions
    void SetupChannel();
    void SetupSample();
    void ParseSWIM(bool is_high_speed);
    BitState ParseBit(bool is_high_speed);
    BitState ParseACK(bool is_high_speed);
    void ParseEntry();

#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'SerialAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class
protected:  //vars
    std::auto_ptr< SWIMAnalyzerSettings > mSettings;
    std::auto_ptr< SWIMAnalyzerResults > mResults;
    bool mSimulationInitilized;
    SWIMSimulationDataGenerator mSimulationDataGenerator;

    AnalyzerChannelData* mSWIM;
    AnalyzerChannelData* mRST;

    U64 mCurrentSample;
    std::vector<U32> mHighSpeedSampleOffsets;
    std::vector<U32> mLowSpeedSampleOffsets;

#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer * __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer * analyzer);

#endif //SWIM_ANALYZER_H
