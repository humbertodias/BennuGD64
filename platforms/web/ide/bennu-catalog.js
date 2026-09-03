// Generated from modules/*/ *_exports.h — BennuGD function/constant catalog.
export const CATALOG = {
  "libblit": {
    "deps": [],
    "funcs": {},
    "consts": [
      "B_HMIRROR",
      "B_VMIRROR",
      "B_TRANSLUCENT",
      "B_ALPHA",
      "B_ABLEND",
      "B_SBLEND",
      "B_NOCOLORKEY"
    ]
  },
  "libfont": {
    "deps": [
      "libgrbase"
    ],
    "funcs": {},
    "consts": []
  },
  "libgrbase": {
    "deps": [],
    "funcs": {},
    "consts": []
  },
  "libjoy": {
    "deps": [
      "libsdlhandler"
    ],
    "funcs": {},
    "consts": [
      "JOY_HAT_CENTERED",
      "JOY_HAT_UP",
      "JOY_HAT_RIGHT",
      "JOY_HAT_DOWN",
      "JOY_HAT_LEFT",
      "JOY_HAT_RIGHTUP",
      "JOY_HAT_RIGHTDOWN",
      "JOY_HAT_LEFTUP",
      "JOY_HAT_LEFTDOWN"
    ]
  },
  "libkey": {
    "deps": [
      "libsdlhandler"
    ],
    "funcs": {},
    "consts": [
      "_ESC",
      "_1",
      "_2",
      "_3",
      "_4",
      "_5",
      "_6",
      "_7",
      "_8",
      "_9",
      "_0",
      "_MINUS",
      "_PLUS",
      "_BACKSPACE",
      "_TAB",
      "_Q",
      "_W",
      "_E",
      "_R",
      "_T",
      "_Y",
      "_U",
      "_I",
      "_O",
      "_P",
      "_L_BRACKET",
      "_R_BRACKET",
      "_ENTER",
      "_C_ENTER",
      "_CONTROL",
      "_A",
      "_S",
      "_D",
      "_F",
      "_G",
      "_H",
      "_J",
      "_K",
      "_L",
      "_SEMICOLON",
      "_APOSTROPHE",
      "_WAVE",
      "_L_SHIFT",
      "_BACKSLASH",
      "_Z",
      "_X",
      "_C",
      "_V",
      "_B",
      "_N",
      "_M",
      "_COMMA",
      "_POINT",
      "_SLASH",
      "_C_BACKSLASH",
      "_R_SHIFT",
      "_C_ASTERISK",
      "_PRN_SCR",
      "_ALT",
      "_SPACE",
      "_CAPS_LOCK",
      "_F1",
      "_F2",
      "_F3",
      "_F4",
      "_F5",
      "_F6",
      "_F7",
      "_F8",
      "_F9",
      "_F10",
      "_NUM_LOCK",
      "_SCROLL_LOCK",
      "_HOME",
      "_C_HOME",
      "_UP",
      "_C_UP",
      "_PGUP",
      "_C_PGUP",
      "_C_MINUS",
      "_LEFT",
      "_C_LEFT",
      "_C_CENTER",
      "_RIGHT",
      "_C_RIGHT",
      "_C_PLUS",
      "_END",
      "_C_END",
      "_DOWN",
      "_C_DOWN",
      "_PGDN",
      "_C_PGDN",
      "_INS",
      "_C_INS",
      "_DEL",
      "_C_DEL",
      "_F11",
      "_F12",
      "_LESS",
      "_EQUALS",
      "_GREATER",
      "_ASTERISK",
      "_R_ALT",
      "_R_CONTROL",
      "_L_ALT",
      "_L_CONTROL",
      "_MENU",
      "_L_WINDOWS",
      "_R_WINDOWS",
      "STAT_RSHIFT",
      "STAT_LSHIFT",
      "STAT_CTRL",
      "STAT_ALT",
      "STAT_RCTRL",
      "STAT_LCTRL",
      "STAT_RALT",
      "STAT_LALT",
      "STAT_NUM",
      "STAT_CAPS",
      "STAT_SHIFT"
    ]
  },
  "libmouse": {
    "deps": [
      "libsdlhandler",
      "libgrbase",
      "libvideo",
      "libblit",
      "librender"
    ],
    "funcs": {},
    "consts": []
  },
  "librender": {
    "deps": [
      "libgrbase",
      "libvideo",
      "libblit"
    ],
    "funcs": {},
    "consts": [
      "C_SCREEN",
      "PARTIAL_DUMP",
      "COMPLETE_DUMP",
      "NO_RESTORE",
      "PARTIAL_RESTORE",
      "COMPLETE_RESTORE",
      "BACKGROUND",
      "SCREEN",
      "SCALE_SCALE2X",
      "SCALE_HQ2X",
      "SCALE_SCANLINE2X",
      "SCALE_NORMAL2X",
      "SCALE_NOFILTER"
    ]
  },
  "libscroll": {
    "deps": [
      "libgrbase",
      "libblit",
      "librender",
      "libvideo"
    ],
    "funcs": {},
    "consts": [
      "C_SCROLL",
      "C_0",
      "C_1",
      "C_2",
      "C_3",
      "C_4",
      "C_5",
      "C_6",
      "C_7",
      "C_8",
      "C_9"
    ]
  },
  "libtext": {
    "deps": [
      "libgrbase",
      "libblit",
      "librender",
      "libfont"
    ],
    "funcs": {},
    "consts": []
  },
  "libvideo": {
    "deps": [
      "libgrbase"
    ],
    "funcs": {},
    "consts": [
      "M320X200",
      "M320X240",
      "M320X400",
      "M360X240",
      "M376X282",
      "M400X300",
      "M512X384",
      "M640X400",
      "M640X480",
      "M800X600",
      "M1024X768",
      "M1280X1024",
      "MODE_WINDOW",
      "MODE_2XSCALE",
      "MODE_FULLSCREEN",
      "MODE_DOUBLEBUFFER",
      "MODE_HARDWARE",
      "MODE_WAITVSYNC",
      "WAITVSYNC",
      "DOUBLE_BUFFER",
      "HW_SURFACE",
      "MODE_8BITS",
      "MODE_16BITS",
      "MODE_32BITS",
      "MODE_8BPP",
      "MODE_16BPP",
      "MODE_32BPP",
      "MODE_MODAL",
      "MODE_FRAMELESS",
      "SCALE_NONE",
      "SRO_NORMAL",
      "SRO_LEFT",
      "SRO_DOWN",
      "SRO_RIGHT",
      "SRA_STRETCH",
      "SRA_PRESERVE"
    ]
  },
  "libwm": {
    "deps": [
      "libsdlhandler"
    ],
    "funcs": {},
    "consts": []
  },
  "mod_blendop": {
    "deps": [
      "libgrbase"
    ],
    "funcs": {
      "BLENDOP_NEW": [
        ""
      ],
      "BLENDOP_IDENTITY": [
        "I"
      ],
      "BLENDOP_TINT": [
        "IFIII"
      ],
      "BLENDOP_TRANSLUCENCY": [
        "IF"
      ],
      "BLENDOP_INTENSITY": [
        "IF"
      ],
      "BLENDOP_SWAP": [
        "I"
      ],
      "BLENDOP_ASSIGN": [
        "III"
      ],
      "BLENDOP_APPLY": [
        "III"
      ],
      "BLENDOP_FREE": [
        "I"
      ],
      "BLENDOP_GRAYSCALE": [
        "II"
      ]
    },
    "consts": []
  },
  "mod_cd": {
    "deps": [],
    "funcs": {
      "CD_DRIVES": [
        ""
      ],
      "CD_STATUS": [
        "I"
      ],
      "CD_NAME": [
        "I"
      ],
      "CD_GETINFO": [
        "I"
      ],
      "CD_PLAY": [
        "II",
        "III"
      ],
      "CD_STOP": [
        "I"
      ],
      "CD_PAUSE": [
        "I"
      ],
      "CD_RESUME": [
        "I"
      ],
      "CD_EJECT": [
        "I"
      ]
    },
    "consts": [
      "CD_TRAYEMPTY",
      "CD_STOPPED",
      "CD_PLAYING",
      "CD_PAUSED",
      "CD_ERROR"
    ]
  },
  "mod_crypt": {
    "deps": [],
    "funcs": {
      "CRYPT_NEW": [
        "IP"
      ],
      "CRYPT_DEL": [
        "P"
      ],
      "CRYPT_ENCRYPT": [
        "PPPI",
        "IPPPI"
      ],
      "CRYPT_DECRYPT": [
        "PPPI",
        "IPPPI"
      ]
    },
    "consts": [
      "CRYPT_NONE",
      "CRYPT_DES",
      "CRYPT_3DES"
    ]
  },
  "mod_debug": {
    "deps": [
      "libkey",
      "librender"
    ],
    "funcs": {
      "TRACE": [
        "I"
      ]
    },
    "consts": []
  },
  "mod_dir": {
    "deps": [],
    "funcs": {
      "CD": [
        "",
        "S"
      ],
      "CHDIR": [
        "S"
      ],
      "MKDIR": [
        "S"
      ],
      "RMDIR": [
        "S"
      ],
      "GLOB": [
        "S"
      ],
      "RM": [
        "S"
      ],
      "DIROPEN": [
        "S"
      ],
      "DIRCLOSE": [
        "I"
      ],
      "DIRREAD": [
        "I"
      ]
    },
    "consts": []
  },
  "mod_draw": {
    "deps": [
      "libgrbase",
      "librender",
      "libdraw"
    ],
    "funcs": {
      "DRAWING_MAP": [
        "II"
      ],
      "DRAWING_COLOR": [
        "I"
      ],
      "DRAW_LINE": [
        "IIII"
      ],
      "DRAW_RECT": [
        "IIII"
      ],
      "DRAW_BOX": [
        "IIII"
      ],
      "DRAW_CIRCLE": [
        "III"
      ],
      "DRAW_FCIRCLE": [
        "III"
      ],
      "DRAW_CURVE": [
        "IIIIIIIII"
      ],
      "DRAWING_Z": [
        "I"
      ],
      "DELETE_DRAW": [
        "I"
      ],
      "MOVE_DRAW": [
        "III"
      ],
      "DRAWING_ALPHA": [
        "I"
      ],
      "DRAWING_STIPPLE": [
        "I"
      ],
      "PUT_PIXEL": [
        "III"
      ],
      "GET_PIXEL": [
        "II"
      ],
      "MAP_GET_PIXEL": [
        "IIII"
      ],
      "MAP_PUT_PIXEL": [
        "IIIII"
      ]
    },
    "consts": []
  },
  "mod_effects": {
    "deps": [
      "libgrbase"
    ],
    "funcs": {
      "GRAYSCALE": [
        "IIB"
      ],
      "RGBSCALE": [
        "IIFFF"
      ],
      "BLUR": [
        "IIB"
      ],
      "FILTER": [
        "IIP"
      ]
    },
    "consts": [
      "BLUR_NORMAL",
      "BLUR_3x3",
      "BLUR_5x5",
      "BLUR_5x5_MAP",
      "GSCALE_RGB",
      "GSCALE_R",
      "GSCALE_G",
      "GSCALE_B",
      "GSCALE_RG",
      "GSCALE_RB",
      "GSCALE_GB",
      "GSCALE_OFF"
    ]
  },
  "mod_file": {
    "deps": [],
    "funcs": {
      "SAVE": [
        "SV++"
      ],
      "LOAD": [
        "SV++"
      ],
      "FOPEN": [
        "SI"
      ],
      "FCLOSE": [
        "I"
      ],
      "FREAD": [
        "IV++",
        "PII"
      ],
      "FWRITE": [
        "IV++",
        "PII"
      ],
      "FSEEK": [
        "III"
      ],
      "FREWIND": [
        "I"
      ],
      "FTELL": [
        "I"
      ],
      "FFLUSH": [
        "I"
      ],
      "FLUSH": [
        "I"
      ],
      "FLENGTH": [
        "I"
      ],
      "FPUTS": [
        "IS"
      ],
      "FGETS": [
        "I"
      ],
      "FEOF": [
        "I"
      ],
      "FILE": [
        "S"
      ],
      "FEXISTS": [
        "S"
      ],
      "FILE_EXISTS": [
        "S"
      ],
      "FREMOVE": [
        "S"
      ],
      "FMOVE": [
        "SS"
      ]
    },
    "consts": [
      "O_READ",
      "O_READWRITE",
      "O_RDWR",
      "O_WRITE",
      "O_ZREAD",
      "O_ZWRITE",
      "SEEK_SET",
      "SEEK_CUR",
      "SEEK_END"
    ]
  },
  "mod_flic": {
    "deps": [],
    "funcs": {
      "START_FLI": [
        "SII"
      ],
      "END_FLI": [
        ""
      ],
      "FRAME_FLI": [
        ""
      ],
      "RESET_FLI": [
        ""
      ],
      "FLI_START": [
        "SII",
        "SIIIIII"
      ],
      "FLI_END": [
        "I"
      ],
      "FLI_FRAME": [
        "I"
      ],
      "FLI_RESET": [
        "I"
      ],
      "FLI_PARAMS": [
        "IIIIIII"
      ],
      "FLI_MOVE": [
        "III"
      ],
      "FLI_ANGLE": [
        "II"
      ],
      "FLI_SIZE": [
        "II"
      ],
      "FLI_FLAGS": [
        "II"
      ],
      "FLI_Z": [
        "II"
      ],
      "FLI_GETINFO": [
        "IPPPPPPPPP"
      ]
    },
    "consts": []
  },
  "mod_grproc": {
    "deps": [
      "libmouse",
      "libgrbase",
      "libvideo",
      "librender",
      "libblit"
    ],
    "funcs": {
      "ADVANCE": [
        "I"
      ],
      "XADVANCE": [
        "II"
      ],
      "GET_ANGLE": [
        "I"
      ],
      "GET_DIST": [
        "I"
      ],
      "COLLISION": [
        "I"
      ],
      "COLLISION_BOX": [
        "I"
      ],
      "COLLISION_CIRCLE": [
        "I"
      ],
      "GET_REAL_POINT": [
        "IPP"
      ]
    },
    "consts": []
  },
  "mod_joy": {
    "deps": [
      "libjoy"
    ],
    "funcs": {
      "JOY_AXES": [
        "",
        "I"
      ],
      "JOY_NUMAXES": [
        "",
        "I"
      ],
      "JOY_GETAXIS": [
        "I",
        "II"
      ],
      "JOY_BUTTONS": [
        "",
        "I"
      ],
      "JOY_NAME": [
        "I"
      ],
      "JOY_NUMBUTTONS": [
        "",
        "I"
      ],
      "JOY_NUMBER": [
        ""
      ],
      "JOY_NUMJOYSTICKS": [
        ""
      ],
      "JOY_SELECT": [
        "I"
      ],
      "JOY_GETBUTTON": [
        "I",
        "II"
      ],
      "JOY_GETPOSITION": [
        "I",
        "II"
      ],
      "JOY_NUMHATS": [
        "",
        "I"
      ],
      "JOY_NUMBALLS": [
        "",
        "I"
      ],
      "JOY_GETHAT": [
        "I",
        "II"
      ],
      "JOY_GETBALL": [
        "IPP",
        "IIPP"
      ],
      "JOY_GETACCEL": [
        "PPP",
        "IPPP"
      ],
      "NUMBER_JOY": [
        ""
      ],
      "SELECT_JOY": [
        "I"
      ],
      "GET_JOY_BUTTON": [
        "I",
        "II"
      ],
      "GET_JOY_POSITION": [
        "I",
        "II"
      ]
    },
    "consts": []
  },
  "mod_key": {
    "deps": [
      "libkey"
    ],
    "funcs": {
      "KEY": [
        "I"
      ]
    },
    "consts": []
  },
  "mod_m7": {
    "deps": [
      "libgrbase",
      "libvideo",
      "librender"
    ],
    "funcs": {
      "MODE7_START": [
        "IIIIIIII",
        "IIIIII"
      ],
      "MODE7_STOP": [
        "I"
      ],
      "START_MODE7": [
        "IIIIIIII",
        "IIIIII"
      ],
      "STOP_MODE7": [
        "I"
      ]
    },
    "consts": [
      "C_M7"
    ]
  },
  "mod_map": {
    "deps": [
      "libgrbase",
      "libvideo",
      "libblit",
      "libfont"
    ],
    "funcs": {
      "MAP_BLOCK_COPY": [
        "IIIIIIIIII"
      ],
      "MAP_PUT": [
        "IIIII"
      ],
      "MAP_XPUT": [
        "IIIIIIII"
      ],
      "MAP_NEW": [
        "III",
        "IIII"
      ],
      "MAP_CLEAR": [
        "III"
      ],
      "MAP_CLONE": [
        "II"
      ],
      "MAP_NAME": [
        "II"
      ],
      "MAP_SET_NAME": [
        "IIS"
      ],
      "MAP_EXISTS": [
        "II"
      ],
      "MAP_XPUTNP": [
        "IIIIIIIIII"
      ],
      "MAP_DEL": [
        "II"
      ],
      "MAP_UNLOAD": [
        "II"
      ],
      "MAP_LOAD": [
        "S",
        "SP"
      ],
      "MAP_SAVE": [
        "IIS"
      ],
      "MAP_BUFFER": [
        "II"
      ],
      "FPG_ADD": [
        "IIII"
      ],
      "FPG_NEW": [
        ""
      ],
      "FPG_EXISTS": [
        "I"
      ],
      "FPG_LOAD": [
        "S",
        "SP"
      ],
      "FPG_SAVE": [
        "IS"
      ],
      "FPG_DEL": [
        "I"
      ],
      "FPG_UNLOAD": [
        "I"
      ],
      "PAL_NEW": [
        ""
      ],
      "PAL_DEL": [
        "I"
      ],
      "PAL_UNLOAD": [
        "I"
      ],
      "PAL_CLONE": [
        "I"
      ],
      "PAL_REFRESH": [
        "",
        "I"
      ],
      "PAL_MAP_GETID": [
        "II"
      ],
      "PAL_MAP_ASSIGN": [
        "III"
      ],
      "PAL_MAP_REMOVE": [
        "II"
      ],
      "PAL_GET": [
        "IIP",
        "IIIP"
      ],
      "PAL_SYS_SET": [
        "I",
        "P"
      ],
      "PAL_SET": [
        "IIP",
        "IIIP"
      ],
      "PAL_SAVE": [
        "S",
        "SI"
      ],
      "PAL_LOAD": [
        "S",
        "SP"
      ],
      "COLORS_SET": [
        "IIP",
        "IIIP"
      ],
      "COLORS_GET": [
        "IIP",
        "IIIP"
      ],
      "PALETTE_ROLL": [
        "III"
      ],
      "PALETTE_CONVERT": [
        "IIP"
      ],
      "COLOR_FIND": [
        "BBB",
        "IBBB",
        "IIBBB"
      ],
      "RGB": [
        "IIBBB",
        "BBBI",
        "BBB"
      ],
      "RGBA": [
        "IIBBBB",
        "BBBBI",
        "BBBB"
      ],
      "RGB_GET": [
        "IIIPPP",
        "IPPPI",
        "IPPP"
      ],
      "RGBA_GET": [
        "IIIPPPP",
        "IPPPPI",
        "IPPPP"
      ],
      "FADE": [
        "IIII"
      ],
      "FADE_ON": [
        ""
      ],
      "FADE_OFF": [
        ""
      ],
      "MAP_INFO_SET": [
        "IIII"
      ],
      "MAP_INFO_GET": [
        "III"
      ],
      "MAP_INFO": [
        "III"
      ],
      "GRAPHIC_SET": [
        "IIII"
      ],
      "GRAPHIC_INFO": [
        "III"
      ],
      "POINT_GET": [
        "IIIPP"
      ],
      "POINT_SET": [
        "IIIII"
      ],
      "CENTER_SET": [
        "IIII"
      ],
      "FNT_LOAD": [
        "S",
        "SP"
      ],
      "FNT_UNLOAD": [
        "I"
      ],
      "FNT_SAVE": [
        "IS"
      ],
      "FNT_NEW": [
        "I",
        "II",
        "IIIIIIII"
      ],
      "BDF_LOAD": [
        "S",
        "SP"
      ],
      "GLYPH_GET": [
        "II"
      ],
      "GLYPH_SET": [
        "IIII"
      ],
      "PNG_LOAD": [
        "S",
        "SP"
      ],
      "PCX_LOAD": [
        "S",
        "SP"
      ],
      "PNG_SAVE": [
        "IIS"
      ],
      "NEW_MAP": [
        "III"
      ],
      "LOAD_MAP": [
        "S",
        "SP"
      ],
      "UNLOAD_MAP": [
        "II"
      ],
      "SAVE_MAP": [
        "IIS"
      ],
      "NEW_PAL": [
        ""
      ],
      "LOAD_PAL": [
        "S",
        "SP"
      ],
      "UNLOAD_PAL": [
        "I"
      ],
      "SAVE_PAL": [
        "S",
        "SI"
      ],
      "SET_COLORS": [
        "IIP",
        "IIIP"
      ],
      "GET_COLORS": [
        "IIP",
        "IIIP"
      ],
      "ROLL_PALETTE": [
        "III"
      ],
      "CONVERT_PALETTE": [
        "IIP"
      ],
      "FIND_COLOR": [
        "BBB",
        "IBBB",
        "IIBBB"
      ],
      "GET_RGB": [
        "IIIPPP",
        "IPPPI",
        "IPPP"
      ],
      "GET_RGBA": [
        "IIIPPPP",
        "IPPPPI",
        "IPPPP"
      ],
      "NEW_FPG": [
        ""
      ],
      "LOAD_FPG": [
        "S",
        "SP"
      ],
      "SAVE_FPG": [
        "IS"
      ],
      "UNLOAD_FPG": [
        "I"
      ],
      "GET_POINT": [
        "IIIPP"
      ],
      "SET_POINT": [
        "IIIII"
      ],
      "SET_CENTER": [
        "IIII"
      ],
      "NEW_FNT": [
        "I",
        "II",
        "IIIIIIII"
      ],
      "LOAD_FNT": [
        "S",
        "SP"
      ],
      "UNLOAD_FNT": [
        "I"
      ],
      "SAVE_FNT": [
        "IS"
      ],
      "LOAD_BDF": [
        "S",
        "SP"
      ],
      "GET_GLYPH": [
        "II"
      ],
      "SET_GLYPH": [
        "IIII"
      ],
      "LOAD_PNG": [
        "S",
        "SP"
      ],
      "LOAD_PCX": [
        "S",
        "SP"
      ],
      "SAVE_PNG": [
        "IIS"
      ]
    },
    "consts": [
      "G_WIDE",
      "G_WIDTH",
      "G_HEIGHT",
      "G_CENTER_X",
      "G_X_CENTER",
      "G_CENTER_Y",
      "G_Y_CENTER",
      "G_PITCH",
      "G_DEPTH",
      "B_CLEAR",
      "CHARSET_ISO8859",
      "CHARSET_CP850",
      "NFB_VARIABLEWIDTH",
      "NFB_FIXEDWIDTH"
    ]
  },
  "mod_math": {
    "deps": [],
    "funcs": {
      "ABS": [
        "F"
      ],
      "POW": [
        "FF"
      ],
      "SQRT": [
        "F"
      ],
      "COS": [
        "F"
      ],
      "SIN": [
        "F"
      ],
      "TAN": [
        "F"
      ],
      "ACOS": [
        "F"
      ],
      "ASIN": [
        "F"
      ],
      "ATAN": [
        "F"
      ],
      "ATAN2": [
        "FF"
      ],
      "ISINF": [
        "F"
      ],
      "ISNAN": [
        "F"
      ],
      "FINITE": [
        "F"
      ],
      "FGET_ANGLE": [
        "IIII"
      ],
      "FGET_DIST": [
        "IIII"
      ],
      "NEAR_ANGLE": [
        "III"
      ],
      "GET_DISTX": [
        "II"
      ],
      "GET_DISTY": [
        "II"
      ]
    },
    "consts": [
      "PI"
    ]
  },
  "mod_mathi": {
    "deps": [],
    "funcs": {
      "ABS": [
        "F"
      ],
      "POW": [
        "FF"
      ],
      "SQRT": [
        "F"
      ],
      "COS": [
        "I"
      ],
      "SIN": [
        "I"
      ],
      "TAN": [
        "I"
      ],
      "ACOS": [
        "I"
      ],
      "ASIN": [
        "I"
      ],
      "ATAN": [
        "I"
      ],
      "ATAN2": [
        "II"
      ],
      "ISINF": [
        "F"
      ],
      "ISNAN": [
        "F"
      ],
      "FINITE": [
        "F"
      ],
      "FGET_ANGLE": [
        "IIII"
      ],
      "FGET_DIST": [
        "IIII"
      ],
      "NEAR_ANGLE": [
        "III"
      ],
      "GET_DISTX": [
        "II"
      ],
      "GET_DISTY": [
        "II"
      ]
    },
    "consts": [
      "PI"
    ]
  },
  "mod_mem": {
    "deps": [],
    "funcs": {
      "MEM_CALLOC": [
        "II"
      ],
      "MEM_ALLOC": [
        "I"
      ],
      "MEM_FREE": [
        "P"
      ],
      "MEM_REALLOC": [
        "PI"
      ],
      "MEM_CMP": [
        "PPI"
      ],
      "MEM_SET": [
        "PBI"
      ],
      "MEM_SETW": [
        "PWI"
      ],
      "MEM_SETI": [
        "PII"
      ],
      "MEM_COPY": [
        "PPI"
      ],
      "MEM_MOVE": [
        "PPI"
      ],
      "MEM_AVAILABLE": [
        ""
      ],
      "MEM_TOTAL": [
        ""
      ],
      "CALLOC": [
        "II"
      ],
      "ALLOC": [
        "I"
      ],
      "FREE": [
        "P"
      ],
      "REALLOC": [
        "PI"
      ],
      "MEMCMP": [
        "PPI"
      ],
      "MEMSET": [
        "PBI"
      ],
      "MEMSETW": [
        "PWI"
      ],
      "MEMSETI": [
        "PII"
      ],
      "MEMCOPY": [
        "PPI"
      ],
      "MEMMOVE": [
        "PPI"
      ],
      "MEMORY_FREE": [
        ""
      ],
      "MEMORY_TOTAL": [
        ""
      ]
    },
    "consts": []
  },
  "mod_mouse": {
    "deps": [
      "libmouse"
    ],
    "funcs": {},
    "consts": []
  },
  "mod_path": {
    "deps": [
      "libgrbase"
    ],
    "funcs": {
      "PATH_FIND": [
        "IIIIIII"
      ],
      "PATH_GETXY": [
        "PP"
      ],
      "PATH_WALL": [
        "I"
      ]
    },
    "consts": [
      "PF_NODIAG",
      "PF_REVERSE"
    ]
  },
  "mod_proc": {
    "deps": [],
    "funcs": {
      "GET_ID": [
        "I"
      ],
      "GET_STATUS": [
        "I"
      ],
      "SIGNAL": [
        "II"
      ],
      "SIGNAL_ACTION": [
        "II",
        "III"
      ],
      "LET_ME_ALONE": [
        ""
      ],
      "EXIT": [
        "SI",
        "S",
        ""
      ],
      "EXISTS": [
        "I"
      ]
    },
    "consts": [
      "S_KILL",
      "S_WAKEUP",
      "S_SLEEP",
      "S_FREEZE",
      "S_FORCE",
      "S_TREE",
      "S_KILL_TREE",
      "S_WAKEUP_TREE",
      "S_SLEEP_TREE",
      "S_FREEZE_TREE",
      "S_KILL_FORCE",
      "S_WAKEUP_FORCE",
      "S_SLEEP_FORCE",
      "S_FREEZE_FORCE",
      "S_KILL_TREE_FORCE",
      "S_WAKEUP_TREE_FORCE",
      "S_SLEEP_TREE_FORCE",
      "S_FREEZE_TREE_FORCE",
      "S_DFL",
      "S_IGN",
      "ALL_PROCESS"
    ]
  },
  "mod_rand": {
    "deps": [],
    "funcs": {
      "RAND_SEED": [
        "I"
      ],
      "RAND": [
        "II"
      ]
    },
    "consts": []
  },
  "mod_regex": {
    "deps": [],
    "funcs": {
      "REGEX": [
        "SS"
      ],
      "REGEX_REPLACE": [
        "SSS"
      ],
      "SPLIT": [
        "SSPI"
      ],
      "JOIN": [
        "SPI"
      ]
    },
    "consts": []
  },
  "mod_say": {
    "deps": [],
    "funcs": {
      "SAY": [
        "S"
      ],
      "SAY_FAST": [
        "S"
      ]
    },
    "consts": []
  },
  "mod_screen": {
    "deps": [
      "libgrbase",
      "libvideo",
      "libblit",
      "librender"
    ],
    "funcs": {
      "REGION_DEFINE": [
        "IIIII"
      ],
      "REGION_OUT": [
        "II"
      ],
      "PUT": [
        "IIII"
      ],
      "XPUT": [
        "IIIIIIII"
      ],
      "SCREEN_PUT": [
        "II"
      ],
      "SCREEN_CLEAR": [
        ""
      ],
      "SCREEN_GET": [
        ""
      ],
      "DEFINE_REGION": [
        "IIIII"
      ],
      "OUT_REGION": [
        "II"
      ],
      "PUT_SCREEN": [
        "II"
      ],
      "CLEAR_SCREEN": [
        ""
      ],
      "GET_SCREEN": [
        ""
      ]
    },
    "consts": []
  },
  "mod_scroll": {
    "deps": [
      "libscroll"
    ],
    "funcs": {
      "SCROLL_START": [
        "IIIIIIII",
        "IIIIII"
      ],
      "SCROLL_STOP": [
        "I"
      ],
      "SCROLL_MOVE": [
        "I"
      ],
      "START_SCROLL": [
        "IIIIIIII",
        "IIIIII"
      ],
      "STOP_SCROLL": [
        "I"
      ],
      "MOVE_SCROLL": [
        "I"
      ]
    },
    "consts": []
  },
  "mod_sort": {
    "deps": [],
    "funcs": {
      "QUICKSORT": [
        "PIIIBB"
      ],
      "KSORT": [
        "V++V++",
        "V++V++I"
      ],
      "SORT": [
        "V++I",
        "V++"
      ]
    },
    "consts": []
  },
  "mod_sound": {
    "deps": [],
    "funcs": {
      "SOUND_INIT": [
        ""
      ],
      "SOUND_CLOSE": [
        ""
      ],
      "LOAD_SONG": [
        "S",
        "SP"
      ],
      "UNLOAD_SONG": [
        "I",
        "P"
      ],
      "PLAY_SONG": [
        "II"
      ],
      "STOP_SONG": [
        ""
      ],
      "PAUSE_SONG": [
        ""
      ],
      "RESUME_SONG": [
        ""
      ],
      "SET_SONG_VOLUME": [
        "I"
      ],
      "IS_PLAYING_SONG": [
        ""
      ],
      "LOAD_WAV": [
        "S",
        "SP"
      ],
      "UNLOAD_WAV": [
        "I",
        "P"
      ],
      "PLAY_WAV": [
        "II",
        "III"
      ],
      "STOP_WAV": [
        "I"
      ],
      "PAUSE_WAV": [
        "I"
      ],
      "RESUME_WAV": [
        "I"
      ],
      "IS_PLAYING_WAV": [
        "I"
      ],
      "FADE_MUSIC_IN": [
        "III"
      ],
      "FADE_MUSIC_OFF": [
        "I"
      ],
      "SET_WAV_VOLUME": [
        "II"
      ],
      "SET_CHANNEL_VOLUME": [
        "II"
      ],
      "RESERVE_CHANNELS": [
        "I"
      ],
      "SET_PANNING": [
        "III"
      ],
      "SET_POSITION": [
        "III"
      ],
      "SET_DISTANCE": [
        "II"
      ],
      "REVERSE_STEREO": [
        "II"
      ],
      "SET_MUSIC_POSITION": [
        "F"
      ]
    },
    "consts": [
      "MODE_MONO",
      "MODE_STEREO",
      "ALL_SOUND"
    ]
  },
  "mod_string": {
    "deps": [],
    "funcs": {
      "STRLEN": [
        "S"
      ],
      "LEN": [
        "S"
      ],
      "UCASE": [
        "S"
      ],
      "LCASE": [
        "S"
      ],
      "STRCASECMP": [
        "SS"
      ],
      "SUBSTR": [
        "SII",
        "SI"
      ],
      "FIND": [
        "SS",
        "SSI"
      ],
      "LPAD": [
        "SI"
      ],
      "RPAD": [
        "SI"
      ],
      "ITOA": [
        "I"
      ],
      "FTOA": [
        "F"
      ],
      "ATOI": [
        "S"
      ],
      "ATOF": [
        "S"
      ],
      "ASC": [
        "S"
      ],
      "CHR": [
        "I"
      ],
      "TRIM": [
        "S"
      ],
      "STRREV": [
        "S"
      ],
      "FORMAT": [
        "I",
        "F",
        "FI"
      ]
    },
    "consts": []
  },
  "mod_sys": {
    "deps": [],
    "funcs": {
      "GETENV": [
        "S"
      ],
      "OS_NAME": [
        ""
      ],
      "EXEC": [
        "ISIP"
      ]
    },
    "consts": [
      "_P_WAIT",
      "_P_NOWAIT"
    ]
  },
  "mod_text": {
    "deps": [
      "libgrbase",
      "libblit",
      "libtext",
      "libfont"
    ],
    "funcs": {
      "WRITE": [
        "IIIIS",
        "IIIIIS"
      ],
      "WRITE_INT": [
        "IIIIP",
        "IIIIIP"
      ],
      "MOVE_TEXT": [
        "III",
        "IIII"
      ],
      "DELETE_TEXT": [
        "I"
      ],
      "WRITE_IN_MAP": [
        "ISI"
      ],
      "TEXT_WIDTH": [
        "IS"
      ],
      "TEXT_HEIGHT": [
        "IS"
      ],
      "GET_TEXT_COLOR": [
        "",
        "I"
      ],
      "SET_TEXT_COLOR": [
        "I",
        "II"
      ],
      "WRITE_VAR": [
        "IIIIV++",
        "IIIIIV++"
      ],
      "WRITE_FLOAT": [
        "IIIIP",
        "IIIIIP"
      ],
      "WRITE_STRING": [
        "IIIIP",
        "IIIIIP"
      ]
    },
    "consts": [
      "ALL_TEXT",
      "ALIGN_TOP_LEFT",
      "ALIGN_TOP",
      "ALIGN_TOP_RIGHT",
      "ALIGN_CENTER_LEFT",
      "ALIGN_CENTER",
      "ALIGN_CENTER_RIGHT",
      "ALIGN_BOTTOM_LEFT",
      "ALIGN_BOTTOM",
      "ALIGN_BOTTOM_RIGHT"
    ]
  },
  "mod_time": {
    "deps": [],
    "funcs": {
      "GET_TIMER": [
        ""
      ],
      "TIME": [
        ""
      ],
      "FTIME": [
        "SI"
      ]
    },
    "consts": []
  },
  "mod_timers": {
    "deps": [],
    "funcs": {},
    "consts": []
  },
  "mod_video": {
    "deps": [
      "libgrbase",
      "libvideo",
      "librender"
    ],
    "funcs": {
      "SET_MODE": [
        "I",
        "II",
        "III",
        "IIII"
      ],
      "SET_FPS": [
        "II"
      ],
      "GET_MODES": [
        "II"
      ],
      "MODE_IS_OK": [
        "IIII"
      ]
    },
    "consts": []
  },
  "mod_wm": {
    "deps": [
      "libgrbase",
      "libvideo",
      "libwm"
    ],
    "funcs": {
      "SET_TITLE": [
        "S"
      ],
      "SET_ICON": [
        "II"
      ],
      "MINIMIZE": [
        ""
      ],
      "MOVE_WINDOW": [
        "II"
      ],
      "SET_WINDOW_POS": [
        "II"
      ],
      "GET_WINDOW_POS": [
        "PP"
      ],
      "GET_WINDOW_SIZE": [
        "PPPP"
      ],
      "GET_DESKTOP_SIZE": [
        "PP"
      ]
    },
    "consts": []
  },
  "mod_wpad": {
    "deps": [
      "libjoy"
    ],
    "funcs": {
      "WPAD_IS_READY": [
        "I"
      ],
      "WPAD_INFO": [
        "II"
      ],
      "WPAD_INFO_BB": [
        "II"
      ],
      "WPAD_RUMBLE": [
        "II"
      ]
    },
    "consts": [
      "WPAD_BATT",
      "WPAD_X",
      "WPAD_Y",
      "WPAD_Z",
      "WPAD_ANGLE",
      "WPAD_PITCH",
      "WPAD_ROLL",
      "WPAD_ACCELX",
      "WPAD_ACCELY",
      "WPAD_ACCELZ",
      "WPAD_IS_BB",
      "WPAD_WTL",
      "WPAD_WTR",
      "WPAD_WBL",
      "WPAD_WBR"
    ]
  }
};
