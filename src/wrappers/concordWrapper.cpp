// SPDX-License-Identifier: LGPL-2.1-or-later

#include "concordWrapper.h"
#include "libconcord.h"

namespace LibConcord
{

ConcordWrapper::ConcordWrapper(QObject *parent) :
    QObject(parent)
{
}
ConcordWrapper::~ConcordWrapper()
{
  if (deinitConcord() != 0) {
    // ignore deinit errors on destruction
  }
}

void ConcordWrapper::staticCallback(uint32_t stage, uint32_t count,
    uint32_t current, uint32_t total, uint32_t counterType, void *arg,
    const uint32_t *stages)
{
  ConcordWrapper *self = static_cast<ConcordWrapper*>(arg);
  if (self != nullptr) {
    emit self->progressUpdated(stage, count, current, total, counterType,
        stages);
  }
}

int ConcordWrapper::initConcord()
{
  return init_concord();
}

int ConcordWrapper::deinitConcord()
{
  return deinit_concord();
}

int ConcordWrapper::getIdentity()
{
  return get_identity(staticCallback, this);
}

const char* ConcordWrapper::getMfg()
{
  return get_mfg();
}

const char* ConcordWrapper::getModel()
{
  return get_model();
}

const char* ConcordWrapper::getCodename()
{
  return get_codename();
}

int ConcordWrapper::getSkin()
{
  return get_skin();
}

int ConcordWrapper::getFwVerMaj()
{
  return get_fw_ver_maj();
}

int ConcordWrapper::getFwVerMin()
{
  return get_fw_ver_min();
}

int ConcordWrapper::getFwType()
{
  return get_fw_type();
}

int ConcordWrapper::getHwVerMaj()
{
  return get_hw_ver_maj();
}

int ConcordWrapper::getHwVerMin()
{
  return get_hw_ver_min();
}

int ConcordWrapper::getHwVerMic()
{
  return get_hw_ver_mic();
}

int ConcordWrapper::getFlashSize()
{
  return get_flash_size();
}

int ConcordWrapper::getFlashMfg()
{
  return get_flash_mfg();
}

int ConcordWrapper::getFlashId()
{
  return get_flash_id();
}

const char* ConcordWrapper::getFlashPartNum()
{
  return get_flash_part_num();
}

int ConcordWrapper::getArch()
{
  return get_arch();
}

int ConcordWrapper::getProto()
{
  return get_proto();
}

const char* ConcordWrapper::getHidMfgStr()
{
  return get_hid_mfg_str();
}

const char* ConcordWrapper::getHidProdStr()
{
  return get_hid_prod_str();
}

int ConcordWrapper::getHidIrl()
{
  return get_hid_irl();
}

int ConcordWrapper::getHidOrl()
{
  return get_hid_orl();
}

int ConcordWrapper::getHidFrl()
{
  return get_hid_frl();
}

int ConcordWrapper::getUsbVid()
{
  return get_usb_vid();
}

int ConcordWrapper::getUsbPid()
{
  return get_usb_pid();
}

int ConcordWrapper::getUsbBcd()
{
  return get_usb_bcd();
}

char* ConcordWrapper::getSerial(bool p)
{
  return get_serial(p ? 1 : 0);
}

int ConcordWrapper::getConfigBytesUsed()
{
  return get_config_bytes_used();
}

int ConcordWrapper::getConfigBytesTotal()
{
  return get_config_bytes_total();
}

const char* ConcordWrapper::getRawErrorString(int err)
{
  return lc_strerror(err);
}

QString ConcordWrapper::errorToString(int err) const
{
  const char *raw = lc_strerror(err);
  if (raw != nullptr) {
    return QString::fromUtf8(raw);
  }
  return QString();
}

QString ConcordWrapper::stageToString(int stage) const
{
  const char *raw = lc_cb_stage_str(stage);
  if (raw != nullptr) {
    return QString::fromUtf8(raw);
  }
  return QString();
}

const char* ConcordWrapper::cbStageStr(int stage)
{
  return lc_cb_stage_str(stage);
}

void ConcordWrapper::freeBlob(uint8_t *ptr)
{
  if (ptr != nullptr) {
    delete_blob(ptr);
  }
}

int ConcordWrapper::readAndParseFile(const char *filename, int *type)
{
  if (filename != nullptr) {
    return read_and_parse_file(const_cast<char*>(filename), type);
  }
  return -1;
}

void ConcordWrapper::freeOpfileObj()
{
  delete_opfile_obj();
}

int ConcordWrapper::resetRemote()
{
  return reset_remote(staticCallback, this);
}

int ConcordWrapper::getTime()
{
  return get_time();
}

int ConcordWrapper::setTime()
{
  return set_time(staticCallback, this);
}

int ConcordWrapper::postConnectTestSuccess()
{
  return post_connect_test_success(staticCallback, this);
}

int ConcordWrapper::postPreconfig()
{
  return post_preconfig(staticCallback, this);
}

int ConcordWrapper::postPostconfig()
{
  return post_postconfig(staticCallback, this);
}

int ConcordWrapper::postPostfirmware()
{
  return post_postfirmware(staticCallback, this);
}

int ConcordWrapper::invalidateFlash()
{
  return invalidate_flash(staticCallback, this);
}

int ConcordWrapper::isConfigDumpSupported()
{
  return is_config_dump_supported();
}

int ConcordWrapper::isConfigUpdateSupported()
{
  return is_config_update_supported();
}

int ConcordWrapper::isFirmwareDumpSupported()
{
  return is_fw_dump_supported();
}

int ConcordWrapper::isFirmwareUpdateSupported(bool direct)
{
  return is_fw_update_supported(direct ? 1 : 0);
}

int ConcordWrapper::updateConfiguration(bool noreset)
{
  return update_configuration(staticCallback, this, noreset ? 1 : 0);
}

int ConcordWrapper::readConfigFromRemote(uint8_t **out, uint32_t *size)
{
  return read_config_from_remote(out, size, staticCallback, this);
}

int ConcordWrapper::writeConfigToRemote()
{
  return write_config_to_remote(staticCallback, this);
}

int ConcordWrapper::writeConfigToFile(const uint8_t *in, uint32_t size,
    const char *fileName, bool binary)
{
  if (in != nullptr && fileName != nullptr) {
    return write_config_to_file(const_cast<uint8_t*>(in), size,
        const_cast<char*>(fileName), binary ? 1 : 0);
  }
  return -1;
}

int ConcordWrapper::verifyRemoteConfig()
{
  return verify_remote_config(staticCallback, this);
}

int ConcordWrapper::prepConfig()
{
  return prep_config(staticCallback, this);
}

int ConcordWrapper::finishConfig()
{
  return finish_config(staticCallback, this);
}

int ConcordWrapper::eraseConfig()
{
  return erase_config(staticCallback, this);
}

int ConcordWrapper::eraseSafemode()
{
  return erase_safemode(staticCallback, this);
}

int ConcordWrapper::readSafemodeFromRemote(uint8_t **out, uint32_t *size)
{
  return read_safemode_from_remote(out, size, staticCallback, this);
}

int ConcordWrapper::writeSafemodeToFile(const uint8_t *in, uint32_t size,
    const char *fileName)
{
  if (in != nullptr && fileName != nullptr) {
    return write_safemode_to_file(const_cast<uint8_t*>(in), size,
        const_cast<char*>(fileName));
  }
  return -1;
}

int ConcordWrapper::isConfigSafeAfterFirmware()
{
  return is_config_safe_after_fw();
}

int ConcordWrapper::updateFirmware(bool noreset, bool direct)
{
  return update_firmware(staticCallback, this, noreset ? 1 : 0, direct ? 1 : 0);
}

int ConcordWrapper::prepFirmware()
{
  return prep_firmware(staticCallback, this);
}

int ConcordWrapper::finishFirmware()
{
  return finish_firmware(staticCallback, this);
}

int ConcordWrapper::eraseFirmware(bool direct)
{
  return erase_firmware(direct ? 1 : 0, staticCallback, this);
}

int ConcordWrapper::readFirmwareFromRemote(uint8_t **out, uint32_t *size)
{
  return read_firmware_from_remote(out, size, staticCallback, this);
}

int ConcordWrapper::writeFirmwareToRemote(bool direct)
{
  return write_firmware_to_remote(direct ? 1 : 0, staticCallback, this);
}

int ConcordWrapper::writeFirmwareToFile(const uint8_t *in, uint32_t size,
    const char *fileName, bool binary)
{
  if (in != nullptr && fileName != nullptr) {
    return write_firmware_to_file(const_cast<uint8_t*>(in), size,
        const_cast<char*>(fileName), binary ? 1 : 0);
  }
  return -1;
}

int ConcordWrapper::getKeyNames(char ***keyNames, uint32_t *length)
{
  if (keyNames != nullptr && length != nullptr) {
    return get_key_names(keyNames, length);
  }
  return -1;
}

void ConcordWrapper::freeKeyNames(char **keyNames, uint32_t length)
{
  if (keyNames != nullptr) {
    delete_key_names(keyNames, length);
  }
}

int ConcordWrapper::setLearningMode(int mode, uint32_t timeout_ms)
{
  return set_learning_mode(mode, timeout_ms);
}

int ConcordWrapper::learnFromRemote(uint32_t *carrierClock, uint32_t **irSignal,
    uint32_t *irSignalLength)
{
  return learn_from_remote(carrierClock, irSignal, irSignalLength,
      staticCallback, this);
}

void ConcordWrapper::freeIrSignal(uint32_t *irSignal)
{
  if (irSignal != nullptr) {
    delete_ir_signal(irSignal);
  }
}

int ConcordWrapper::encodeForPosting(uint32_t carrierClock,
    const uint32_t *irSignal, uint32_t irSignalLength, char **encodedSignal)
{
  if (irSignal != nullptr && encodedSignal != nullptr) {
    return encode_for_posting(carrierClock, const_cast<uint32_t*>(irSignal),
        irSignalLength, encodedSignal);
  }
  return -1;
}

void ConcordWrapper::freeEncodedSignal(char *encodedSignal)
{
  if (encodedSignal != nullptr) {
    delete_encoded_signal(encodedSignal);
  }
}

int ConcordWrapper::postNewCode(const char *keyName, const char *encodedSignal)
{
  if (keyName != nullptr && encodedSignal != nullptr) {
    return post_new_code(const_cast<char*>(keyName),
        const_cast<char*>(encodedSignal), staticCallback, this);
  }
  return -1;
}

int ConcordWrapper::mhGetCfgProperties(void *properties)
{
  if (properties != nullptr) {
    return mh_get_cfg_properties(
        static_cast<struct mh_cfg_properties*>(properties));
  }
  return -1;
}

int ConcordWrapper::mhSetCfgProperties(const void *properties)
{
  if (properties != nullptr) {
    return mh_set_cfg_properties(
        static_cast<const struct mh_cfg_properties*>(properties));
  }
  return -1;
}

int ConcordWrapper::mhGetWifiNetworks(void *networks)
{
  if (networks != nullptr) {
    return mh_get_wifi_networks(static_cast<struct mh_wifi_networks*>(networks));
  }
  return -1;
}

int ConcordWrapper::mhGetWifiConfig(void *config)
{
  if (config != nullptr) {
    return mh_get_wifi_config(static_cast<struct mh_wifi_config*>(config));
  }
  return -1;
}

int ConcordWrapper::mhSetWifiConfig(const void *config)
{
  if (config != nullptr) {
    return mh_set_wifi_config(static_cast<const struct mh_wifi_config*>(config));
  }
  return -1;
}

const char* ConcordWrapper::mhGetSerial()
{
  return mh_get_serial();
}

int ConcordWrapper::mhReadFile(const char *filename, uint8_t *buffer,
    uint32_t buflen, uint32_t *dataRead)
{
  if (filename != nullptr && buffer != nullptr && dataRead != nullptr) {
    return mh_read_file(filename, buffer, buflen, dataRead);
  }
  return -1;
}

int ConcordWrapper::mhWriteFile(const char *filename, const uint8_t *buffer,
    uint32_t buflen)
{
  if (filename != nullptr && buffer != nullptr) {
    return mh_write_file(filename, const_cast<uint8_t*>(buffer), buflen);
  }
  return -1;
}

int ConcordWrapper::getTimeSecond()
{
  return get_time_second();
}

int ConcordWrapper::getTimeMinute()
{
  return get_time_minute();
}

int ConcordWrapper::getTimeHour()
{
  return get_time_hour();
}

int ConcordWrapper::getTimeDay()
{
  return get_time_day();
}

int ConcordWrapper::getTimeDow()
{
  return get_time_dow();
}

int ConcordWrapper::getTimeMonth()
{
  return get_time_month();
}

int ConcordWrapper::getTimeYear()
{
  return get_time_year();
}

int ConcordWrapper::getTimeUtcOffset()
{
  return get_time_utc_offset();
}

const char* ConcordWrapper::getTimeTimezone()
{
  return get_time_timezone();
}

} // namespace LibConcord
