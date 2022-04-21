#ifndef DISPLAY_TUNER_VERSION
#error Please define macro DISPLAY_TUNER_VERSION in CMakeList.txt
#endif

__attribute__((used, section("DISPLAY_TUNER_VERSION"), visibility("default")))
const char g_display_tuner_version[] = DISPLAY_TUNER_VERSION;
