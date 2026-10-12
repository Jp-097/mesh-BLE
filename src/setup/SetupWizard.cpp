//setup auth username//

#include "setup/SetupWizard.h"

static Preferences prefs;

bool SetupWizard::isConfigured() {
    prefs.begin("cfg", true);
    bool ok = prefs.isKey("name");
    prefs.end();
    return ok;
}

void SetupWizard::saveName(const char* name) {
    prefs.begin("cfg", false);
    prefs.putString("name", name);
    prefs.end();
}