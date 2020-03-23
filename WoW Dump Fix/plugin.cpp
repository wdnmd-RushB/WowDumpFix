#include "plugin.h"
#include "fix_dump.h"

Debuggee debuggee;

// Plugin exported command.
static const char cmdWoWDumpFix[] = "WoWDumpFix";
static const char WoWTargetVersion[] = "X.XX.XXX.XXXX";
static const char realPluginVersion[] = "v5.0.0";
static const char authorName[] = "changeofpace (WoW mod by Zylla)";
static const char githubSourceURL[] = R"(https://github.com/adde88/WoW-Dump-Fix)";

///////////////////////////////////////////////////////////////////////////////
// Added Commands

static bool cbWoWDumpFix(int argc, char* argv[])
{
    //暂停所有线程
    DbgCmdExec("suspendallthreads");
    Sleep(1000);
    pluginLog("Executing %s %s.\n", PLUGIN_NAME, realPluginVersion);
    pluginLog("This plugin is updated for World of Warcraft version %s.\n", WoWTargetVersion);
    if (!fixdump::current::FixWoW()) {
        pluginLog("Failed to complete. Open an issue on GitHub with the error message(s) and log output:\n");
        pluginLog("    %s\n", githubSourceURL);
        DbgCmdExec("resumeallthreads");
        return false;
    }
    pluginLog("Completed successfully! Use Scylla to dump WowClassic.exe.\n");
    pluginLog("After All. call x64dbg command \"resumeallthreads\"\n");
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// x64dbg

PLUG_EXPORT void CBCREATEPROCESS(CBTYPE cbType, PLUG_CB_CREATEPROCESS* Info)
{
    static const char WoWModuleName[] = "\\WowClassic.exe";
    if (!strstr(Info->modInfo->ModuleName, WoWModuleName)) {

        debuggee.hProcess = Info->fdProcessInfo->hProcess;
        debuggee.imageBase = Info->modInfo->BaseOfImage;
    }
}

enum { PLUGIN_MENU_ABOUT };

PLUG_EXPORT void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    switch (info->hEntry)
    {
    case PLUGIN_MENU_ABOUT: {
        const int maxMessageBoxStringSize = 1024;
		char buf[maxMessageBoxStringSize] = "";

		_snprintf_s(buf, maxMessageBoxStringSize, _TRUNCATE,
                    "Author:  %s.\n\nsource code:  %s.",
                    authorName, githubSourceURL);

        MessageBoxA(hwndDlg, buf, "About", 0);
    }
    break;
    }
}

bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
    if (!_plugin_registercommand(pluginHandle, cmdWoWDumpFix, cbWoWDumpFix, true)) {
        pluginLog("failed to register command %s.\n", cmdWoWDumpFix);
        return false;
    }
    return true;
}

bool pluginStop()
{
    _plugin_menuclear(hMenu);
    _plugin_unregistercommand(pluginHandle, cmdWoWDumpFix);
    return true;
}

void pluginSetup()
{
    _plugin_menuaddentry(hMenu, PLUGIN_MENU_ABOUT, "&About");
}

void pluginLog(const char* Format, ...)
{
    va_list valist;
    char buf[MAX_STRING_SIZE];
    RtlZeroMemory(buf, MAX_STRING_SIZE);

    _snprintf_s(buf, MAX_STRING_SIZE, _TRUNCATE, "[%s] ", PLUGIN_NAME);

    va_start(valist, Format);
    _vsnprintf_s(buf + strlen(buf), sizeof(buf) - strlen(buf), _TRUNCATE, Format, valist);
    va_end(valist);

    _plugin_logputs(buf);
}