#pragma once

#include <QObject>
#include <QString>
#include <cstdint>

namespace LibConcord
{

//todo redefinition of the error codes defined in libconcord.h
//don't want to include the defines in the header...
constexpr int ErrorCodeError = 1;
constexpr int ErrorCodeInvalidDataFromRemote = 2;
constexpr int ErrorCodeRead = 3;
constexpr int ErrorCodeWrite = 4;
constexpr int ErrorCodeInvalidate = 5;
constexpr int ErrorCodeErase = 6;
constexpr int ErrorCodeVerify = 7;
constexpr int ErrorCodePost = 8;
constexpr int ErrorCodeGetTime = 9;
constexpr int ErrorCodeSetTime = 10;
constexpr int ErrorCodeConnect = 11;
constexpr int ErrorCodeOs = 12;
constexpr int ErrorCodeOsNet = 13;
constexpr int ErrorCodeOsFile = 14;
constexpr int ErrorCodeUnsupp = 15;
constexpr int ErrorCodeInvalidConfig = 16;
constexpr int ErrorCodeIrOverflow = 17;

constexpr int FileTypeConnectivity = 1;
constexpr int FileTypeConfiguration = 2;
constexpr int FileTypeFirmware = 3;
constexpr int FileTypeLearnIr = 4;

constexpr int LearnSingle = 0;
constexpr int LearnStream = 1;

constexpr int CallbackCounterTypeSteps = 5;
constexpr int CallbackCounterTypeBytes = 6;

constexpr int CallbackStageNumStages = 0xFF;
constexpr int CallbackStageGetIdentity = 7;
constexpr int CallbackStageInitializeUpdate = 8;
constexpr int CallbackStageInvalidateFlash = 9;
constexpr int CallbackStageEraseFlash = 10;
constexpr int CallbackStageWriteConfig = 11;
constexpr int CallbackStageVerifyConfig = 12;
constexpr int CallbackStageFinalizeUpdate = 13;
constexpr int CallbackStageReadConfig = 14;
constexpr int CallbackStageWriteFirmware = 15;
constexpr int CallbackStageReadFirmware = 16;
constexpr int CallbackStageReadSafemode = 17;
constexpr int CallbackStageReset = 18;
constexpr int CallbackStageSetTime = 19;
constexpr int CallbackStageHttp = 20;
constexpr int CallbackStageLearn = 21;

class ConcordWrapper: public QObject
{
  Q_OBJECT
  public:
    explicit ConcordWrapper(QObject *parent = nullptr);
    ~ConcordWrapper();

  signals:
    void progressUpdated(uint32_t stage, uint32_t count, uint32_t current,
        uint32_t total, uint32_t counterType, const uint32_t *stages);

  public:
    int initConcord();
    int deinitConcord();
    int getIdentity();

    const char* getMfg();
    const char* getModel();
    const char* getCodename();
    int getSkin();
    int getFwVerMaj();
    int getFwVerMin();
    int getFwType();
    int getHwVerMaj();
    int getHwVerMin();
    int getHwVerMic();
    int getFlashSize();
    int getFlashMfg();
    int getFlashId();
    const char* getFlashPartNum();
    int getArch();
    int getProto();
    const char* getHidMfgStr();
    const char* getHidProdStr();
    int getHidIrl();
    int getHidOrl();
    int getHidFrl();
    int getUsbVid();
    int getUsbPid();
    int getUsbBcd();
    char* getSerial(bool p);
    int getConfigBytesUsed();
    int getConfigBytesTotal();

    const char* getRawErrorString(int err);
    QString errorToString(int err) const;
    const char* cbStageStr(int stage);
    QString stageToString(int stage) const;
    void freeBlob(uint8_t *ptr);
    int readAndParseFile(const char *filename, int *type);
    void freeOpfileObj();

    int resetRemote();
    int getTime();
    int setTime();
    int postConnectTestSuccess();
    int postPreconfig();
    int postPostconfig();
    int postPostfirmware();
    int invalidateFlash();

    int isConfigDumpSupported();
    int isConfigUpdateSupported();
    int isFirmwareDumpSupported();
    int isFirmwareUpdateSupported(bool direct);

    int updateConfiguration(bool noreset);
    int readConfigFromRemote(uint8_t **out, uint32_t *size);
    int writeConfigToRemote();
    int writeConfigToFile(const uint8_t *in, uint32_t size,
        const char *fileName, bool binary);
    int verifyRemoteConfig();
    int prepConfig();
    int finishConfig();
    int eraseConfig();

    int eraseSafemode();
    int readSafemodeFromRemote(uint8_t **out, uint32_t *size);
    int writeSafemodeToFile(const uint8_t *in, uint32_t size, const char *fileName);

    int isConfigSafeAfterFirmware();
    int updateFirmware(bool noreset, bool direct);
    int prepFirmware();
    int finishFirmware();
    int eraseFirmware(bool direct);
    int readFirmwareFromRemote(uint8_t **out, uint32_t *size);
    int writeFirmwareToRemote(bool direct);
    int writeFirmwareToFile(const uint8_t *in, uint32_t size, const char *fileName, bool binary);

    int getKeyNames(char ***keyNames, uint32_t *length);
    void freeKeyNames(char **keyNames, uint32_t length);
    int setLearningMode(int mode, uint32_t timeout_ms);
    int learnFromRemote(uint32_t *carrierClock, uint32_t **irSignal, uint32_t *irSignalLength);
    void freeIrSignal(uint32_t *irSignal);
    int encodeForPosting(uint32_t carrierClock, const uint32_t *irSignal, uint32_t irSignalLength, char **encodedSignal);
    void freeEncodedSignal(char *encodedSignal);
    int postNewCode(const char *keyName, const char *encodedSignal);

    int mhGetCfgProperties(void *properties);
    int mhSetCfgProperties(const void *properties);
    int mhGetWifiNetworks(void *networks);
    int mhGetWifiConfig(void *config);
    int mhSetWifiConfig(const void *config);
    const char* mhGetSerial();
    int mhReadFile(const char *filename, uint8_t *buffer, uint32_t buflen, uint32_t *dataRead);
    int mhWriteFile(const char *filename, const uint8_t *buffer, uint32_t buflen);

    int getTimeSecond();
    int getTimeMinute();
    int getTimeHour();
    int getTimeDay();
    int getTimeDow();
    int getTimeMonth();
    int getTimeYear();
    int getTimeUtcOffset();
    const char* getTimeTimezone();

  private:
    static void staticCallback(uint32_t stage, uint32_t count, uint32_t current,
        uint32_t total, uint32_t counterType, void *arg,
        const uint32_t *stages);
};

} // namespace LibConcord
