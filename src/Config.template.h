#pragma once
#include "VcsTag.h"

constexpr bool SET_WINDOW_ICON          = @SET_WINDOW_ICON@;
constexpr bool DEBUG                    = @DEBUG@;
constexpr bool X86                      = @X86@;
constexpr bool BETA                     = @BETA@;
constexpr bool SNAPSHOT                 = @SNAPSHOT@;
constexpr bool MOD                      = @MOD@;
constexpr bool NOHTTP                   = @NOHTTP@;
constexpr bool LUACONSOLE               = @LUACONSOLE@;
constexpr bool ALLOW_FAKE_NEWER_VERSION = @ALLOW_FAKE_NEWER_VERSION@;
constexpr bool USE_UPDATESERVER         = @USE_UPDATESERVER@;
constexpr bool CAN_INSTALL              = @CAN_INSTALL@;
constexpr bool USE_BLUESCREEN           = @USE_BLUESCREEN@;
constexpr bool INSTALL_CHECK            = @INSTALL_CHECK@;
constexpr bool IGNORE_UPDATES           = @IGNORE_UPDATES@;
constexpr bool ENFORCE_HTTPS            = @ENFORCE_HTTPS@;
constexpr bool SECURE_CIPHERS_ONLY      = @SECURE_CIPHERS_ONLY@;
constexpr bool USE_SYSTEM_CERT_PROVIDER = @USE_SYSTEM_CERT_PROVIDER@;
constexpr bool FFTW_PLAN_MEASURE        = @FFTW_PLAN_MEASURE@;
constexpr bool ALLOW_QUIT               = @ALLOW_QUIT@;
constexpr bool ALLOW_WINDOW_FRAME_OPS   = @ALLOW_WINDOW_FRAME_OPS@;
constexpr bool ALLOW_DATA_FOLDER        = @ALLOW_DATA_FOLDER@;
constexpr char PATH_SEP_CHAR            = '@PATH_SEP_CHAR@';

constexpr char SERVER[]         = "@SERVER@";
constexpr char STATICSERVER[]   = "@STATICSERVER@";
constexpr char UPDATESERVER[]   = "@UPDATESERVER@";
constexpr char IDENT_PLATFORM[] = "@IDENT_PLATFORM@";
constexpr char IDENT[]          = "@IDENT@";
constexpr char APPNAME[]        = "@APPNAME@";
constexpr char APPCOMMENT[]     = "@APPCOMMENT@";
constexpr char APPEXE[]         = "@APPEXE@";
constexpr char APPID[]          = "@APPID@";
constexpr char APPDATA[]        = "@APPDATA@";
constexpr char APPVENDOR[]      = "@APPVENDOR@";

constexpr int SAVE_VERSION         = 97;
constexpr int MINOR_VERSION        = 0;
constexpr int BUILD_NUM            = 352;
constexpr int SNAPSHOT_ID          = @SNAPSHOT_ID@;
constexpr int MOD_ID               = @MOD_ID@;
constexpr int FUTURE_SAVE_VERSION  = 98;
constexpr int FUTURE_MINOR_VERSION = 0;

constexpr char IDENT_RELTYPE    = SNAPSHOT ? 'S' : (BETA ? 'B' : 'R');

constexpr char SCHEME[]         = "https://";
constexpr char STATICSCHEME[]   = "https://";
constexpr char LOCAL_SAVE_DIR[] = "Saves";
constexpr char STAMPS_DIR[]     = "stamps";
constexpr char BRUSH_DIR[]      = "Brushes";

constexpr int httpMaxConcurrentStreams = 50;
constexpr int httpConnectTimeoutS      = 15;
