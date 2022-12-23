#ifndef storage_h
#define storage_h

#include <Arduino.h>
#include <LittleFS.h>
#include <timer.h>

class Storage {
    private:

    public:
        Storage();

        bool begin();
        String readAll(const char *path);
        // writeFile ovewrites all file content.
        bool writeFile(const char *path, const char *message);
        bool appendFile(const char *path, const char *message);
        bool remove(const char *path);
        size_t fileSize(const char *path);
        bool exists(const char *path);
        File open(const char *path);
        File open(const char *path, const char *mode);
        String getSize();
        String getUsed();
        String getFree();
};

#endif
