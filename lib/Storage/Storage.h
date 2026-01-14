#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "Types.h"

class StorageManager {
public:
    void saveCredentials(const char* identity, const char* username, const char* pass, const char* apiKey);
    UserCredentials loadCredentials();
    void clearCredentials();
};

extern StorageManager storage;