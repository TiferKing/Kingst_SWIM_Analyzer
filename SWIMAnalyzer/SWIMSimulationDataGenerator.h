#pragma once
#ifndef SWIM_SIMULATION_DATA_GENERATOR
#define SWIM_SIMULATION_DATA_GENERATOR

#include <AnalyzerHelpers.h>

class SWIMAnalyzerSettings;

class SWIMSimulationDataGenerator
{
public:
    SWIMSimulationDataGenerator();
    ~SWIMSimulationDataGenerator();

    void Initialize(U32 simulation_sample_rate, SWIMAnalyzerSettings* settings);
    U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels);

protected:
    SWIMAnalyzerSettings* mSettings;
    U32 mSimulationSampleRateHz;
    U64 mValue;

protected: //SWIM specific
    ClockGenerator mHSIClockGenerator;
    ClockGenerator mLSIClockGenerator;

    void GenerateSWIMTransaction();
    void GenerateEntrySequence();
    void GenerateROTFSequence(U8 data_len, U32 address, U8 data[], U8 high_spped = false);
    void GenerateWOTFSequence(U8 data_len, U32 address, U8 data[], U8 high_spped = false);
    void GenerateSRSTSequence(U8 high_spped = false);
    void GenerateByteHost(U8 data, U8 high_spped = false);
    void GenerateByteSlave(U8 data, U8 high_spped = false);
    void GenerateBitLow(U8 high_spped = false);
    void GenerateBitHigh(U8 high_spped = false);


    SimulationChannelDescriptorGroup mSWIMSimulationChannels;
    SimulationChannelDescriptor* mSWIM;
    SimulationChannelDescriptor* mRST;
};
#endif //SWIM_SIMULATION_DATA_GENERATOR
