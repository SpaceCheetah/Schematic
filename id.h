#pragma once

#include "wx/defs.h"

namespace id {
    enum id {
        tool_wire = wxID_HIGHEST + 1,
        tool_resistor,
        tool_volt_source,
        tool_amp_source,
        tool_bin,
        tool_capacitor,
        tool_switch,
        rotate,
        rotate_cw,
        rotate_ccw,
        flip,
        set_value,
        toggle_up,
        toggle_left,
        toggle_right,
        toggle_down,
        toggle_dependent,
        toggle_closed,
        file_new,
        file_save,
        file_save_as,
        file_load,
        view_dot_size,
        view_rotated_text,
        view_shaded_background,
        dot_size_slider
    };
}