# disp-regdump

Generates `drivers/video/sigmastar/ms_disp_tbl.h`: the register program
SigmaStar's own infinity2m display and HDMI TX drivers write to bring up
HDMI + VGA at one output mode, as the vendor U-Boot's `cmd_bootlogo.c`
drives them. Those drivers (about 250 KB of code with the vendor's libjpeg)
do not fit in U-Boot's 128 KiB slot; their output, a few KB of tables, does.

`make` extracts `drivers/mstar/{disp,hdmitx}` from the SSR621 boot release
(`BOOT_TGZ=boot_621_release_20200522.tar.gz`), points their register
accessors at a recorder (`patch.sh`, `rec.c`, `hdmi_rec.c`) and builds a
static ARM program (the vendor code assumes 32-bit pointers). Run it on any
ARM Linux, one log per mode, then:

    ./regdump 1080p60 > out/1080p60.log      # ... for each mode
    ./gen-tables.py ../../../drivers/video/sigmastar/ms_disp_tbl.h \
        out/1080p60.log out/1080p50.log out/720p60.log out/720p50.log \
        out/1280x1024.log out/1024x768.log out/1440x900.log

The program writes masked writes as masked (U-Boot replays them as
read-modify-write against the hardware's reset values), keeps delays, and
leaves out the MOP window (`ms_disp_show()` sets it per picture) and the
HDMI TX analog trims (ATOP 0x19/0x1A/0x38: the IPL writes each chip's efuse
values there). Checked against a vendor U-Boot (Xiongmai NVR) traced in
qemu-bench: the display, OP2, DAC, MOP-rotate and HDMI TX banks match.
