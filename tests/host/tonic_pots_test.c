/* host test: Tonic's knobs (pots) over the 16-bit store: defaults, round trips, precision kept */
#include <stdio.h>
#include "dsp/tonic.h"
int main(void)
{
    static tonic_t T;
    int bad = 0, i, pot, v;
    tonic_init(&T, 44100);
    for (i = 0; i < TN_NPOTS; i++) {
        const x0x_param_t *d = tonic_param(0, i);
        if (i < TN_NPARAMS && d->def != tonic_val_to_pot(i, tonic_init_val(i))) {
            printf("default of %s: %d, shows %d\n", d->name, d->def, tonic_val_to_pot(i, tonic_init_val(i)));
            bad++;
        }
        for (pot = 0; pot <= d->max && i < TN_NPARAMS; pot++)
            if (tonic_val_to_pot(i, tonic_pot_to_val(i, pot)) != pot) {
                printf("round trip %s pot %d -> %d\n", d->name, pot, tonic_val_to_pot(i, tonic_pot_to_val(i, pot)));
                bad++;
            }
    }
    /* a loaded value keeps its precision while the pot is re-applied unchanged; moved, it takes the pot's */
    tonic_set(&T, 3, TNP_OSCFREQ, 31234);
    tonic_set_pot(&T, 3, TNP_OSCFREQ, tonic_val_to_pot(TNP_OSCFREQ, 31234));
    if (T.p[3][TNP_OSCFREQ] != 31234) { printf("precision lost\n"); bad++; }
    tonic_set_pot(&T, 3, TNP_OSCFREQ, 70);
    if (tonic_val_to_pot(TNP_OSCFREQ, T.p[3][TNP_OSCFREQ]) != 70) { printf("pot move not taken\n"); bad++; }
    tonic_set_pot(&T, 2, TN_POT_PAN, 10);
    if (T.pan[2] != 10) { printf("pan\n"); bad++; }
    for (v = 8; v < 11; v++) if (tonic_nparams(v) != 0) { printf("voice %d\n", v); bad++; }
    printf(bad ? "FAIL %d\n" : "tonic pots ok\n", bad);
    return bad != 0;
}
