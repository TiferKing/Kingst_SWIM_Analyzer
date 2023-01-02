#pragma once
#ifndef SWIM_ANALYZER_SETTINGS
#define SWIM_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class SWIMAnalyzerSettings : public AnalyzerSettings
{
public:
    SWIMAnalyzerSettings();
    virtual ~SWIMAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    virtual void LoadSettings(const char* settings);
    virtual const char* SaveSettings();

    void UpdateInterfacesFromSettings();

    Channel mSWIMChannel;
    Channel mResetChannel;

    U32 mHSIClockFreq;  // Unit Hz
    U32 mLSIClockFreq;  // Unit Hz

protected:
    std::auto_ptr< AnalyzerSettingInterfaceChannel >    mSWIMChannelInterface;
    std::auto_ptr< AnalyzerSettingInterfaceChannel >    mResetChannelInterface;
    std::auto_ptr< AnalyzerSettingInterfaceInteger >    mHSIClockFreqInterface;
    std::auto_ptr< AnalyzerSettingInterfaceInteger >    mLSIClockFreqInterface;
};

#endif //SWIM_ANALYZER_SETTINGS