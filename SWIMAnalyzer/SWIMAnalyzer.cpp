
#include "SWIMAnalyzer.h"
#include "SWIMAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

SWIMAnalyzer::SWIMAnalyzer()
    : Analyzer(),
    mSettings(new SWIMAnalyzerSettings()),
    mSimulationInitilized(false),
    mSWIM(NULL),
    mRST(NULL)
{
    SetAnalyzerSettings(mSettings.get());
}

SWIMAnalyzer::~SWIMAnalyzer()
{
    KillThread();
}

void SWIMAnalyzer::SetupResults()
{
    mResults.reset(new SWIMAnalyzerResults(this, mSettings.get()));
    SetAnalyzerResults(mResults.get());

    if (mSettings->mSWIMChannel != UNDEFINED_CHANNEL) {
        mResults->AddChannelBubblesWillAppearOn(mSettings->mSWIMChannel);
    }
    if (mSettings->mResetChannel != UNDEFINED_CHANNEL) {
        mResults->AddChannelBubblesWillAppearOn(mSettings->mResetChannel);
    }
}

void SWIMAnalyzer::WorkerThread()
{
    SetupChannel();
    SetupSample();

    if (mSWIM->GetBitState() == BIT_LOW)
    {
        mSWIM->AdvanceToNextEdge();
    }

    for (; ;)
    {
        if (mSWIM->GetBitState() == BIT_HIGH)
        {
            mSWIM->AdvanceToNextEdge();
        }

        U64 starting_sample = mSWIM->GetSampleNumber();
        bool high_speed_bit = false;
        bool is_entry = false;

        
        if (mRST != NULL)
        {
            mRST->AdvanceToAbsPosition(starting_sample);
            if (mRST->GetBitState() == BIT_LOW)
            {
                is_entry = true;
            }
        }
        if(!is_entry)
        {
            // Auto detect bit speed
            for (;;)
            {
                starting_sample = mSWIM->GetSampleNumber();
                U64 next_sample = mSWIM->GetSampleOfNextEdge();
                U64 sample_num = next_sample - starting_sample;
                ClockGenerator clk_gen;
                clk_gen.Init(mSettings->mHSIClockFreq / 2, GetSampleRate());
                U64 sample_1 = clk_gen.AdvanceByHalfPeriod(14);
                U64 sample_2 = clk_gen.AdvanceByHalfPeriod(30);
                U64 sample_3 = clk_gen.AdvanceByTimeS(0.5e-3);
                if (sample_num <= sample_1)
                {
                    high_speed_bit = true;
                    break;
                }
                else if (sample_num <= sample_2)
                {
                    high_speed_bit = false;
                    break;
                }
                else if(sample_num >= sample_3)
                {
                    is_entry = true;
                    break;
                }
                else
                {
                    mSWIM->AdvanceToNextEdge();
                    mSWIM->AdvanceToNextEdge();
                }
            }
        }

        if (is_entry)
        {
            ParseEntry();
        }
        else
        {
            ParseSWIM(high_speed_bit);
        }

        mResults->CommitResults();

        CheckIfThreadShouldExit();
    }
}

bool SWIMAnalyzer::NeedsRerun()
{
    return false;
}

void SWIMAnalyzer::SetupChannel()
{
    mSWIM = GetAnalyzerChannelData(mSettings->mSWIMChannel);

    if (mSettings->mResetChannel != UNDEFINED_CHANNEL)
    {
        mRST = GetAnalyzerChannelData(mSettings->mResetChannel);
    }
    else
    {
        mRST = NULL;
    }
}

void SWIMAnalyzer::SetupSample()
{
    ClockGenerator clk_gen;
    clk_gen.Init(mSettings->mHSIClockFreq / 2, GetSampleRate());
    mHighSpeedSampleOffsets.clear();
    mHighSpeedSampleOffsets.push_back(0);
    for (size_t i = 0; i < 9; i++)
    {
        mHighSpeedSampleOffsets.push_back(clk_gen.AdvanceByHalfPeriod());
    }

    clk_gen.Init(mSettings->mHSIClockFreq / 2, GetSampleRate());
    mLowSpeedSampleOffsets.clear();
    mLowSpeedSampleOffsets.push_back(0);
    for (size_t i = 0; i < 21; i++)
    {
        mLowSpeedSampleOffsets.push_back(clk_gen.AdvanceByHalfPeriod());
    }

    mTimeoutSampleNum = clk_gen.AdvanceByTimeS(5e-4); // Set bit timeout 0.5ms
}

void SWIMAnalyzer::ParseSWIM(bool is_high_speed)
{

    U64 starting_sample = mSWIM->GetSampleNumber();
    U64 ending_sample;
    Frame frame;

    // Get header
    if (mSWIM->GetBitState() == BIT_HIGH)
    {
        mSWIM->AdvanceToNextEdge();
    }
    U64 header = 0;
    DataBuilder data_builder;
    data_builder.Reset(&header, AnalyzerEnums::MsbFirst, 5);
    bool ack = false;
    FrameType type;

    for (size_t i = 0; i < 5; i++)
    {
        data_builder.AddBit(ParseBit(is_high_speed));
    }

    ack = ParseACK(is_high_speed) == BIT_HIGH;

    ending_sample = mSWIM->GetSampleNumber();

    switch (header & 0xE)
    {
    case SRST_HEADER:
        type = SRSTFrame;
        break;
    case ROTF_HEADER:
        type = ROTFFrame;
        break;
    case WOTF_HEADER:
        type = WOTFFrame;
        break;
    default:
        type = UnknowFrame;
        break;
    }
    if (AnalyzerHelpers::IsOdd(AnalyzerHelpers::GetOnesCount(header)))
    {
        type = ErrorFrame;
    }
    frame.mStartingSampleInclusive = starting_sample;
    frame.mEndingSampleInclusive = ending_sample;
    frame.mData1 = 0;
    frame.mData2 = 0;
    frame.mType = type;
    frame.mFlags = ack;
    mResults->AddFrame(frame);

    U64 data_len = 0;
    FrameType type_parse = type;
    if ((type_parse == ROTFFrame) || (type_parse == WOTFFrame))
    {
        // data_len frame
        if (mSWIM->GetBitState() == BIT_HIGH)
        {
            mSWIM->AdvanceToNextEdge();
        }
        data_builder.Reset(&data_len, AnalyzerEnums::MsbFirst, 10);
        starting_sample = mSWIM->GetSampleNumber();
        for (size_t i = 0; i < 10; i++)
        {
            data_builder.AddBit(ParseBit(is_high_speed));
        }
        ack = ParseACK(is_high_speed) == BIT_HIGH;
        if (AnalyzerHelpers::IsOdd(AnalyzerHelpers::GetOnesCount(data_len)))
        {
            type = ErrorFrame;
        }
        else
        {
            type = DataLenFrame;
        }
        ending_sample = mSWIM->GetSampleNumber();
        frame.mStartingSampleInclusive = starting_sample;
        frame.mEndingSampleInclusive = ending_sample;
        frame.mData1 = data_len >> 1;
        frame.mData2 = 0;
        frame.mType = type;
        frame.mFlags = ack;
        mResults->AddFrame(frame);
        data_len >>= 1;

        //address frame
        U64 address = 0;
        U64 address_tmp = 0;
        //address E
        if (mSWIM->GetBitState() == BIT_HIGH)
        {
            mSWIM->AdvanceToNextEdge();
        }
        starting_sample = mSWIM->GetSampleNumber();
        data_builder.Reset(&address_tmp, AnalyzerEnums::MsbFirst, 10);
        for (size_t i = 0; i < 10; i++)
        {
            data_builder.AddBit(ParseBit(is_high_speed));
        }
        ack = ParseACK(is_high_speed) == BIT_HIGH;
        if (AnalyzerHelpers::IsOdd(AnalyzerHelpers::GetOnesCount(address_tmp)))
        {
            type = ErrorFrame;
        }
        else
        {
            type = AddressFrame;
        }
        address = ((address_tmp & 0x1FE) >> 1) << 16;

        //address H
        if (mSWIM->GetBitState() == BIT_HIGH)
        {
            mSWIM->AdvanceToNextEdge();
        }
        address_tmp = 0;
        data_builder.Reset(&address_tmp, AnalyzerEnums::MsbFirst, 10);
        for (size_t i = 0; i < 10; i++)
        {
            data_builder.AddBit(ParseBit(is_high_speed));
        }
        ack &= ParseACK(is_high_speed) == BIT_HIGH;
        if (AnalyzerHelpers::IsOdd(AnalyzerHelpers::GetOnesCount(address_tmp)))
        {
            type = ErrorFrame;
        }
        else
        {
            type = AddressFrame;
        }
        address |= ((address_tmp & 0x1FE) >> 1) << 8;

        //address L
        if (mSWIM->GetBitState() == BIT_HIGH)
        {
            mSWIM->AdvanceToNextEdge();
        }
        address_tmp = 0;
        data_builder.Reset(&address_tmp, AnalyzerEnums::MsbFirst, 10);
        for (size_t i = 0; i < 10; i++)
        {
            data_builder.AddBit(ParseBit(is_high_speed));
        }
        ack &= ParseACK(is_high_speed) == BIT_HIGH;
        if (AnalyzerHelpers::IsOdd(AnalyzerHelpers::GetOnesCount(address_tmp)))
        {
            type = ErrorFrame;
        }
        else
        {
            type = AddressFrame;
        }
        address |= (address_tmp & 0x1FE) >> 1;

        ending_sample = mSWIM->GetSampleNumber();
        frame.mStartingSampleInclusive = starting_sample;
        frame.mEndingSampleInclusive = ending_sample;
        frame.mData1 = address;
        frame.mData2 = 0;
        frame.mType = type;
        frame.mFlags = ack;
        mResults->AddFrame(frame);
    }

    if (type_parse == ROTFFrame)
    {
        //Read Data

        for (size_t j = 0; j < data_len; j++)
        {
            if (mSWIM->GetBitState() == BIT_HIGH)
            {
                mSWIM->AdvanceToNextEdge();
            }
            U64 data;
            data_builder.Reset(&data, AnalyzerEnums::MsbFirst, 10);
            starting_sample = mSWIM->GetSampleNumber();
            for (size_t i = 0; i < 10; i++)
            {
                data_builder.AddBit(ParseBit(is_high_speed));
            }
            ack = ParseACK(is_high_speed) == BIT_HIGH;
            if (AnalyzerHelpers::IsEven(AnalyzerHelpers::GetOnesCount(data)))
            {
                type = ErrorFrame;
            }
            else
            {
                type = DataReadFrame;
            }
            ending_sample = mSWIM->GetSampleNumber();
            frame.mStartingSampleInclusive = starting_sample;
            frame.mEndingSampleInclusive = ending_sample;
            frame.mData1 = (data >> 1) & 0xFF;
            frame.mData2 = 0;
            frame.mType = type;
            frame.mFlags = ack;
            mResults->AddFrame(frame);
        }

    }
    else if (type_parse == WOTFFrame)
    {
        //Write Data

        for (size_t j = 0; j < data_len; j++)
        {
            if (mSWIM->GetBitState() == BIT_HIGH)
            {
                mSWIM->AdvanceToNextEdge();
            }
            U64 data;
            data_builder.Reset(&data, AnalyzerEnums::MsbFirst, 10);
            starting_sample = mSWIM->GetSampleNumber();
            for (size_t i = 0; i < 10; i++)
            {
                data_builder.AddBit(ParseBit(is_high_speed));
            }
            ack = ParseACK(is_high_speed) == BIT_HIGH;
            if (AnalyzerHelpers::IsOdd(AnalyzerHelpers::GetOnesCount(data)))
            {
                type = ErrorFrame;
            }
            else
            {
                type = DataWriteFrame;
            }
            ending_sample = mSWIM->GetSampleNumber();
            frame.mStartingSampleInclusive = starting_sample;
            frame.mEndingSampleInclusive = ending_sample;
            frame.mData1 = (data >> 1) & 0xFF;
            frame.mData2 = 0;
            frame.mType = type;
            frame.mFlags = ack;
            mResults->AddFrame(frame);
        }
    }
}

BitState SWIMAnalyzer::ParseBit(bool is_high_speed)
{
    U32 high_bit_num = 0;
    U32 low_bit_num = 0;
    BitState bit;
    U64 starting_sample = mSWIM->GetSampleNumber();

    if (is_high_speed)
    {
        for (size_t j = 0; j < mHighSpeedSampleOffsets.size(); j++)
        {
            mSWIM->Advance(mHighSpeedSampleOffsets[j]);
            if (mSWIM->GetBitState() == BIT_LOW)
            {
                low_bit_num++;
            }
            else
            {
                high_bit_num++;
            }
        }

        U64 ending_sample = mSWIM->GetSampleNumber();

        if (low_bit_num <= 4)
        {
            bit = BIT_HIGH;
            mResults->AddMarker((starting_sample + ending_sample) / 2, AnalyzerResults::One, mSettings->mSWIMChannel);
        }
        else
        {
            bit = BIT_LOW;
            mResults->AddMarker((starting_sample + ending_sample) / 2, AnalyzerResults::Zero, mSettings->mSWIMChannel);
        }
    }
    else
    {
        for (size_t j = 0; j < mLowSpeedSampleOffsets.size(); j++)
        {
            mSWIM->Advance(mLowSpeedSampleOffsets[j]);
            if (mSWIM->GetBitState() == BIT_LOW)
            {
                low_bit_num++;
            }
            else
            {
                high_bit_num++;
            }
        }

        U64 ending_sample = mSWIM->GetSampleNumber();

        if (low_bit_num <= 8)
        {
            bit = BIT_HIGH;
            mResults->AddMarker((starting_sample + ending_sample) / 2, AnalyzerResults::One, mSettings->mSWIMChannel);
        }
        else
        {
            bit = BIT_LOW;
            mResults->AddMarker((starting_sample + ending_sample) / 2, AnalyzerResults::Zero, mSettings->mSWIMChannel);
        }
    }
    
    if ((mSWIM->GetSampleOfNextEdge() - mSWIM->GetSampleNumber()) < mTimeoutSampleNum)
    {
        // Next bit present
        mSWIM->AdvanceToNextEdge();
    }
    return bit;
}

BitState SWIMAnalyzer::ParseACK(bool is_high_speed)
{
    U32 high_bit_num = 0;
    U32 low_bit_num = 0;
    BitState bit;

    if (mSWIM->GetBitState() == BIT_HIGH)
    {
        if ((mSWIM->GetSampleOfNextEdge() - mSWIM->GetSampleNumber()) < mTimeoutSampleNum)
        {
            mSWIM->AdvanceToNextEdge();
        }
        else
        {
            // ACK edge not present
            return BIT_LOW;
        }
    }
    U64 starting_sample = mSWIM->GetSampleNumber();

    if (is_high_speed)
    {
        for (size_t j = 0; j < mHighSpeedSampleOffsets.size(); j++)
        {
            mSWIM->Advance(mHighSpeedSampleOffsets[j]);
            if (mSWIM->GetBitState() == BIT_LOW)
            {
                low_bit_num++;
            }
            else
            {
                high_bit_num++;
            }
        }
        if (low_bit_num > 0)
        {
            // ACK detected
            U64 ending_sample = mSWIM->GetSampleNumber();

            if (low_bit_num <= 4)
            {
                bit = BIT_HIGH;
                mResults->AddMarker((starting_sample + ending_sample) / 2, AnalyzerResults::Square, mSettings->mSWIMChannel);
            }
            else
            {
                bit = BIT_LOW;
                mResults->AddMarker((starting_sample + ending_sample) / 2, AnalyzerResults::ErrorSquare, mSettings->mSWIMChannel);
            }
        }
        else
        {
            // No ACK detected
            mSWIM->AdvanceToAbsPosition(starting_sample);
            bit = BIT_LOW;
        }
    }
    else
    {
        for (size_t j = 0; j < mLowSpeedSampleOffsets.size(); j++)
        {
            mSWIM->Advance(mLowSpeedSampleOffsets[j]);
            if (mSWIM->GetBitState() == BIT_LOW)
            {
                low_bit_num++;
            }
            else
            {
                high_bit_num++;
            }
        }

        if (low_bit_num > 0)
        {
            // ACK detected
            U64 ending_sample = mSWIM->GetSampleNumber();

            if (low_bit_num <= 8)
            {
                bit = BIT_HIGH;
                mResults->AddMarker((starting_sample + ending_sample) / 2, AnalyzerResults::Square, mSettings->mSWIMChannel);
            }
            else
            {
                bit = BIT_LOW;
                mResults->AddMarker((starting_sample + ending_sample) / 2, AnalyzerResults::ErrorSquare, mSettings->mSWIMChannel);
            }
        }
        else
        {
            // No ACK detected
            mSWIM->AdvanceToAbsPosition(starting_sample);
            bit = BIT_LOW;
        }
    }
    return bit;
}

void SWIMAnalyzer::ParseEntry()
{
    U64 starting_sample = mSWIM->GetSampleNumber();
    U64 ending_sample = 0;
    Frame frame;

    for (size_t i = 0; i < 17; i++)
    {
        // Skip 17 edges for 8 pulses
        mSWIM->AdvanceToNextEdge();
    }
    ending_sample = mSWIM->GetSampleNumber();

    frame.mStartingSampleInclusive = starting_sample;
    frame.mEndingSampleInclusive = ending_sample;
    frame.mData1 = 0;
    frame.mData2 = 0;
    frame.mType = EntryPulses;
    mResults->AddFrame(frame);

    // HSI calibration pulse
    mSWIM->AdvanceToNextEdge();
    starting_sample = mSWIM->GetSampleNumber();
    mSWIM->AdvanceToNextEdge();
    ending_sample = mSWIM->GetSampleNumber();

    frame.mStartingSampleInclusive = starting_sample;
    frame.mEndingSampleInclusive = ending_sample;
    frame.mData1 = GetSampleRate() / (ending_sample - starting_sample) * 128 * 2;
    frame.mData2 = 0;
    frame.mType = EntryHSICalibrate;
    mResults->AddFrame(frame);
    
    // WOTF Header
    mSWIM->AdvanceToNextEdge();
    ParseSWIM(false);
}

U32 SWIMAnalyzer::GenerateSimulationData(U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels)
{
    if (mSimulationInitilized == false) {
        mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData(minimum_sample_index, device_sample_rate, simulation_channels);
}


U32 SWIMAnalyzer::GetMinimumSampleRateHz()
{
    return mSettings->mHSIClockFreq; //we don't have any idea, depends on the SPI rate, etc.; return the lowest rate.
}

const char* SWIMAnalyzer::GetAnalyzerName() const
{
    return "SWIM";
}

const char* GetAnalyzerName()
{
    return "SWIM";
}

Analyzer* CreateAnalyzer()
{
    return new SWIMAnalyzer();
}

void DestroyAnalyzer(Analyzer* analyzer)
{
    delete analyzer;
}
