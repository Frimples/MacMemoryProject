// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <string.h>

#include "platforms/platforms.h"
#include "m68k.h"

static int setup_platform_macse(struct emulator_config *cfg) {
    int ram_index = -1;
    for (int i = 0; i < MAX_NUM_MAPPED_ITEMS; i++) {
        if (cfg->map_type[i] == MAPTYPE_RAM_WTC &&
            cfg->map_offset[i] == 0 && cfg->map_high[i] >= 0x400000 && cfg->map_data[i]) {
            ram_index = i;
            break;
        }
    }
    if (ram_index < 0 || cfg->physical_write_range_count == 0)
        return 0;

    unsigned int cursor = 0;
    for (unsigned int n = 0; n < cfg->physical_write_range_count; n++) {
        unsigned int low = cfg->physical_write_low[n];
        unsigned int high = cfg->physical_write_high[n];
        if (low < cursor || high > cfg->map_high[ram_index])
            continue;
        if (cursor < low)
            m68k_add_write_range(cursor, low, cfg->map_data[ram_index] + cursor);
        cursor = high;
    }
    if (cursor < cfg->map_high[ram_index])
        m68k_add_write_range(cursor, cfg->map_high[ram_index],
            cfg->map_data[ram_index] + cursor);
    return 0;
}

void create_platform_macse(struct platform_config *cfg, char *subsys) {
    cfg->id = PLATFORM_MACSE;
    cfg->custom_read = NULL;
    cfg->custom_write = NULL;
    cfg->register_read = NULL;
    cfg->register_write = NULL;
    cfg->platform_initial_setup = setup_platform_macse;

    if (subsys) {
        cfg->subsys = malloc(strlen(subsys) + 1);
        strcpy(cfg->subsys, subsys);
    }
}
