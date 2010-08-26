#ifndef INTERFACE_H
#define INTERFACE_H

struct menu_section
{
    char *icon;
    const char *name;
    int itemcount;
};
typedef struct menu_section menu_section;

struct menu_wall
{
    pixel colour;
    const char *descs;
};
typedef struct menu_wall menu_wall;

static menu_wall mwalls[] =
{
    {PIXPACK(0xC0C0C0), "Wall. Indestructible. Blocks everything. Conductive."},
    {PIXPACK(0x808080), "E-Wall. Becomes transparent when electricity is connected."},
    {PIXPACK(0xFF8080), "Detector. Generates electricity when a particle is inside."},
    {PIXPACK(0x808080), "Streamline. Set start point of a streamline."},
    {PIXPACK(0x808080), "Sign. Click on a sign to edit it or anywhere else to place a new one."},
    {PIXPACK(0x8080FF), "Fan. Accelerates air. Use line tool to set direction and strength."},
    {PIXPACK(0xC0C0C0), "Wall. Blocks most particles but lets liquids through. Conductive."},
    {PIXPACK(0x808080), "Wall. Absorbs particles but lets air currents through."},
    {PIXPACK(0x808080), "Erases walls."},
    {PIXPACK(0x808080), "Wall. Indestructible. Blocks everything."},
    {PIXPACK(0x3C3C3C), "Wall. Indestructible. Blocks particles, allows air"},
    {PIXPACK(0x575757), "Wall. Indestructible. Blocks liquids and gasses, allows solids"},
    {PIXPACK(0xFFFF22), "Conductor, allows particles, conducts electricity"},
    {PIXPACK(0x242424), "E-Hole, absorbs particles, release them when powered"},
    {PIXPACK(0xFFFFFF), "Air, creates airflow and pressure"},
    {PIXPACK(0xFFBB00), "Heats the targetted element."},
    {PIXPACK(0x00BBFF), "Cools the targetted element."},
    {PIXPACK(0x303030), "Vacuum, reduces air pressure."},
    {PIXPACK(0x579777), "Wall. Indestructible. Blocks liquids and solids, allows gasses"},
};

#define SC_WALL 0
#define SC_SPECIAL 8
#define SC_POWDERS 5
#define SC_SOLIDS 6
#define SC_ELEC 1
#define SC_EXPLOSIVE 2
#define SC_GAS 3
#define SC_LIQUID 4
#define SC_NUCLEAR 7
#define SC_TOTAL 9

static menu_section msections[] =
{
    {"\xC1", "Walls", 0},
    {"\xC2", "Electronics", 0},
    {"\xC3", "Explosives", 0},
    {"\xC5", "Gasses", 0},
    {"\xC4", "Liquids", 0},
    {"\xD0", "Powders", 0},
    {"\xD1", "Solids", 0},
    {"\xC6", "Radioactive", 0},
    {"\xCC", "Special", 0},
};

void menu_count(void);
#endif