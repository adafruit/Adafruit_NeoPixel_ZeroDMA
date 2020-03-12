# Oh NO.
# The MUX table is different for "L" variant devices!
# Table 7-2 is a different thing, I'd appended it as if
# it's part of 7-1.

# SERCOM-pin-finding tool for Adafruit_NeoPixel_ZeroDMA.
# Scans each item in the Arduino boards.txt file, comparing a board's
# assigned pins against a table of pins from the SAMD21 & SAMD51 datasheets,
# looking for items that can work as SPI SERCOM MOSI pins, generating code
# for the Adafruit_NeoPixel_ZeroDMA library. THIS IS ONLY A STARTING POINT
# and THE OUTPUT REQUIRES SIGNIFICANT EDITING.
# First, the #ifdef for each board must be cleaned up. Right now it just
# dumps the entirety of a board's .build.extra_flags string, not knowing
# which element actually uniquely identifies a board.
# e.g. this is what's output for the Feather M0:
# #if defined(-DARDUINO_SAMD_ZERO -DARM_MATH_CM0PLUS -DADAFRUIT_FEATHER_M0 ...
# ...which needs to be manually pared down to:
# #if defined(ADAFRUIT_FEATHER_M0)

# Second, the code lists EVERY VALID MOSI PIN PER SERCOM, without regard to
# special peripherals on a board. So this step involves manually removing
# all items relating to "reserved" SERCOMs (for example, SERCOM3 on the
# MONSTER M4SK, which is intended for the PDM mic)...and then, for the
# remaining SERCOMs, deleting lines that reference valid but "unreasonable"
# pin numbers (e.g. on-board peripherals like QSPI or USB host enable, or
# debug pins), leaving only pins with sensible physical connection points
# (such as Arduino shield or FeatherWing header pins).
# Finally, one must PICK AND CHOOSE among the remaining list of pins for
# each SERCOM. Although each SERCOM can route to a number of potentional
# pins, for the sake of documenting the library it's MUCH simpler to
# explain that for a given board, one can use pins X, Y or Z for output,
# not trying to explain "if you use pin X, can't use pin A or B" and so
# forth, there's just way too many permutations. So this involves looking
# at a board's design and deciding which subset of pins (one per SERCOM)
# provide a good range of options to the user (physically spread around the
# board, not overlapping other peripherals, etc.).
# See Adafruit_NeoPixel_ZeroDMA source for an explanation of it's use of
# SERCOMs and PADs...it's really bending some rules, largely in order for
# this SERCOM pin-finding game to even work (finding matching valid MISO
# and SCK pins for each MOSI becomes an exponentially more complex problem).

# This is hastily-written hack code to do a task and not a Shining Example
# of Python or anything like that.

# pylint: disable=missing-docstring, too-many-locals, too-many-branches

import re

CORE_PATH = "/Users/pburgess/Git/ArduinoCore-samd/"
BOARDS_FILE = CORE_PATH + "boards.txt"
VARIANTS_PATH = CORE_PATH + "variants"

# The 'MUX' table holds data culled from the signal multiplex tables in the
# current (as of this writing) SAMD21 and SAMD51 datasheets (e.g. Table 7-1
# in the SAMD21 datasheet). ALL available pins for a family (SAMD21 or
# SAMD51) are present, though most devices in a family will offer only a
# subset of these signals (e.g. SAMD21E, G and J have different numbers
# of pins), and boards using these devices may be designed to leave some
# unconnected. The information is cross-referenced against data in an
# Arduino board's variant.cpp file to identify actual valid-pins-for-board.
# These lists should NOT be sorted or thinned! They appear here in the same
# order as the datasheets, for easier side-by-side proofreading and updating.
# Each line contains the following elements from the multiplex tables:
# - String indicating a PORT and bit, following the datasheet naming
#   convention (e.g. "Pn00" through "Pn31" -- PA-PB on SAMD21, PA-PD on '51)
# - String containing SERCOM number (0-7) and PAD (0-3) when pin is set to
#   "SERCOM" peripheral (or None if not available in that mux state).
# - String containing SERCOM number (0-7) and PAD (0-3) when pin is set to
#   "SERCOM-ALT" peripheral (or None if not available in that mux state).
# (Yes, these could be more efficiently represented as numeric values,
# but the aim here was copy-and-paste simplicity, as manual translation
# could lead to more errors. This program doesn't need to be space or speed
# efficient, it just generates pin tables occasionally.)
# TIP: to copy PDF tables from Preview on Mac, hold down OPTION key and
# select a rectangular region, then copy. There will still be a lot of cruft
# to edit out, but much less than when trying to copy-paste the whole table
# with a regular text selection. Sometimes neither works well, like the
# table was populated in a weird order or not properly formed. Oh well.

MUX = [
    # From SAM D21/DA1 Family datasheet, 2020
    # These are for A/B/C/D variant devices
    [("PA00", None, "SERCOM1/PAD[0]"), # ------------------ Table 7-1 part 1
     ("PA01", None, "SERCOM1/PAD[1]"),
     ("PA02", None, None),
     ("PA03", None, None),
     ("PB04", None, None),
     ("PB05", None, None),
     ("PB06", None, None),
     ("PB07", None, None),
     ("PB08", None, "SERCOM4/PAD[0]"),
     ("PB09", None, "SERCOM4/PAD[1]"),
     ("PA04", None, "SERCOM0/PAD[0]"),
     ("PA05", None, "SERCOM0/PAD[1]"),
     ("PA06", None, "SERCOM0/PAD[2]"),
     ("PA07", None, "SERCOM0/PAD[3]"),
     ("PA08", "SERCOM0/PAD[0]", "SERCOM2/PAD[0]"),
     ("PA09", "SERCOM0/PAD[1]", "SERCOM2/PAD[1]"),
     ("PA10", "SERCOM0/PAD[2]", "SERCOM2/PAD[2]"),
     ("PA11", "SERCOM0/PAD[3]", "SERCOM2/PAD[3]"),
     ("PB10", None, "SERCOM4/PAD[2]"),
     ("PB11", None, "SERCOM4/PAD[3]"),
     ("PB12", "SERCOM4/PAD[0]", None),
     ("PB13", "SERCOM4/PAD[1]", None),
     ("PB14", "SERCOM4/PAD[2]", None),
     ("PB15", "SERCOM4/PAD[3]", None), # ------------------ Table 7-1 part 2
     ("PA12", "SERCOM2/PAD[0]", "SERCOM4/PAD[0]"),
     ("PA13", "SERCOM2/PAD[1]", "SERCOM4/PAD[1]"),
     ("PA14", "SERCOM2/PAD[2]", "SERCOM4/PAD[2]"),
     ("PA15", "SERCOM2/PAD[3]", "SERCOM4/PAD[3]"),
     ("PA16", "SERCOM1/PAD[0]", "SERCOM3/PAD[0]"),
     ("PA17", "SERCOM1/PAD[1]", "SERCOM3/PAD[1]"),
     ("PA18", "SERCOM1/PAD[2]", "SERCOM3/PAD[2]"),
     ("PA19", "SERCOM1/PAD[3]", "SERCOM3/PAD[3]"),
     ("PB16", "SERCOM5/PAD[0]", None),
     ("PB17", "SERCOM5/PAD[1]", None),
     ("PA20", "SERCOM5/PAD[2]", "SERCOM3/PAD[2]"),
     ("PA21", "SERCOM5/PAD[3]", "SERCOM3/PAD[3]"),
     ("PA22", "SERCOM3/PAD[0]", "SERCOM5/PAD[0]"),
     ("PA23", "SERCOM3/PAD[1]", "SERCOM5/PAD[1]"),
     ("PA24", "SERCOM3/PAD[2]", "SERCOM5/PAD[2]"),
     ("PA25", "SERCOM3/PAD[3]", "SERCOM5/PAD[3]"),
     ("PB22", None, "SERCOM5/PAD[2]"),
     ("PB23", None, "SERCOM5/PAD[3]"),
     ("PA27", None, None),
     ("PA28", None, None),
     ("PA30", None, "SERCOM1/PAD[2]"),
     ("PA31", None, "SERCOM1/PAD[3]"),
     ("PB30", None, "SERCOM5/PAD[0]"),
     ("PB31", None, "SERCOM5/PAD[1]"),
     ("PB00", None, "SERCOM5/PAD[2]"),
     ("PB01", None, "SERCOM5/PAD[3]"),
     ("PB02", None, "SERCOM5/PAD[0]"),
     ("PB03", None, "SERCOM5/PAD[1]")],
    # These are for "L" variant SAMD21 devices:
    [("PA02", None, None), # ------------------------------ Table 7-2 part 1
     ("PA03", None, None),
     ("PB04", None, None),
     ("PB05", None, None),
     ("PB08", None, "SERCOM4/PAD[0]"),
     ("PB09", None, "SERCOM4/PAD[1]"),
     ("PA04", None, "SERCOM0/PAD[0]"),
     ("PA05", None, "SERCOM0/PAD[1]"),
     ("PA06", None, "SERCOM0/PAD[2]"),
     ("PA07", None, "SERCOM0/PAD[3]"),
     ("PA08", "SERCOM0/PAD[0]", "SERCOM2/PAD[0]"),
     ("PA09", "SERCOM0/PAD[1]", "SERCOM2/PAD[1]"),
     ("PA10", "SERCOM0/PAD[2]", "SERCOM2/PAD[2]"),
     ("PA11", "SERCOM0/PAD[3]", "SERCOM2/PAD[3]"),
     ("PB10", None, "SERCOM4/PAD[2]"),
     ("PB11", None, "SERCOM4/PAD[3]"),
     ("PA12", "SERCOM2/PAD[0]", "SERCOM4/PAD[0]"),
     ("PA13", "SERCOM2/PAD[1]", "SERCOM4/PAD[1]"),
     ("PA14", "SERCOM2/PAD[2]", "SERCOM4/PAD[2]"),
     ("PA15", "SERCOM2/PAD[3]", "SERCOM4/PAD[3]"),
     ("PA16", "SERCOM1/PAD[0]", "SERCOM3/PAD[0]"),
     ("PA17", "SERCOM1/PAD[1]", "SERCOM3/PAD[1]"),
     ("PA18", "SERCOM1/PAD[2]", "SERCOM3/PAD[2]"),
     ("PA19", "SERCOM1/PAD[3]", "SERCOM3/PAD[3]"), # ------ Table 7-2 part 2
     ("PA20", "SERCOM5/PAD[2]", "SERCOM3/PAD[2]"),
     ("PA21", "SERCOM5/PAD[3]", "SERCOM3/PAD[3]"),
     ("PA22", "SERCOM3/PAD[0]", "SERCOM5/PAD[0]"),
     ("PA23", "SERCOM3/PAD[1]", "SERCOM5/PAD[1]"),
     ("PA24", "SERCOM3/PAD[2]", "SERCOM5/PAD[2]"),
     ("PA25", "SERCOM3/PAD[3]", "SERCOM5/PAD[3]"),
     ("PA27", None, None),
     ("PA28", None, None),
     ("PA30", None, "SERCOM1/PAD[2]"),
     ("PA31", None, "SERCOM1/PAD[3]"),
     ("PB00", None, None),
     ("PB01", None, None),
     ("PB02", None, "SERCOM5/PAD[0]"),
     ("PB03", None, "SERCOM5/PAD[1]")], # end SAMD21
    # From SAM D5x/E5x Family Data Sheet, 2019
    [("PB03", None, "SERCOM5/PAD[1]"), # ------------------ Table 6-1 part 1
     ("PA00", None, "SERCOM1/PAD[0]"),
     ("PA01", None, "SERCOM1/PAD[1]"),
     ("PC00", None, None),
     ("PC01", None, None),
     ("PC02", None, None),
     ("PC03", None, None),
     ("PA02", None, None),
     ("PA03", None, None),
     ("PB04", None, None),
     ("PB05", None, None),
     ("PD00", None, None),
     ("PD01", None, None),
     ("PB06", None, None),
     ("PB07", None, None),
     ("PB08", None, "SERCOM4/PAD[0]"), # ------------------ Table 6-1 part 2
     ("PB09", None, "SERCOM4/PAD[1]"),
     ("PA04", None, "SERCOM0/PAD[0]"),
     ("PA05", None, "SERCOM0/PAD[1]"),
     ("PA06", None, "SERCOM0/PAD[2]"),
     ("PA07", None, "SERCOM0/PAD[3]"),
     ("PC04", "SERCOM6/PAD[0]", None),
     ("PC05", "SERCOM6/PAD[1]", None),
     ("PC06", "SERCOM6/PAD[2]", None),
     ("PC07", "SERCOM6/PAD[3]", None),
     ("PA08", "SERCOM0/PAD[0]", "SERCOM2/PAD[1]"),
     ("PA09", "SERCOM0/PAD[1]", "SERCOM2/PAD[0]"),
     ("PA10", "SERCOM0/PAD[2]", "SERCOM2/PAD[2]"),
     ("PA11", "SERCOM0/PAD[3]", "SERCOM2/PAD[3]"),
     ("PB10", None, "SERCOM4/PAD[2]"),
     ("PB11", None, "SERCOM4/PAD[3]"),
     ("PB12", "SERCOM4/PAD[0]", None),
     ("PB13", "SERCOM4/PAD[1]", None),
     ("PB14", "SERCOM4/PAD[2]", None),
     ("PB15", "SERCOM4/PAD[3]", None),
     ("PD08", "SERCOM7/PAD[0]", "SERCOM6/PAD[1]"),
     ("PD09", "SERCOM7/PAD[1]", "SERCOM6/PAD[0]"),
     ("PD10", "SERCOM7/PAD[2]", "SERCOM6/PAD[2]"),
     ("PD11", "SERCOM7/PAD[3]", "SERCOM6/PAD[3]"),
     ("PD12", None, None),
     ("PC10", "SERCOM6/PAD[2]", "SERCOM7/PAD[2]"),
     ("PC11", "SERCOM6/PAD[3]", "SERCOM7/PAD[3]"), # ------ Table 6-1 part 3
     ("PC12", "SERCOM7/PAD[0]", "SERCOM6/PAD[1]"),
     ("PC13", "SERCOM7/PAD[1]", "SERCOM6/PAD[0]"),
     ("PC14", "SERCOM7/PAD[2]", "SERCOM6/PAD[2]"),
     ("PC15", "SERCOM7/PAD[3]", "SERCOM6/PAD[3]"),
     ("PA12", "SERCOM2/PAD[0]", "SERCOM4/PAD[1]"),
     ("PA13", "SERCOM2/PAD[1]", "SERCOM4/PAD[0]"),
     ("PA14", "SERCOM2/PAD[2]", "SERCOM4/PAD[2]"),
     ("PA15", "SERCOM2/PAD[3]", "SERCOM4/PAD[3]"),
     ("PA16", "SERCOM1/PAD[0]", "SERCOM3/PAD[1]"),
     ("PA17", "SERCOM1/PAD[1]", "SERCOM3/PAD[0]"),
     ("PA18", "SERCOM1/PAD[2]", "SERCOM3/PAD[2]"),
     ("PA19", "SERCOM1/PAD[3]", "SERCOM3/PAD[3]"),
     ("PC16", "SERCOM6/PAD[0]", "SERCOM0/PAD[1]"),
     ("PC17", "SERCOM6/PAD[1]", "SERCOM0/PAD[0]"),
     ("PC18", "SERCOM6/PAD[2]", "SERCOM0/PAD[2]"),
     ("PC19", "SERCOM6/PAD[3]", "SERCOM0/PAD[3]"),
     ("PC20", None, None),
     ("PC21", None, None),
     ("PC22", "SERCOM1/PAD[0]", "SERCOM3/PAD[1]"),
     ("PC23", "SERCOM1/PAD[1]", "SERCOM3/PAD[0]"),
     ("PD20", "SERCOM1/PAD[2]", "SERCOM3/PAD[2]"),
     ("PD21", "SERCOM1/PAD[3]", "SERCOM3/PAD[3]"),
     ("PB16", "SERCOM5/PAD[0]", None),
     ("PB17", "SERCOM5/PAD[1]", None),
     ("PB18", "SERCOM5/PAD[2]", "SERCOM7/PAD[2]"),
     ("PB19", "SERCOM5/PAD[3]", "SERCOM7/PAD[3]"), # ------ Table 6-1 part 4
     ("PB20", "SERCOM3/PAD[0]", "SERCOM7/PAD[1]"),
     ("PB21", "SERCOM3/PAD[1]", "SERCOM7/PAD[0]"),
     ("PA20", "SERCOM5/PAD[2]", "SERCOM3/PAD[2]"),
     ("PA21", "SERCOM5/PAD[3]", "SERCOM3/PAD[3]"),
     ("PA22", "SERCOM3/PAD[0]", "SERCOM5/PAD[1]"),
     ("PA23", "SERCOM3/PAD[1]", "SERCOM5/PAD[0]"),
     ("PA24", "SERCOM3/PAD[2]", "SERCOM5/PAD[2]"),
     ("PA25", "SERCOM3/PAD[3]", "SERCOM5/PAD[3]"),
     ("PB22", "SERCOM1/PAD[2]", "SERCOM5/PAD[2]"),
     ("PB23", "SERCOM1/PAD[3]", "SERCOM5/PAD[3]"),
     ("PB24", "SERCOM0/PAD[0]", "SERCOM2/PAD[1]"),
     ("PB25", "SERCOM0/PAD[1]", "SERCOM2/PAD[0]"),
     ("PB26", "SERCOM2/PAD[0]", "SERCOM4/PAD[1]"),
     ("PB27", "SERCOM2/PAD[1]", "SERCOM4/PAD[0]"),
     ("PB28", "SERCOM2/PAD[2]", "SERCOM4/PAD[2]"),
     ("PB29", "SERCOM2/PAD[3]", "SERCOM4/PAD[3]"),
     ("PC24", "SERCOM0/PAD[2]", "SERCOM2/PAD[2]"),
     ("PC25", "SERCOM0/PAD[3]", "SERCOM2/PAD[3]"),
     ("PC26", None, None),
     ("PC27", "SERCOM1/PAD[0]", None),
     ("PC28", "SERCOM1/PAD[1]", None),
     ("PA27", None, None),
     ("PA30", None, "SERCOM1/PAD[2]"),
     ("PA31", None, "SERCOM1/PAD[3]"), # ------------------ Table 6-1 part 5
     ("PB30", None, "SERCOM5/PAD[1]"),
     ("PB31", None, "SERCOM5/PAD[0]"),
     ("PC30", None, None),
     ("PC31", None, None),
     ("PB00", None, "SERCOM5/PAD[2]"),
     ("PB01", None, "SERCOM5/PAD[3]"),
     ("PB02", None, "SERCOM5/PAD[0]")] # end SAMD51
]


# Given a variant.h filename, look for items that might indicate the first
# analog pin number (these are always "above" the digital pin numbers in
# Arduino*), and the number of analog pins...above this total, any pins are
# usually very special functions like debug pins or QSPI and should totally
# be avoided as possible outputs.
# * Not necessarily true. Grand Central has "Additional ADC" below the
#   main analog pins. This kind of botched the output for that board and
#   it'll probably need revisiting.
def boardpins(headerfile):
    headerlines = [line.rstrip() for line in open(headerfile)]
    num_analog_inputs = 0
    num_analog_outputs = 0
    pin_a0 = -1
    pin_sda = -1
    pin_scl = -1
    pin_spi_mosi = -1
    pin_spi_miso = -1
    pin_spi_sck = -1
    pin_sda1 = -1
    pin_scl1 = -1
    pin_spi1_mosi = -1
    pin_spi1_miso = -1
    pin_spi1_sck = -1
    for headerline in headerlines:
        pieces = headerline.split()
        if len(pieces) >= 3:
            if pieces[0] == "#define":
                if pieces[1] == "PIN_A0":
                    pin_a0 = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "NUM_ANALOG_INPUTS":
                    num_analog_inputs = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "NUM_ANALOG_OUTPUTS":
                    num_analog_outputs = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_WIRE_SDA":
                    pin_sda = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_WIRE_SCL":
                    pin_scl = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_SPI_MOSI":
                    pin_spi_mosi = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_SPI_MISO":
                    pin_spi_miso = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_SPI_SCK":
                    pin_spi_sck = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_WIRE_SDA1":
                    pin_sda1 = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_WIRE_SCL1":
                    pin_scl1 = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_SPI1_MOSI":
                    pin_spi1_mosi = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_SPI1_MISO":
                    pin_spi1_miso = int(re.sub("[^0-9]", "", pieces[2]))
                elif pieces[1] == "PIN_SPI1_SCK":
                    pin_spi1_sck = int(re.sub("[^0-9]", "", pieces[2]))
    num_analog = max(num_analog_inputs, num_analog_outputs)
    last_analog = pin_a0 + num_analog - 1
    header_pin_list = (pin_a0, last_analog, pin_sda, pin_scl, pin_spi_mosi,
                       pin_spi_miso, pin_spi_sck, pin_sda1, pin_scl1,
                       pin_spi1_mosi, pin_spi1_miso, pin_spi1_sck)
    return header_pin_list


# Look for PORT/bit combos that match up between the MUX table and
# a board's variant.cpp file, make a new board-unique mux list.
# Exclude any pins above a certain index (these are usually debug pins
# or other specialized peripherals).
def boardmux(cppfile, mux_table_index, pin_limit):
    cpplines = [line.rstrip() for line in open(cppfile)]
    pin_num = 0
    sub_mux = []
    activated = False

    # regex search for lines starting with "(whitespace){(whitespace)PORT"
    pindesc1 = re.compile(r"\s*{\s*PORT")
    pindesc2 = re.compile(r"\s*{\s*NOT_A_PORT")

    for cppline in cpplines: # Each line in variant.cpp...
        # Don't start counting or writing anything until the
        # pin description table start has been found...
        if activated:
            # regex search identifies items in g_APinDescription[] table
            if pindesc2.match(cppline): # NOT_A_PORT found
                pin_num += 1 # Increment pin number but do no other work
            elif pindesc1.match(cppline): # PORT found
                pinid = cppline.split("PORT")[1].split(",")
                # pinid[0] will be PORT letter (e.g. "A"), pinid[1] is bit #
                shortid = f'P{pinid[0]}{int(pinid[1]):02}' # e.g. "PA00"
                #if shortid == "PA05":
                #    print(shortid, str(pin_num))
                # Compare shortid against first element of each MUX item...
                for mux_item in MUX[mux_table_index]:
                    if mux_item[0] == shortid:
                        # A match! Append a new item to the sub_mux list...
                        # this is a tuple from the MUX list with the Arduino
                        # pin number also included.
                        sub_mux.append(mux_item + (pin_num,))
                        break
                pin_num += 1
            if pin_num > pin_limit:
                break
        else:
            # Comments in the PyPortal pin descriptions (looking just like
            # actual pin description lines, but in a comment block) were
            # throwing things off...so don't "activate" the pin counting
            # until the pin description table is started...
            if cppline.startswith("const PinDescription g_APinDescription["):
                activated = True

    return sub_mux


# Search a mux table for a SERCOM # and PAD #, returning a subset mux list
def match_sercom_and_pad(mux_list, sercom_num, sercom_pad):
    sublist = []
    match_string = f'SERCOM{sercom_num}/PAD[{sercom_pad}]'
    for mux_item in mux_list:
        # Check both the SERCOM and SERCOM-ALT entries:
        if mux_item[1] and mux_item[1] == match_string:
            sublist.append(mux_item)
        elif mux_item[2] and mux_item[2] == match_string:
            sublist.append(mux_item)
    return sublist


# Given a board's "EXTRA_FLAGS" string (pulled from boards.txt), pare down
# to (hopefully) just the vital subset that uniquely identifies a board.
def reduce_flags(defines):
    remove_list = [
        "-DARDUINO_SAMD_ZERO",
        "-DARDUINO_SAMD_FEATHER_M0",
        "-D__SAMD21E18A__",
        "-D__SAMD21G18A__",
        "-D__SAMD51__",
        "-D__SAMD51G19A__",
        "-D__SAMD51J19A__",
        "-D__SAMD51J20A__",
        "-D__SAMD51P20A__",
        "-D__FPU_PRESENT",
        "-mfloat-abi",
        "-DARM_MATH_CM0PLUS",
        "-DARM_MATH_CM4",
        "-DCRYSTALLESS",
        "{build.usb_flags}",
        "-DADAFRUIT_HALLOWING ", # Keep space at end! (Use _M0/_M4 def only)
        "-D",                    # Remove leading "-D" from remaining defines
        " ",                     # Remove spaces
    ]
    for remove in remove_list:
        defines = defines.replace(remove, "")
    return defines


# Process each board in boards file ----------------------------------------

LINES = [line.rstrip() for line in open(BOARDS_FILE)]

for line in LINES:
    if not line or line[0] == '#':
        continue
    if line.find(".build.variant=") >= 0:
        sep = line.split('=')   # Separate name=value
        variant = sep[1]        # Extract variant string
        sep = sep[0].split('.') # Separate name hierarchy
        lookfor = sep[0]        # Extract root part to search for...
        lookfor = lookfor + ".build.extra_flags=" # Append hierarchy
        for line2 in LINES:
            if line2.find(lookfor) >= 0:
                sep = line2.split('=') # Separate name=value
                extra_flags = sep[1]
                isM0 = extra_flags.find("SAMD21") >= 0
                isM4 = extra_flags.find("SAMD51") >= 0
                if isM0 and isM4:
                    print(variant + " has both M0 and M4 signatures")
                    continue
                elif not isM0 and not isM4:
                    print(variant + " has neither M0 nor M4 signatures")
                    continue
                variant_path = VARIANTS_PATH + "/" + variant
                variant_cpp = variant_path + "/variant.cpp"
                variant_h = variant_path + "/variant.h"
                extra_flags = reduce_flags(extra_flags)
                print("#if defined(" + extra_flags + ")")
                specialpins = boardpins(variant_h)
                max_pin = max(specialpins)
                first_analog_pin = specialpins[0]
                last_analog_pin = specialpins[1]
                # Skip "L" variant table (SAMD21) for now,
                # no Adafruit boards are using it.
                if isM4:
                    table_to_use = 2
                else:
                    table_to_use = 0
                board_mux = boardmux(variant_cpp, table_to_use, max_pin)
                pin_list = []
                for sercom in range(8):
                    sercom_pin_list = []
                    # Get a complete list of candidate MOSIs...
                    # On M4, only PADs 0 and 3 are valid for MOSI...
                    mosi_list = (match_sercom_and_pad(board_mux, sercom, 0) +
                                 match_sercom_and_pad(board_mux, sercom, 3))
                    # On M0 only, PAD 2 is also an option...
                    if isM0:
                        mosi_list += match_sercom_and_pad(board_mux, sercom, 2)
                    # To cull list for candidate pins, make sure item is
                    # populated (not an empty entry) and that the pin number
                    # isn't already in the sercom_pin_list.
                    for mosi in mosi_list:
                        if mosi == []:
                            continue
                        reject_pin = False
                        for pin in sercom_pin_list:
                            # Item 3 is Arduino pin # in mosi[],
                            # Item 1 is Arduino pin # in sercom_pin_list[]
                            if pin[1] == mosi[3]:
                                reject_pin = True
                                break
                        if reject_pin:
                            continue  # Already in pin list
                        if mosi[1] and mosi[1][6] == str(sercom):
                            mosi_pad = int(mosi[1][12])
                            sercom_alt = False
                        elif mosi[2] and mosi[2][6] == str(sercom):
                            mosi_pad = int(mosi[2][12])
                            sercom_alt = True
                        # Add SERCOM number, pin number, MOSI PAD # and
                        # whether pin peripheral is SERCOM or SERCOM-ALT
                        # to pin_list. That's all we need.
                        pin_list.append(
                            (sercom, mosi[3], mosi_pad, sercom_alt))

                pin_list.sort() # Sort by SERCOM and pin number
                prev_sercom = pin_list[0][0]
                for pin in pin_list:
                    # Remap analog pin numbers to A0, A1, ...
                    # Remap special function pins to human labels
                    if (pin[1] >= first_analog_pin and
                            pin[1] <= last_analog_pin):
                        pin_str = "A" + str(pin[1] - first_analog_pin)
                    elif pin[1] == specialpins[2]:
                        pin_str = "SDA"
                    elif pin[1] == specialpins[3]:
                        pin_str = "SCL"
                    elif pin[1] == specialpins[4]:
                        pin_str = "MOSI"
                    elif pin[1] == specialpins[5]:
                        pin_str = "MISO"
                    elif pin[1] == specialpins[6]:
                        pin_str = "SCK"
                    elif pin[1] == specialpins[7]:
                        pin_str = "SDA1"
                    elif pin[1] == specialpins[8]:
                        pin_str = "SCL1"
                    elif pin[1] == specialpins[9]:
                        pin_str = "MOSI1"
                    elif pin[1] == specialpins[10]:
                        pin_str = "MISO1"
                    elif pin[1] == specialpins[11]:
                        pin_str = "SCK1"
                    else:
                        pin_str = str(pin[1])
                    pin_str = pin_str.rjust(5)  # Right-align, 5 chars
                    # If MOSI PAD # is 2 (M0), SCK PAD MUST be 3!
                    # MOSI PAD of 0 could have SCK on PAD 0 or 3,
                    # while MOSI PAD 3 must have SCK on PAD 1...
                    # so both of the latter are handled using PAD 1.
                    if pin[2] == 2:
                        pad_str = "SPI_PAD_" + str(pin[2]) + "_SCK_3"
                    else:
                        pad_str = "SPI_PAD_" + str(pin[2]) + "_SCK_1"
                    if pin[3]:
                        sercom_str = "PIO_SERCOM_ALT"
                    else:
                        sercom_str = "PIO_SERCOM"
                    # Put a blank line between SERCOMs for legibility
                    if(pin[0] != prev_sercom):
                        print()
                        prev_sercom = pin[0]
                    print("  &sercom" + str(pin[0]) +
                          ", SERCOM" + str(pin[0]) +
                          ", SERCOM" + str(pin[0]) + "_DMAC_ID_TX," +
                          pin_str + ", " + pad_str + ", " + sercom_str + ",")

                print("#endif")
                print()


# Enums for valid SPI PAD assignments:
# SPI_PAD_0_SCK_1 means MOSI is on SERCOMn.0 and SCK is on SERCOMn.1
# SPI_PAD_2_SCK_3 means MOSI is on SERCOMn.2 and SCK is on SERCOMn.3 <- SAMD21
# SPI_PAD_3_SCK_1 means MOSI is on SERCOMn.3 and SCK is on SERCOMn.1
# SPI_PAD_0_SCK_3 means MOSI is on SERCOMn.0 and SCK is on SERCOMn.3
