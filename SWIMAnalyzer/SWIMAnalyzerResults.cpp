#include "SWIMAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "SWIMAnalyzer.h"
#include "SWIMAnalyzerSettings.h"
#include <iostream>
#include <sstream>

//#pragma warning(disable: 4996) //warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.

SWIMAnalyzerResults::SWIMAnalyzerResults(SWIMAnalyzer* analyzer, SWIMAnalyzerSettings* settings)
    : AnalyzerResults(),
    mSettings(settings),
    mAnalyzer(analyzer)
{
}

SWIMAnalyzerResults::~SWIMAnalyzerResults()
{
}

void SWIMAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base)    //unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    Frame frame = GetFrame(frame_index);

    switch (frame.mType)
    {
    case EntryFrame:
        AddResultString("Entry Frame In Reset State");
        break;
    case EntryPulses:
        AddResultString("Entry Pulses");
        break;
    case EntryHSICalibrate:
        {
            std::stringstream ss;
            ss << "HSI Freq: " << frame.mData1 / 1e6 << "MHz";
            AddResultString(ss.str().c_str());
        }
        break;
    case SRSTFrame:
        if (frame.mFlags)
        {
            AddResultString("SRST [A]");
        }
        else
        {
            AddResultString("SRST [N]");
        }
        break;
    case ROTFFrame:
        if (frame.mFlags)
        {
            AddResultString("ROTF [A]");
        }
        else
        {
            AddResultString("ROTF [N]");
        }
        break;
    case WOTFFrame:
        if (frame.mFlags)
        {
            AddResultString("WOTF [A]");
        }
        else
        {
            AddResultString("WOTF [N]");
        }
        break;
    case DataLenFrame:
        if (frame.mFlags)
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "N: " << number_str << " [A]";
            AddResultString(ss.str().c_str());
        }
        else
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "N: " << number_str << " [N]";
            AddResultString(ss.str().c_str());
        }
        break;
    case AddressFrame:
        if (frame.mFlags)
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 24, number_str, 128);
            std::stringstream ss;
            ss << "Addr: " << number_str << " [A]";
            AddResultString(ss.str().c_str());
        }
        else
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 24, number_str, 128);
            std::stringstream ss;
            ss << "Addr: " << number_str << " [N]";
            AddResultString(ss.str().c_str());
        }
        break;
    case DataReadFrame:
        if (frame.mFlags)
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "RD: " << number_str << " [A]";
            AddResultString(ss.str().c_str());
        }
        else
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "RD: " << number_str << " [N]";
            AddResultString(ss.str().c_str());
        }
        break;
    case DataWriteFrame:
        if (frame.mFlags)
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "WD: " << number_str << " [A]";
            AddResultString(ss.str().c_str());
        }
        else
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "WD: " << number_str << " [N]";
            AddResultString(ss.str().c_str());
        }
        break;
    case ErrorFrame:
        if (frame.mFlags)
        {
            AddResultString("Error Frame [A]");
        }
        else
        {
            AddResultString("Error Frame [N]");
        }
        break;
    case UnknowFrame:
        if (frame.mFlags)
        {
            AddResultString("Unknow Frame [A]");
        }
        else
        {
            AddResultString("Unknow Frame [N]");
        }
        break;
    default:
        break;
    }
}

void SWIMAnalyzerResults::GenerateExportFile(const char* file, DisplayBase display_base, U32 /*export_type_user_id*/)
{
    //export_type_user_id is only important if we have more than one export type.

    std::stringstream ss;
    void* f = AnalyzerHelpers::StartFile(file);

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    ss << "Time [s],Frame ID,Type,Data,ACK" << std::endl;

    bool reset_used = true;

    if (mSettings->mResetChannel == UNDEFINED_CHANNEL)
    {
        reset_used = false;
    }

    U64 num_frames = GetNumFrames();
    for (U32 i = 0; i < num_frames; i++) {
        Frame frame = GetFrame(i);

        char time_str[128];
        AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128);

        ss << time_str << "," << i << ",";
        
        char data_str[128] = "";

        switch (frame.mType)
        {
        case EntryFrame:
            ss << "Entry," << ",," << std::endl;
            break;
        case EntryPulses:
            ss << "Entry," << ",," << std::endl;
            break;
        case EntryHSICalibrate:
            ss << "Freq," << frame.mData1 / 1e6 << ",," << std::endl;
            break;
        case SRSTFrame:
            ss << "SRST," << "," << ((frame.mFlags == 0) ? "N" : "A") << std::endl;
            break;
        case ROTFFrame:
            ss << "ROTF," << "," << ((frame.mFlags == 0) ? "N" : "A") << std::endl;
            break;
        case WOTFFrame:
            ss << "WOTF," << "," << ((frame.mFlags == 0) ? "N" : "A") << std::endl;
            break;
        case DataLenFrame:
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, data_str, 128);
            ss << "N," << data_str <<"," << ((frame.mFlags == 0) ? "N" : "A") << std::endl;
            break;
        case AddressFrame:
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 24, data_str, 128);
            ss << "ADDR," << data_str << "," << ((frame.mFlags == 0) ? "N" : "A") << std::endl;
            break;
        case DataReadFrame:
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, data_str, 128);
            ss << "RD," << data_str << "," << ((frame.mFlags == 0) ? "N" : "A") << std::endl;
            break;
        case DataWriteFrame:
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, data_str, 128);
            ss << "WD," << data_str << "," << ((frame.mFlags == 0) ? "N" : "A") << std::endl;
            break;
        case ErrorFrame:
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, data_str, 128);
            ss << "Error," << data_str << "," << ((frame.mFlags == 0) ? "N" : "A") << std::endl;
            break;
        case UnknowFrame:
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, data_str, 128);
            ss << "Unknown," << data_str << "," << ((frame.mFlags == 0) ? "N" : "A") << std::endl;
            break;
        default:
            break;
        }

        AnalyzerHelpers::AppendToFile((U8*)ss.str().c_str(), ss.str().length(), f);
        ss.str(std::string());

        if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true) {
            AnalyzerHelpers::EndFile(f);
            return;
        }
    }

    UpdateExportProgressAndCheckForCancel(num_frames, num_frames);
    AnalyzerHelpers::EndFile(f);
}

void SWIMAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
    ClearTabularText();
    Frame frame = GetFrame(frame_index);

    switch (frame.mType)
    {
    case EntryFrame:
        AddTabularText("Entry Frame In Reset State");
        break;
    case EntryPulses:
        AddTabularText("Entry Pulses");
        break;
    case EntryHSICalibrate:
    {
        std::stringstream ss;
        ss << "HSI Freq: " << frame.mData1 / 1e6 << "MHz";
        AddTabularText(ss.str().c_str());
    }
    break;
    case SRSTFrame:
        if (frame.mFlags)
        {
            AddTabularText("SRST [A]");
        }
        else
        {
            AddTabularText("SRST [N]");
        }
        break;
    case ROTFFrame:
        if (frame.mFlags)
        {
            AddTabularText("ROTF [A]");
        }
        else
        {
            AddTabularText("ROTF [N]");
        }
        break;
    case WOTFFrame:
        if (frame.mFlags)
        {
            AddTabularText("WOTF [A]");
        }
        else
        {
            AddTabularText("WOTF [N]");
        }
        break;
    case DataLenFrame:
        if (frame.mFlags)
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "N: " << number_str << " [A]";
            AddTabularText(ss.str().c_str());
        }
        else
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "N: " << number_str << " [N]";
            AddTabularText(ss.str().c_str());
        }
        break;
    case AddressFrame:
        if (frame.mFlags)
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 24, number_str, 128);
            std::stringstream ss;
            ss << "Addr: " << number_str << " [A]";
            AddTabularText(ss.str().c_str());
        }
        else
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 24, number_str, 128);
            std::stringstream ss;
            ss << "Addr: " << number_str << " [N]";
            AddTabularText(ss.str().c_str());
        }
        break;
    case DataReadFrame:
        if (frame.mFlags)
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "RD: " << number_str << " [A]";
            AddTabularText(ss.str().c_str());
        }
        else
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "RD: " << number_str << " [N]";
            AddTabularText(ss.str().c_str());
        }
        break;
    case DataWriteFrame:
        if (frame.mFlags)
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "WD: " << number_str << " [A]";
            AddTabularText(ss.str().c_str());
        }
        else
        {
            char number_str[128];
            AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
            std::stringstream ss;
            ss << "WD: " << number_str << " [N]";
            AddTabularText(ss.str().c_str());
        }
        break;
    case ErrorFrame:
        if (frame.mFlags)
        {
            AddTabularText("Error Frame [A]");
        }
        else
        {
            AddTabularText("Error Frame [N]");
        }
        break;
    case UnknowFrame:
        if (frame.mFlags)
        {
            AddTabularText("Unknow Frame [A]");
        }
        else
        {
            AddTabularText("Unknow Frame [N]");
        }
        break;
    default:
        break;
    }
}

void SWIMAnalyzerResults::GeneratePacketTabularText(U64 /*packet_id*/, DisplayBase /*display_base*/)    //unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    AddResultString("not supported");
}

void SWIMAnalyzerResults::GenerateTransactionTabularText(U64 /*transaction_id*/, DisplayBase /*display_base*/)    //unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    AddResultString("not supported");
}
