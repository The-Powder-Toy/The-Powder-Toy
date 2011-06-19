#ifndef LANG_H
#define LANG_H
//#include "defines.h"

#ifdef ENGLISH //English

char *beta_hud_text = "Me's Mod version %d.%d FPS:%d Parts:%d Generation:%d Gravity:%d Air:%d";
char *debug_hud_text = "Me's Mod FPS:%d Parts:%d Gen:%d Grav:%d Air:%d";
char *hud_text = "Me4502's Mod version %d.%d FPS:%d";
char *add_simulation_tags = "Add simulation tags.";
char *add_remove_simulation_tags = "Add and remove simulation tags.";
char *save_under_current_name = "Save the simulation under the current name.";
char *save_under_new_name = "Save the simulation under a new name.";
char *replace_mode_text = " [REPLACE MODE]";

#elif defined INDONESIAN //Indonesian

char *beta_hud_text = "Me's Mod versi %d.%d FPS:%d Bagian:%d Generation:%d Gravity:%d Udara:%d";
char *debug_hud_text = "Aku Mod FPS:%d Bagian:%d Gen:%d Grav:%d Udara:%d";
char *hud_text = "Me4502's Mod versi %d.%d FPS:%d";
char *add_simulation_tags = "Tambahkan tag simulasi.";
char *add_remove_simulation_tags = "Menambah dan menghapus tag simulasi.";
char *save_under_current_name = "Simpan simulasi dengan nama saat ini.";
char *save_under_new_name = "Simpan simulasi di bawah nama baru.";
#else //No Language Entered

char *beta_hud_text = "%d.%d %d %d %d %d %d";
char *debug_hud_text = "%d %d %d %d %d";
char *hud_text = "%d.%d %d";
char *add_simulation_tags = "";
char *add_remove_simulation_tags = "";
char *save_under_current_name = "";
char *save_under_new_name = "";

#endif
#endif
