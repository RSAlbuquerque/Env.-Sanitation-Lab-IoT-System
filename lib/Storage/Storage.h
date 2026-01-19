#pragma once

#include "Types.h"

#include <Arduino.h>
#include <Preferences.h>

class StorageManager {
  public:
    void saveCredentials(const char *identity, const char *username, const char *pass, const char *apiKey);
    UserCredentials loadCredentials();
    void clearCredentials();
};

extern StorageManager storage;