#include "Storage.h"

StorageManager storage;

void StorageManager::saveCredentials(const char* identity, const char* username, const char* pass, const char* apiKey) {
    Preferences preferences;
    preferences.begin("credentials", false);
    
    preferences.putString("identity", identity);
    preferences.putString("username", username);
    preferences.putString("password", pass);
    preferences.putString("apiKey", apiKey);
    
    preferences.end();
}

UserCredentials StorageManager::loadCredentials() {
    UserCredentials creds;
    Preferences preferences;
    
    preferences.begin("credentials", true);
    
    creds.identity = preferences.getString("identity", "");
    creds.username = preferences.getString("username", "");
    creds.password = preferences.getString("password", "");
    creds.apiKey   = preferences.getString("apiKey", "");
    
    preferences.end();

    // If username is empty, we assume data is invalid/missing
    if (creds.username == "") {
        creds.valid = false;
    } else {
        creds.valid = true;
    }

    return creds;
}

void StorageManager::clearCredentials() {
    Preferences preferences;
    preferences.begin("credentials", false);
    preferences.clear();
    preferences.end();
}