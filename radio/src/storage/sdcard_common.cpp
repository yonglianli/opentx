/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"
#include "storage.h"
#include "sdcard_common.h"
#include "modelslist.h"
#include "conversions/conversions.h"

// defined either in sdcard_raw.cpp or sdcard_yaml.cpp
void storageCreateModelsList();

void getModelPath(char * path, const char * filename)
{
  strcpy(path, STR_MODELS_PATH);
  path[sizeof(MODELS_PATH)-1] = '/';
  strcpy(&path[sizeof(MODELS_PATH)], filename);
}

void storageEraseAll(bool warn)
{
  TRACE("storageEraseAll");

#if defined(LIBOPENUI)
  // the theme has not been loaded before
  static_cast<ThemeBase*>(theme)->load();
#endif

  generalDefault();
  modelDefault(1);

  if (warn) {
    ALERT(STR_STORAGE_WARNING, STR_BAD_RADIO_DATA, AU_BAD_RADIODATA);
  }

  RAISE_ALERT(STR_STORAGE_WARNING, STR_STORAGE_FORMAT, NULL, AU_NONE);

  storageFormat();
  storageDirty(EE_GENERAL|EE_MODEL);
  storageCheck(true);
}

void storageFormat()
{
  sdCheckAndCreateDirectory(RADIO_PATH);
  sdCheckAndCreateDirectory(MODELS_PATH);
  storageCreateModelsList();
}

void storageCheck(bool immediately)
{
  if (storageDirtyMsk & EE_GENERAL) {
    TRACE("eeprom write general");
    storageDirtyMsk -= EE_GENERAL;
    const char * error = writeGeneralSettings();
    if (error) {
      TRACE("writeGeneralSettings error=%s", error);
    }
  }

  if (storageDirtyMsk & EE_MODEL) {
    TRACE("eeprom write model");
    storageDirtyMsk -= EE_MODEL;
    const char * error = writeModel();
    if (error) {
      TRACE("writeModel error=%s", error);
    }
  }
}

const char * createModel()
{
  preModelLoad();

  char filename[LEN_MODEL_FILENAME+1];
  memset(filename, 0, sizeof(filename));
  strcpy(filename, MODEL_FILENAME_PATTERN);

  int index = findNextFileIndex(filename, LEN_MODEL_FILENAME, MODELS_PATH);
  if (index > 0) {
    modelDefault(index);
    memcpy(g_eeGeneral.currModelFilename, filename, sizeof(g_eeGeneral.currModelFilename));
    storageDirty(EE_GENERAL);
    storageDirty(EE_MODEL);
    storageCheck(true);
  }
  postModelLoad(false);

  return g_eeGeneral.currModelFilename;
}

const char * loadModel(const char * filename, bool alarms)
{
  uint8_t version;
  
  preModelLoad();

  const char * error = readModel(filename, (uint8_t *)&g_model, sizeof(g_model), &version);
  if (error) {
    TRACE("loadModel error=%s", error);
  }
  
  if (error) {
    modelDefault(0) ;
    storageCheck(true);
    alarms = false;
  }
  else if (version < EEPROM_VER) {
    convertModelData(version);
  }

  postModelLoad(alarms);

  return error;
}

void storageReadAll()
{
  TRACE("storageReadAll");

  if (loadRadioSettings() != nullptr) {
    storageEraseAll(true);
  }

  for (uint8_t i = 0; languagePacks[i] != nullptr; i++) {
    if (!strncmp(g_eeGeneral.ttsLanguage, languagePacks[i]->id, 2)) {
      currentLanguagePackIdx = i;
      currentLanguagePack = languagePacks[i];
    }
  }

  if (loadModel(g_eeGeneral.currModelFilename, false) != nullptr) {
    sdCheckAndCreateDirectory(MODELS_PATH);
    createModel();
  }

  // Wipe models list in case
  // it's being reloaded after USB connection
  modelslist.clear();

  // and reload the list
  modelslist.load();
}

