#include "SWIMAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>

SWIMAnalyzerSettings::SWIMAnalyzerSettings()
    : mSWIMChannel(UNDEFINED_CHANNEL),
    mResetChannel(UNDEFINED_CHANNEL),
    mHSIClockFreq(16000000),
    mLSIClockFreq(128000)
{
    mSWIMChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
    mSWIMChannelInterface->SetTitleAndTooltip("SWIM", "Single Wire Interface Module");
    mSWIMChannelInterface->SetChannel(mSWIMChannel);
    mSWIMChannelInterface->SetSelectionOfNoneIsAllowed(true);

    mResetChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
    mResetChannelInterface->SetTitleAndTooltip("RST", "CPU Reset Line");
    mResetChannelInterface->SetChannel(mResetChannel);
    mResetChannelInterface->SetSelectionOfNoneIsAllowed(true);

    mHSIClockFreqInterface.reset(new AnalyzerSettingInterfaceInteger());
    mHSIClockFreqInterface->SetTitleAndTooltip("HSI Clock Freq.", "Specify the HSI clock frequency.");
    mHSIClockFreqInterface->SetMax(25000000);
    mHSIClockFreqInterface->SetMin(10000000);
    mHSIClockFreqInterface->SetInteger(mHSIClockFreq);

    mLSIClockFreqInterface.reset(new AnalyzerSettingInterfaceInteger());
    mLSIClockFreqInterface->SetTitleAndTooltip("LSI Clock Freq.", "Specify the LSI clock frequency.");
    mLSIClockFreqInterface->SetMax(200000);
    mLSIClockFreqInterface->SetMin(30000);
    mLSIClockFreqInterface->SetInteger(mLSIClockFreq);


    AddInterface(mSWIMChannelInterface.get());
    AddInterface(mResetChannelInterface.get());
    AddInterface(mHSIClockFreqInterface.get());
    AddInterface(mLSIClockFreqInterface.get());

    AddExportOption(0, "Export as text/csv file");
    AddExportExtension(0, "Text file", "txt");
    AddExportExtension(0, "CSV file", "csv");

    ClearChannels();
    AddChannel(mSWIMChannel, "SWIM", false);
    AddChannel(mResetChannel, "RST", false);
}

    SWIMAnalyzerSettings::~SWIMAnalyzerSettings()
{
}

bool SWIMAnalyzerSettings::SetSettingsFromInterfaces()
{
    Channel swim = mSWIMChannelInterface->GetChannel();
    Channel rst = mResetChannelInterface->GetChannel();

    std::vector<Channel> channels;
    channels.push_back(swim);
    channels.push_back(rst);

    if (AnalyzerHelpers::DoChannelsOverlap(&channels[0], channels.size()) == true) {
        SetErrorText("Please select different channels for each input.");
        return false;
    }

    if (swim == UNDEFINED_CHANNEL) {
        SetErrorText("Please select at least one input for SWIM.");
        return false;
    }

    mSWIMChannel = mSWIMChannelInterface->GetChannel();
    mResetChannel = mResetChannelInterface->GetChannel();

    mHSIClockFreq = mHSIClockFreqInterface->GetInteger();
    mLSIClockFreq = mLSIClockFreqInterface->GetInteger();

    ClearChannels();
    AddChannel(mSWIMChannel, "SWIM", mSWIMChannel != UNDEFINED_CHANNEL);
    AddChannel(mResetChannel, "RST", mResetChannel != UNDEFINED_CHANNEL);

    return true;
}

void SWIMAnalyzerSettings::LoadSettings(const char* settings)
{
    SimpleArchive text_archive;
    text_archive.SetString(settings);

    const char* name_string;  //the first thing in the archive is the name of the protocol analyzer that the data belongs to.
    text_archive >> &name_string;
    if (strcmp(name_string, "KingstSpiAnalyzer") != 0) {
        AnalyzerHelpers::Assert("Kingst: Provided with a settings string that doesn't belong to us;");
    }

    text_archive >> mSWIMChannel;
    text_archive >> mResetChannel;
    text_archive >> mHSIClockFreq;
    text_archive >> mLSIClockFreq;

    ClearChannels();
    AddChannel(mSWIMChannel, "SWIM", mSWIMChannel != UNDEFINED_CHANNEL);
    AddChannel(mResetChannel, "RST", mResetChannel != UNDEFINED_CHANNEL);

    UpdateInterfacesFromSettings();
}

const char* SWIMAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << "KingstSpiAnalyzer";
    text_archive << mSWIMChannel;
    text_archive << mResetChannel;
    text_archive << mHSIClockFreq;
    text_archive << mLSIClockFreq;

    return SetReturnString(text_archive.GetString());
}

void SWIMAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mSWIMChannelInterface->SetChannel(mSWIMChannel);
    mResetChannelInterface->SetChannel(mResetChannel);
    mHSIClockFreqInterface->SetInteger(mHSIClockFreq);
    mLSIClockFreqInterface->SetInteger(mLSIClockFreq);
}