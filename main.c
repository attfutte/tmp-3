#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

enum {
    GREE_OFF,
    GREE_ON,
};

enum {
    GREE_MODE_AUTO,
    GREE_MODE_COOL,
    GREE_MODE_DRY,
    GREE_MODE_FAN,
    GREE_MODE_HEAT,
};

enum {
    GREE_FAN_AUTO,
    GREE_FAN_LOW,
    GREE_FAN_MID,
    GREE_FAN_HIGH,
};

enum {
    GREE_LOUVER_OFF,
    GREE_LOUVER_FULL,
    GREE_LOUVER_TOP,
    GREE_LOUVER_MIDTOP,
    GREE_LOUVER_MID,
    GREE_LOUVER_MIDBOTTOM,
    GREE_LOUVER_BOTTOM,
    GREE_LOUVER_BOTTOM_3,
    GREE_LOUVER_MID_3 = 0x9,
    GREE_LOUVER_TOP_3 = 0xb,
};

enum {
    GREE_DISPMODE_OFF,
    GREE_DISPMODE_SETTING,
    GREE_DISPMODE_INDOOR,
    GREE_DISPMODE_OUTDOOR,
};

static const char *names[][12] = {
    { "GREE_OFF", "GREE_ON", },
    { "GREE_MODE_AUTO", "GREE_MODE_COOL", "GREE_MODE_DRY", "GREE_MODE_FAN", "GREE_MODE_HEAT", },
    { "GREE_FAN_AUTO", "GREE_FAN_LOW", "GREE_FAN_MID", "GREE_FAN_HIGH", },
    { "GREE_LOUVER_OFF", "GREE_LOUVER_FULL", "GREE_LOUVER_TOP", "GREE_LOUVER_MIDTOP", "GREE_LOUVER_MID",
      "GREE_LOUVER_MIDBOTTOM", "GREE_LOUVER_BOTTOM", "GREE_LOUVER_BOTTOM_3", "", "GREE_LOUVER_MID_3", "",
      "GREE_LOUVER_TOP_3", },
    { "GREE_DISP_MODE_OFF", "GREE_DISP_MODE_SETTING", "GREE_DISP_MODE_INDOOR", "GREE_DISP_MODE_OUTDOOR", },
};

typedef struct {
    uint32_t bits[2];
    uint8_t mode;
    uint8_t power;
    uint8_t fan;
    uint8_t louver;
    uint8_t temp;
    uint8_t turbo;
    uint8_t disp;
    uint8_t health;
    uint8_t xfan;
    uint8_t disp_mode;
} gree_t;

char *bstr(uint32_t v)
{
    char *s = (char *)malloc(sizeof(uint32_t) * 8 + 1);
    for (int i = 31; i >= 0; i--) s[31 - i] = v & (1 << i) ? '1' : '0';
    s[32] = 0;
    return s;
}

uint32_t brev(uint32_t v)
{
    uint32_t r = 0;
    uint8_t s = sizeof(v) * 8 - 1;

    for (int i = s; i >= 0; i--)
        r |= ((v & (1 << i)) >> i) << (s - i);

    return r;
}

int gree_init(gree_t *state)
{
    memset(state, 0, sizeof(gree_t));
    state->mode = GREE_MODE_AUTO;
    state->power = GREE_ON;
    state->fan = GREE_FAN_LOW;
    state->louver = GREE_LOUVER_OFF;
    state->temp = 25;
    state->turbo = GREE_OFF;
    state->disp = GREE_OFF;
    state->health = GREE_OFF;
    state->xfan = GREE_OFF;
    state->disp_mode = GREE_DISPMODE_OFF;

    return 0;
}

uint32_t *gree_bits(gree_t *state)
{
    state->bits[0] = state->bits[1] = 0;
    state->bits[0] |= 0x50000000;

    state->bits[0] |= state->mode;

    state->bits[0] |= state->power << 0x3;

    state->bits[0] |= state->fan << 0x4;

    if (state->louver == GREE_LOUVER_FULL ||
        state->louver == GREE_LOUVER_BOTTOM_3 ||
        state->louver == GREE_LOUVER_MID_3 ||
        state->louver == GREE_LOUVER_TOP_3)
        state->bits[0] |= 1 << 6;
    state->bits[1] |= state->louver;

    state->bits[0] |= (state->temp - 0x10) << 0x8;
    state->bits[1] |= ((state->temp + state->mode + 0x2) % 0x10) << 0x1c;

    state->bits[0] |= state->turbo << 0x14;

    state->bits[0] |= state->disp << 0x15;

    if (state->power == GREE_ON)
        state->bits[0] |= state->health << 0x16;

    state->bits[0] |= state->xfan << 0x17;

    state->bits[1] |= state->disp_mode << 0x8;

    state->bits[0] = brev(state->bits[0]);
    state->bits[1] = brev(state->bits[1]);

    if (state->power == GREE_OFF)
        state->bits[1] ^= 1;

    return state->bits;
}

void gree_dump(gree_t *state)
{
    printf("mode      = %s\n", names[1][state->mode]);
    printf("power     = %s\n", names[0][state->power]);
    printf("fan       = %s\n", names[2][state->fan]);
    printf("louver    = %s\n", names[3][state->louver]);
    printf("temp      = %dC\n", state->temp);
    printf("turbo     = %s\n", names[0][state->turbo]);
    printf("disp      = %s\n", names[0][state->disp]);
    printf("health    = %s\n", names[0][state->health]);
    printf("xfan      = %s\n", names[0][state->xfan]);
    printf("disp_mode = %s\n", names[4][state->disp_mode]);
}

int main(int argc, char *argv[])
{
    gree_t state;
    gree_init(&state);
    gree_dump(&state);

    int c, d, p;
    while ((c = getchar()) != 'q') {
        d = p = 0;
        switch (c) {
            case '1':
                state.mode = (state.mode + 1) % 5;
                d = 1;
                break;
            case '2':
                state.power = (state.power + 1) % 2;
                d = p = 1;
                break;
            case '3':
                state.fan = (state.fan + 1) % 4;
                d = 1;
                break;
            case '4':
                {
                    uint8_t l = state.louver + 1;
                    if (l == 8 || l == 10)
                        ++l;
                    state.louver = l % 12;
                    d = 1;
                }
                break;
            case '5':
                if (++state.temp > 30)
                    state.temp = 16;
                d = 1;
                break;
            case '6':
                state.turbo = (state.turbo + 1) % 2;
                d = 1;
                break;
            case '7':
                state.disp = (state.disp + 1) % 2;
                d = 1;
                break;
            case '8':
                state.health = (state.health + 1) % 2;
                d = 1;
                break;
            case '9':
                state.xfan = (state.xfan + 1) % 2;
                d = 1;
                break;
            case '0':
                state.disp_mode = (state.disp_mode + 1) % 4;
                d = 1;
                break;
            default:
                break;
        }

        if (d) {
            gree_dump(&state);

            uint32_t *bits = gree_bits(&state);
            printf("RCV:  .  - 0 [0: S%s0102%s0\n", bstr(bits[0]), bstr(bits[1]));
        }
    }

    return 0;
}
