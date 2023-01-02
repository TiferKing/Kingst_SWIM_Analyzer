#include "SWIMSimulationDataGenerator.h"
#include "SWIMAnalyzerSettings.h"

SWIMSimulationDataGenerator::SWIMSimulationDataGenerator()
{
}

SWIMSimulationDataGenerator::~SWIMSimulationDataGenerator()
{
}

void SWIMSimulationDataGenerator::Initialize(U32 simulation_sample_rate, SWIMAnalyzerSettings* settings)
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mHSIClockGenerator.Init(mSettings->mHSIClockFreq, simulation_sample_rate);
    mLSIClockGenerator.Init(mSettings->mLSIClockFreq, simulation_sample_rate);

    if (settings->mSWIMChannel != UNDEFINED_CHANNEL) {
        mSWIM = mSWIMSimulationChannels.Add(settings->mSWIMChannel, mSimulationSampleRateHz, BIT_HIGH);
    }
    else {
        mSWIM = NULL;
    }

    if (settings->mResetChannel != UNDEFINED_CHANNEL) {
        mRST = mSWIMSimulationChannels.Add(settings->mResetChannel, mSimulationSampleRateHz, BIT_HIGH);
    }
    else {
        mRST = NULL;
    }

    mSWIMSimulationChannels.AdvanceAll(mLSIClockGenerator.AdvanceByTimeS(10e-6));     //insert 10us of idle

    mValue = 0;
}

U32 SWIMSimulationDataGenerator::GenerateSimulationData(U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels)
{
    U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

    while (mSWIM->GetCurrentSampleNumber() < adjusted_largest_sample_requested)
    {
        GenerateSWIMTransaction();
        mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByTimeS(1e-3));  //insert 1ms of idle
    }

    *simulation_channels = mSWIMSimulationChannels.GetArray();
    return mSWIMSimulationChannels.GetCount();
}

void SWIMSimulationDataGenerator::GenerateSWIMTransaction()
{
    U8 data[128] = { 0 };

    for (size_t i = 0; i < sizeof(data); i++)
    {
        data[i] = i;
    }

    GenerateEntrySequence();
    mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(1024 * 2));
    GenerateSRSTSequence(false);
    mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(1024 * 2));
    GenerateWOTFSequence(128, 0x00, data, false);
    mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(1024 * 2));
    GenerateROTFSequence(128, 0x00, data, false);
    mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(1024 * 2));

    GenerateSRSTSequence(true);
    mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(1024 * 2));
    GenerateWOTFSequence(128, 0x00, data, true);
    mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(1024 * 2));
    GenerateROTFSequence(128, 0x00, data, true);
    mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(1024 * 2));
}

void SWIMSimulationDataGenerator::GenerateEntrySequence()
{
    if (mRST != NULL)
    {
        mRST->TransitionIfNeeded(BIT_LOW);
    } // Reset signal

    mSWIMSimulationChannels.AdvanceAll(mLSIClockGenerator.AdvanceByHalfPeriod(2.0));

    if (mSWIM != NULL)
    {
        mSWIM->TransitionIfNeeded(BIT_LOW);

        mSWIMSimulationChannels.AdvanceAll(mLSIClockGenerator.AdvanceByTimeS(1e-3));

        for (size_t i = 0; i < 4; i++)
        {
            mSWIM->Transition();
            mSWIMSimulationChannels.AdvanceAll(mLSIClockGenerator.AdvanceByTimeS(0.5e-3));
            mSWIM->Transition();
            mSWIMSimulationChannels.AdvanceAll(mLSIClockGenerator.AdvanceByTimeS(0.5e-3));
        } // Four pulses at 1 kHz.

        for (size_t i = 0; i < 4; i++)
        {
            mSWIM->Transition();
            mSWIMSimulationChannels.AdvanceAll(mLSIClockGenerator.AdvanceByTimeS(0.25e-3));
            mSWIM->Transition();
            mSWIMSimulationChannels.AdvanceAll(mLSIClockGenerator.AdvanceByTimeS(0.25e-3));
        } // Four pulses at 2 kHz.

        mSWIM->Transition();
        mSWIMSimulationChannels.AdvanceAll(mLSIClockGenerator.AdvanceByTimeS(0.25e-3));
        // Waiting for HSI to start up.

        mSWIM->Transition();
        mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(128 * 2));
        // Slave send 128 pulses at HSI frequence.

        mSWIM->Transition();
        mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByTimeS(400e-6));
        // Waiting for HSI to calibrate.
    }

    // Setting SWIM_CSR register
    U8 data[] = { 0xA0 };
    GenerateWOTFSequence(1, 0x7F80, data, false);

    if (mRST != NULL)
    {
        mRST->TransitionIfNeeded(BIT_HIGH);
    } // Reset signal
}

void SWIMSimulationDataGenerator::GenerateROTFSequence(U8 data_len, U32 address, U8 data[], U8 high_spped)
{
    // ROTF Head: 00011 1
    GenerateBitLow(high_spped);
    GenerateBitLow(high_spped);
    GenerateBitLow(high_spped);
    GenerateBitHigh(high_spped);
    GenerateBitHigh(high_spped);
    GenerateBitHigh(high_spped);

    // N:
    GenerateByteHost(data_len, high_spped);

    // Address
    GenerateByteHost(address >> 16, high_spped);
    GenerateByteHost(address >> 8, high_spped);
    GenerateByteHost(address, high_spped);

    // Data
    for (size_t i = 0; i < data_len; i++)
    {
        GenerateByteSlave(data[i], high_spped);
    }
}

void SWIMSimulationDataGenerator::GenerateWOTFSequence(U8 data_len, U32 address, U8 data[], U8 high_spped)
{
    // WOTF Head: 00101 1
    GenerateBitLow(high_spped);
    GenerateBitLow(high_spped);
    GenerateBitHigh(high_spped);
    GenerateBitLow(high_spped);
    GenerateBitHigh(high_spped);
    GenerateBitHigh(high_spped);

    // N:
    GenerateByteHost(data_len, high_spped);

    // Address
    GenerateByteHost(address >> 16, high_spped);
    GenerateByteHost(address >> 8, high_spped);
    GenerateByteHost(address, high_spped);

    // Data
    for (size_t i = 0; i < data_len; i++)
    {
        GenerateByteHost(data[i], high_spped);
    }
}

void SWIMSimulationDataGenerator::GenerateSRSTSequence(U8 high_spped)
{
    // WOTF Head: 00000 1
    GenerateBitLow(high_spped);
    GenerateBitLow(high_spped);
    GenerateBitLow(high_spped);
    GenerateBitLow(high_spped);
    GenerateBitLow(high_spped);
    GenerateBitHigh(high_spped);
}

void SWIMSimulationDataGenerator::GenerateByteHost(U8 data, U8 high_spped)
{
    // Header bit
    GenerateBitLow(high_spped);

    // 8-bit data
    BitExtractor bit_extractor(data, AnalyzerEnums::ShiftOrder::MsbFirst, 8);
    for (size_t i = 0; i < 8; i++)
    {
        if (bit_extractor.GetNextBit())
        {
            GenerateBitHigh(high_spped);
        }
        else
        {
            GenerateBitLow(high_spped);
        }
    }

    // Parity bit
    if (AnalyzerHelpers::IsOdd(AnalyzerHelpers::GetOnesCount(data)) == true)
    {
        GenerateBitHigh(high_spped);
    }
    else
    {
        GenerateBitLow(high_spped);
    }

    // ACK bit
    GenerateBitHigh(high_spped);
}

void SWIMSimulationDataGenerator::GenerateByteSlave(U8 data, U8 high_spped)
{
    // Header bit
    GenerateBitHigh(high_spped);

    // 8-bit data
    BitExtractor bit_extractor(data, AnalyzerEnums::ShiftOrder::MsbFirst, 8);
    for (size_t i = 0; i < 8; i++)
    {
        if (bit_extractor.GetNextBit())
        {
            GenerateBitHigh(high_spped);
        }
        else
        {
            GenerateBitLow(high_spped);
        }
    }

    // Parity bit
    if (AnalyzerHelpers::IsOdd(AnalyzerHelpers::GetOnesCount(data)) == true)
    {
        GenerateBitHigh(high_spped);
    }
    else
    {
        GenerateBitLow(high_spped);
    }

    // ACK bit
    GenerateBitHigh(high_spped);
}

void SWIMSimulationDataGenerator::GenerateBitLow(U8 high_spped)
{
    if (mSWIM != NULL)
    {
        mSWIM->TransitionIfNeeded(BIT_LOW);

        mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(high_spped == false ? 20 * 2 : 8 * 2));

        mSWIM->TransitionIfNeeded(BIT_HIGH);

        mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(2 * 2));
    }
}

void SWIMSimulationDataGenerator::GenerateBitHigh(U8 high_spped)
{
    if (mSWIM != NULL)
    {
        mSWIM->TransitionIfNeeded(BIT_LOW);

        mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(2 * 2));

        mSWIM->TransitionIfNeeded(BIT_HIGH);

        mSWIMSimulationChannels.AdvanceAll(mHSIClockGenerator.AdvanceByHalfPeriod(high_spped == false ? 20 * 2 : 8 * 2));
    }
}