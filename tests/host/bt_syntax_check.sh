#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# HOST SYNTAX CHECK of the Classic Bluetooth stack (firmware/src/bt/br/bt_inq.c, which pulls in bt_inq_core, bt_inq_con,
# bt_page, bt_acl, bt_l2cap, bt_sdps, bt_avrcp, bt_avdtp, bt_sbc, bt_audio, bta_mem, ...) against the radio headers copied
# into firmware/src/bt/hal/. Only gcc -fsyntax-only: nothing is linked, nothing runs. The glue's names come from
# tests/host/bt_stubs/bt_glue_stub.h (a stand-in, NOT the real glue). Two builds: AFH on and off.
#   tests/host/bt_syntax_check.sh
set -eu
cd "$(dirname "$0")/../.."
CC="${CC:-cc}"
G=tests/host/bt_stubs
mkdir -p "$G/br"
for afh in 1 0; do
    (cd firmware/src/bt/br && "$CC" -fsyntax-only -w -include "../../../../$G/bt_glue_stub.h" \
        -DFELUCCA_BT_AUDIO=1 -DFELUCCA_FLASH=1 -DFELUCCA_BLE=1 -DFELUCCA_BT_DIAG=0 -DFELUCCA_BT_AFH=$afh -DFELUCCA_DUAL=0 -DIN_POOL= \
        -I../hal -I"../../../../$G" -I"../../../../$G/br" -I../../../hal bt_inq.c)
    echo "bt_inq.c syntax ok (AFH=$afh)"
done
