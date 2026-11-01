#define MG_INST_MESS(mgId, no) MG_INST_##mgId##_##no
#define MG_INST_DECA_MESS(mgId, no) MG_INST_DECA_##mgId##_##no


MGDATA MgDataTbl[] = {
	{
        6,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M601,
		0x3c0000,
		{
			{ INSTPIC_ANM_m601 },
			{ INSTPIC_ANM_m601n }
		},
		{
			{ "movie/m601.thp", "", "" },
			{ "movie/m601n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_DECA_MESS(m601, 1),
				MG_INST_DECA_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        7,
        MG_TYPE_4P,
		MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M602,
		0x3d0000,
		{
			{ INSTPIC_ANM_m602 },
			{ INSTPIC_ANM_m602 }
		},
		{
			{ "movie/m602.thp", "", "" },
			{ "movie/m602.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m602, 1),
				MG_INST_MESS(m602, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m602, 3),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m602, 1),
				MG_INST_MESS(m602, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m602, 3),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        8,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M603,
		0x3e0000,
		{
			{ INSTPIC_ANM_m603 },
			{ INSTPIC_ANM_m603n }
		},
		{
			{ "movie/m603.thp", "", "" },
			{ "movie/m603n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m603, 1),
				MG_INST_MESS(m603, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m603, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m603, 1),
				MG_INST_MESS(m603, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m603, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        9,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M604,
		0x3f0000,
		{
			{ INSTPIC_ANM_m604e },
			{ INSTPIC_ANM_m604en }
		},
		{
			{ "movie/m604e.thp", "", "" },
			{ "movie/m604en.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m604, 1),
				MG_INST_MESS(m604, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m604, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m604n, 1),
				MG_INST_MESS(m604n, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m604n, 4),
				MG_INST_MESS_NULL,
			},

			{
				MG_INST_DECA_MESS(m604, 1),
				MG_INST_DECA_MESS(m604, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m604, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        10,
        MG_TYPE_4P,
		MG_FLAG_4P|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M605,
		0x400000,
		{
			{ INSTPIC_ANM_m605 },
			{ INSTPIC_ANM_m605 }
		},
		{
			{ "movie/m605.thp", "", "" },
			{ "movie/m605.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m605, 1),
				MG_INST_MESS(m605, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m605, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m605, 1),
				MG_INST_MESS(m605, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m605, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        11,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M606,
		0x410000,
		{
			{ INSTPIC_ANM_m606 },
			{ INSTPIC_ANM_m606n }
		},
		{
			{ "movie/m606.thp", "", "" },
			{ "movie/m606n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m606, 1),
				MG_INST_MESS(m606, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m606, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m606, 1),
				MG_INST_MESS(m606, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m606, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_DECA_MESS(m606, 1),
				MG_INST_DECA_MESS(m606, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m606, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        12,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M607,
		0x420000,
		{
			{ INSTPIC_ANM_m607 },
			{ INSTPIC_ANM_m607n }
		},
		{
			{ "movie/m607.thp", "", "" },
			{ "movie/m607n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m607, 1),
				MG_INST_MESS(m607, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m607, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m607, 1),
				MG_INST_MESS(m607, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m607, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        13,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M608,
		0x430000,
		{
			{ INSTPIC_ANM_m608 },
			{ INSTPIC_ANM_m608n }
		},
		{
			{ "movie/m608.thp", "", "" },
			{ "movie/m608n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m608, 1),
				MG_INST_MESS(m608, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m608, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m608, 1),
				MG_INST_MESS(m608, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m608, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_DECA_MESS(m608, 1),
				MG_INST_DECA_MESS(m608, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m608, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        14,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M609,
		0x440000,
		{
			{ INSTPIC_ANM_m609 },
			{ INSTPIC_ANM_m609n }
		},
		{
			{ "movie/m609.thp", "", "" },
			{ "movie/m609n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m609, 1),
				MG_INST_MESS(m609, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m609, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m609, 1),
				MG_INST_MESS(m609, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m609, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        15,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M610,
		0x450000,
		{
			{ INSTPIC_ANM_m610 },
			{ INSTPIC_ANM_m610n }
		},
		{
			{ "movie/m610.thp", "", "" },
			{ "movie/m610n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m610, 1),
				MG_INST_MESS(m610, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m610, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m610, 1),
				MG_INST_MESS(m610, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m610, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        16,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M611,
		0x460000,
		{
			{ INSTPIC_ANM_m611 },
			{ INSTPIC_ANM_m611n }
		},
		{
			{ "movie/m611.thp", "", "" },
			{ "movie/m611n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m611, 1),
				MG_INST_MESS(m611, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m611, 4),
				MG_INST_MESS(m611, 5),
			},
			{
				MG_INST_MESS(m611, 1),
				MG_INST_MESS(m611, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m611, 4),
				MG_INST_MESS(m611, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        17,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M612,
		0x470000,
		{
			{ INSTPIC_ANM_m612 },
			{ INSTPIC_ANM_m612n }
		},
		{
			{ "movie/m612.thp", "", "" },
			{ "movie/m612n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m612, 1),
				MG_INST_MESS(m612, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m612, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m612, 1),
				MG_INST_MESS(m612, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m612, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        18,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M613,
		0x480000,
		{
			{ INSTPIC_ANM_m613 },
			{ INSTPIC_ANM_m613n }
		},
		{
			{ "movie/m613.thp", "", "" },
			{ "movie/m613n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m613, 1),
				MG_INST_MESS(m613, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m613, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m613, 1),
				MG_INST_MESS(m613, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m613, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        19,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_ACTION|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M614,
		0x490000,
		{
			{ INSTPIC_ANM_m614 },
			{ INSTPIC_ANM_m614n }
		},
		{
			{ "movie/m614.thp", "", "" },
			{ "movie/m614n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m614, 1),
				MG_INST_MESS(m614, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m614, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m614, 1),
				MG_INST_MESS(m614, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m614, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        20,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_ACTION|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M615,
		0x4a0000,
		{
			{ INSTPIC_ANM_m615 },
			{ INSTPIC_ANM_m615n }
		},
		{
			{ "movie/m615.thp", "", "" },
			{ "movie/m615n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m615, 1),
				MG_INST_MESS(m615, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m615, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m615, 1),
				MG_INST_MESS(m615, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m615, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_DECA_MESS(m615, 1),
				MG_INST_DECA_MESS(m615, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m615, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        21,
        MG_TYPE_4P,
		MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M616,
		0x4b0000,
		{
			{ INSTPIC_ANM_m616 },
			{ INSTPIC_ANM_m616 }
		},
		{
			{ "movie/m616.thp", "", "" },
			{ "movie/m616.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m616, 1),
				MG_INST_MESS(m616, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m616, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m616, 1),
				MG_INST_MESS(m616, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m616, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        22,
        MG_TYPE_KETTOU,
		MG_FLAG_DISABLE_DAY|MG_FLAG_4P|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M617,
		0x4c0000,
		{
			{ INSTPIC_ANM_m617 },
			{ INSTPIC_ANM_m617 }
		},
		{
			{ "movie/m617.thp", "", "" },
			{ "movie/m617.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m617, 1),
				MG_INST_MESS(m617, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m617, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m617, 1),
				MG_INST_MESS(m617, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m617, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        23,
        MG_TYPE_4P,
		MG_FLAG_4P|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M618,
		0x4d0000,
		{
			{ INSTPIC_ANM_m618 },
			{ INSTPIC_ANM_m618 }
		},
		{
			{ "movie/m618.thp", "", "" },
			{ "movie/m618.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m618, 1),
				MG_INST_MESS(m618, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m618, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m618, 1),
				MG_INST_MESS(m618, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m618, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        24,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_SKILL|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M619,
		0x4e0000,
		{
			{ INSTPIC_ANM_m619 },
			{ INSTPIC_ANM_m619n }
		},
		{
			{ "movie/m619.thp", "", "" },
			{ "movie/m619n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m619, 1),
				MG_INST_MESS(m619, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m619, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m619, 1),
				MG_INST_MESS(m619, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m619, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        25,
        MG_TYPE_4P,
		MG_FLAG_DISABLE_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M620,
		0x4f0000,
		{
			{ INSTPIC_ANM_m620 },
			{ INSTPIC_ANM_m620 }
		},
		{
			{ "movie/m620.thp", "", "" },
			{ "movie/m620.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m620, 1),
				MG_INST_MESS(m620, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m620, 4),
				MG_INST_MESS(m620, 5),
			},
			{
				MG_INST_MESS(m620, 1),
				MG_INST_MESS(m620, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m620, 4),
				MG_INST_MESS(m620, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        26,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_ACTION|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M621,
		0x500000,
		{
			{ INSTPIC_ANM_m621 },
			{ INSTPIC_ANM_m621n }
		},
		{
			{ "movie/m621.thp", "", "" },
			{ "movie/m621n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m621, 1),
				MG_INST_MESS(m621, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m621, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m621, 1),
				MG_INST_MESS(m621, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m621, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_DECA_MESS(m621, 1),
				MG_INST_DECA_MESS(m621, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m621, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        27,
        MG_TYPE_4P,
		MG_FLAG_COIN|MG_FLAG_4P|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M622,
		0x510000,
		{
			{ INSTPIC_ANM_m622 },
			{ INSTPIC_ANM_m622 }
		},
		{
			{ "movie/m622.thp", "", "" },
			{ "movie/m622.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m622, 1),
				MG_INST_MESS(m622, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m622, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m622, 1),
				MG_INST_MESS(m622, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m622, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        28,
        MG_TYPE_1VS3,
		MG_FLAG_HAS_NIGHT|MG_FLAG_COIN|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M623,
		0x520000,
		{
			{ INSTPIC_ANM_m623 },
			{ INSTPIC_ANM_m623n }
		},
		{
			{ "movie/m623.thp", "", "" },
			{ "movie/m623n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m623, 1),
				MG_INST_MESS(m623, 2),
				MG_INST_MESS(m623, 3),
				MG_INST_MESS(m623, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m623, 1),
				MG_INST_MESS(m623, 2),
				MG_INST_MESS(m623, 3),
				MG_INST_MESS(m623, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        29,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M624,
		0x530000,
		{
			{ INSTPIC_ANM_m624 },
			{ INSTPIC_ANM_m624 }
		},
		{
			{ "movie/m624.thp", "", "" },
			{ "movie/m624.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m624, 1),
				MG_INST_MESS(m624, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m624, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m624, 1),
				MG_INST_MESS(m624, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m624, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        30,
        MG_TYPE_1VS3,
		MG_FLAG_HAS_NIGHT|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M625,
		0x540000,
		{
			{ INSTPIC_ANM_m625 },
			{ INSTPIC_ANM_m625n }
		},
		{
			{ "movie/m625.thp", "", "" },
			{ "movie/m625n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m625, 1),
				MG_INST_MESS(m625, 2),
				MG_INST_MESS(m625, 3),
				MG_INST_MESS(m625, 4),
				MG_INST_MESS(m625, 5),
			},
			{
				MG_INST_MESS(m625, 1),
				MG_INST_MESS(m625, 2),
				MG_INST_MESS(m625, 3),
				MG_INST_MESS(m625, 4),
				MG_INST_MESS(m625, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        31,
        MG_TYPE_1VS3,
		MG_FLAG_HAS_NIGHT|MG_FLAG_PACK_EASY|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M626,
		0x550000,
		{
			{ INSTPIC_ANM_m626 },
			{ INSTPIC_ANM_m626n }
		},
		{
			{ "movie/m626.thp", "", "" },
			{ "movie/m626n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m626, 1),
				MG_INST_MESS(m626, 2),
				MG_INST_MESS(m626, 3),
				MG_INST_MESS(m626, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m626, 1),
				MG_INST_MESS(m626, 2),
				MG_INST_MESS(m626, 3),
				MG_INST_MESS(m626, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        32,
        MG_TYPE_1VS3,
		MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M627,
		0x560000,
		{
			{ INSTPIC_ANM_m627 },
			{ INSTPIC_ANM_m627 }
		},
		{
			{ "movie/m627.thp", "", "" },
			{ "movie/m627.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m627, 1),
				MG_INST_MESS(m627, 2),
				MG_INST_MESS(m627, 3),
				MG_INST_MESS(m627, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m627, 1),
				MG_INST_MESS(m627, 2),
				MG_INST_MESS(m627, 3),
				MG_INST_MESS(m627, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        33,
        MG_TYPE_1VS3,
		MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M628,
		0x570000,
		{
			{ INSTPIC_ANM_m628 },
			{ INSTPIC_ANM_m628 }
		},
		{
			{ "movie/m628.thp", "", "" },
			{ "movie/m628.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m628, 1),
				MG_INST_MESS(m628, 2),
				MG_INST_MESS(m628, 3),
				MG_INST_MESS(m628, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m628, 1),
				MG_INST_MESS(m628, 2),
				MG_INST_MESS(m628, 3),
				MG_INST_MESS(m628, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        34,
        MG_TYPE_1VS3,
		MG_FLAG_HAS_NIGHT|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M629,
		0x580000,
		{
			{ INSTPIC_ANM_m629 },
			{ INSTPIC_ANM_m629n }
		},
		{
			{ "movie/m629.thp", "", "" },
			{ "movie/m629n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m629, 1),
				MG_INST_MESS(m629, 2),
				MG_INST_MESS(m629, 3),
				MG_INST_MESS(m629, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m629, 1),
				MG_INST_MESS(m629, 2),
				MG_INST_MESS(m629, 3),
				MG_INST_MESS(m629, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        35,
        MG_TYPE_1VS3,
		MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M630,
		0x590000,
		{
			{ INSTPIC_ANM_m630 },
			{ INSTPIC_ANM_m630 }
		},
		{
			{ "movie/m630.thp", "", "" },
			{ "movie/m630.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m630, 1),
				MG_INST_MESS(m630, 2),
				MG_INST_MESS(m630, 3),
				MG_INST_MESS(m630, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m630, 1),
				MG_INST_MESS(m630, 2),
				MG_INST_MESS(m630, 3),
				MG_INST_MESS(m630, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        36,
        MG_TYPE_1VS3,
		MG_FLAG_HAS_NIGHT|MG_FLAG_PACK_EASY|MG_FLAG_PACK_ACTION|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M631,
		0x5a0000,
		{
			{ INSTPIC_ANM_m631 },
			{ INSTPIC_ANM_m631n }
		},
		{
			{ "movie/m631.thp", "", "" },
			{ "movie/m631n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m631, 1),
				MG_INST_MESS(m631, 2),
				MG_INST_MESS(m631, 3),
				MG_INST_MESS(m631, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m631n, 1),
				MG_INST_MESS(m631n, 2),
				MG_INST_MESS(m631n, 3),
				MG_INST_MESS(m631n, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        37,
        MG_TYPE_1VS3,
		MG_FLAG_PACK_EASY|MG_FLAG_PACK_ACTION|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M632,
		0x5b0000,
		{
			{ INSTPIC_ANM_m632 },
			{ INSTPIC_ANM_m632 }
		},
		{
			{ "movie/m632.thp", "", "" },
			{ "movie/m632.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m632, 1),
				MG_INST_MESS(m632, 2),
				MG_INST_MESS(m632, 3),
				MG_INST_MESS(m632, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m632, 1),
				MG_INST_MESS(m632, 2),
				MG_INST_MESS(m632, 3),
				MG_INST_MESS(m632, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        38,
        MG_TYPE_1VS3,
		MG_FLAG_PACK_ACTION|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M633,
		0x5c0000,
		{
			{ INSTPIC_ANM_m633 },
			{ INSTPIC_ANM_m633 }
		},
		{
			{ "movie/m633.thp", "", "" },
			{ "movie/m633.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m633, 1),
				MG_INST_MESS(m633, 2),
				MG_INST_MESS(m633, 3),
				MG_INST_MESS(m633, 4),
				MG_INST_MESS(m633, 5),
			},
			{
				MG_INST_MESS(m633, 1),
				MG_INST_MESS(m633, 2),
				MG_INST_MESS(m633, 3),
				MG_INST_MESS(m633, 4),
				MG_INST_MESS(m633, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        39,
        MG_TYPE_1VS3,
		MG_FLAG_HAS_NIGHT|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M634,
		0x5d0000,
		{
			{ INSTPIC_ANM_m634 },
			{ INSTPIC_ANM_m634n }
		},
		{
			{ "movie/m634.thp", "", "" },
			{ "movie/m634n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m634, 1),
				MG_INST_MESS(m634, 2),
				MG_INST_MESS(m634, 3),
				MG_INST_MESS(m634, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m634, 1),
				MG_INST_MESS(m634, 2),
				MG_INST_MESS(m634, 3),
				MG_INST_MESS(m634, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        40,
        MG_TYPE_2VS2,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M635,
		0x5e0000,
		{
			{ INSTPIC_ANM_m635e },
			{ INSTPIC_ANM_m635en }
		},
		{
			{ "movie/m635e.thp", "", "" },
			{ "movie/m635en.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m635, 1),
				MG_INST_MESS(m635, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m635, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m635, 1),
				MG_INST_MESS(m635, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m635, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        41,
        MG_TYPE_2VS2,
		MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M636,
		0x5f0000,
		{
			{ INSTPIC_ANM_m636 },
			{ INSTPIC_ANM_m636 }
		},
		{
			{ "movie/m636.thp", "", "" },
			{ "movie/m636.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m636, 1),
				MG_INST_MESS(m636, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m636, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m636, 1),
				MG_INST_MESS(m636, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m636, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        42,
        MG_TYPE_2VS2,
		MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M637,
		0x600000,
		{
			{ INSTPIC_ANM_m637 },
			{ INSTPIC_ANM_m637 }
		},
		{
			{ "movie/m637.thp", "", "" },
			{ "movie/m637.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m637, 1),
				MG_INST_MESS(m637, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m637, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m637, 1),
				MG_INST_MESS(m637, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m637, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        43,
        MG_TYPE_2VS2,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M638,
		0x610000,
		{
			{ INSTPIC_ANM_m638 },
			{ INSTPIC_ANM_m638n }
		},
		{
			{ "movie/m638.thp", "", "" },
			{ "movie/m638n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m638, 1),
				MG_INST_MESS(m638, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m638, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m638, 1),
				MG_INST_MESS(m638, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m638, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        44,
        MG_TYPE_2VS2,
		MG_FLAG_DISABLE_DAY|MG_FLAG_4P|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M639,
		0x620000,
		{
			{ INSTPIC_ANM_m639 },
			{ INSTPIC_ANM_m639 }
		},
		{
			{ "movie/m639.thp", "", "" },
			{ "movie/m639.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m639, 1),
				MG_INST_MESS(m639, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m639, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m639, 1),
				MG_INST_MESS(m639, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m639, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        45,
        MG_TYPE_2VS2,
		MG_FLAG_4P|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M640,
		0x630000,
		{
			{ INSTPIC_ANM_m640 },
			{ INSTPIC_ANM_m640 }
		},
		{
			{ "movie/m640.thp", "", "" },
			{ "movie/m640.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m640, 1),
				MG_INST_MESS(m640, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m640, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m640, 1),
				MG_INST_MESS(m640, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m640, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        46,
        MG_TYPE_2VS2,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M641,
		0x640000,
		{
			{ INSTPIC_ANM_m641 },
			{ INSTPIC_ANM_m641n }
		},
		{
			{ "movie/m641.thp", "", "" },
			{ "movie/m641n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m641, 1),
				MG_INST_MESS(m641, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m641, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m641n, 1),
				MG_INST_MESS(m641n, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m641n, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        47,
        MG_TYPE_2VS2,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_COIN|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M642,
		0x650000,
		{
			{ INSTPIC_ANM_m642 },
			{ INSTPIC_ANM_m642n }
		},
		{
			{ "movie/m642.thp", "", "" },
			{ "movie/m642n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m642, 1),
				MG_INST_MESS(m642, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m642, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m642, 1),
				MG_INST_MESS(m642, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m642, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        48,
        MG_TYPE_2VS2,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_GRPORDER|MG_FLAG_PACK_ACTION|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M643,
		0x660000,
		{
			{ INSTPIC_ANM_m643 },
			{ INSTPIC_ANM_m643n }
		},
		{
			{ "movie/m643.thp", "", "" },
			{ "movie/m643n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m643, 1),
				MG_INST_MESS(m643, 2),
				MG_INST_MESS(m643, 3),
				MG_INST_MESS(m643, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m643, 1),
				MG_INST_MESS(m643, 2),
				MG_INST_MESS(m643, 3),
				MG_INST_MESS(m643, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        49,
        MG_TYPE_2VS2,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_ACTION|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M644,
		0x670000,
		{
			{ INSTPIC_ANM_m644 },
			{ INSTPIC_ANM_m644n }
		},
		{
			{ "movie/m644.thp", "", "" },
			{ "movie/m644n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m644, 1),
				MG_INST_MESS(m644, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m644, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m644, 1),
				MG_INST_MESS(m644, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m644, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        50,
        MG_TYPE_2VS2,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_GRPORDER|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M645,
		0x680000,
		{
			{ INSTPIC_ANM_m645 },
			{ INSTPIC_ANM_m645n }
		},
		{
			{ "movie/m645.thp", "", "" },
			{ "movie/m645n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m645, 1),
				MG_INST_MESS(m645, 2),
				MG_INST_MESS(m645, 3),
				MG_INST_MESS(m645, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m645, 1),
				MG_INST_MESS(m645, 2),
				MG_INST_MESS(m645, 3),
				MG_INST_MESS(m645, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        51,
        MG_TYPE_BATTLE,
		MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M646,
		0x690000,
		{
			{ INSTPIC_ANM_m646 },
			{ INSTPIC_ANM_m646 }
		},
		{
			{ "movie/m646.thp", "", "" },
			{ "movie/m646.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m646, 1),
				MG_INST_MESS(m646, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m646, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m646, 1),
				MG_INST_MESS(m646, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m646, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_DECA_MESS(m646, 1),
				MG_INST_DECA_MESS(m646, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m646, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        52,
        MG_TYPE_BATTLE,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M647,
		0x6a0000,
		{
			{ INSTPIC_ANM_m647 },
			{ INSTPIC_ANM_m647n }
		},
		{
			{ "movie/m647.thp", "", "" },
			{ "movie/m647n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m647, 1),
				MG_INST_MESS(m647, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m647, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m647, 1),
				MG_INST_MESS(m647, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m647, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        53,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M648,
		0x6b0000,
		{
			{ INSTPIC_ANM_m648 },
			{ INSTPIC_ANM_m648n }
		},
		{
			{ "movie/m648.thp", "", "" },
			{ "movie/m648n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m648, 1),
				MG_INST_MESS(m648, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m648, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m648, 1),
				MG_INST_MESS(m648, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m648, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_DECA_MESS(m648, 1),
				MG_INST_DECA_MESS(m648, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m648, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        54,
        MG_TYPE_BATTLE,
		MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M649,
		0x6c0000,
		{
			{ INSTPIC_ANM_m649 },
			{ INSTPIC_ANM_m649 }
		},
		{
			{ "movie/m649.thp", "", "" },
			{ "movie/m649.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m649, 1),
				MG_INST_MESS(m649, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m649, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m649, 1),
				MG_INST_MESS(m649, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m649, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_DECA_MESS(m649, 1),
				MG_INST_DECA_MESS(m649, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m649, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        55,
        MG_TYPE_4P,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M650,
		0x6d0000,
		{
			{ INSTPIC_ANM_m650 },
			{ INSTPIC_ANM_m650n }
		},
		{
			{ "movie/m650.thp", "", "" },
			{ "movie/m650n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m650, 1),
				MG_INST_MESS(m650, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m650, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m650, 1),
				MG_INST_MESS(m650, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m650, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_DECA_MESS(m650, 1),
				MG_INST_DECA_MESS(m650, 2),
				MG_INST_MESS_NULL,
				MG_INST_DECA_MESS(m650, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        56,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M651,
		0x6e0000,
		{
			{ INSTPIC_ANM_m651 },
			{ INSTPIC_ANM_m651 }
		},
		{
			{ "movie/m651.thp", "", "" },
			{ "movie/m651.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m651, 1),
				MG_INST_MESS(m651, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m651, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m651, 1),
				MG_INST_MESS(m651, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m651, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        57,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M652,
		0x6f0000,
		{
			{ INSTPIC_ANM_m652 },
			{ INSTPIC_ANM_m652 }
		},
		{
			{ "movie/m652.thp", "", "" },
			{ "movie/m652.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m652, 1),
				MG_INST_MESS(m652, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m652, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m652, 1),
				MG_INST_MESS(m652, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m652, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        58,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M653,
		0x700000,
		{
			{ INSTPIC_ANM_m653 },
			{ INSTPIC_ANM_m653 }
		},
		{
			{ "movie/m653.thp", "", "" },
			{ "movie/m653.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m653, 1),
				MG_INST_MESS(m653, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m653, 4),
				MG_INST_MESS(m653, 5),
			},
			{
				MG_INST_MESS(m653, 1),
				MG_INST_MESS(m653, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m653, 4),
				MG_INST_MESS(m653, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        59,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M654,
		0x710000,
		{
			{ INSTPIC_ANM_m654 },
			{ INSTPIC_ANM_m654 }
		},
		{
			{ "movie/m654.thp", "", "" },
			{ "movie/m654.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m654, 1),
				MG_INST_MESS(m654, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m654, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m654, 1),
				MG_INST_MESS(m654, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m654, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        60,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_SKILL|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M655,
		0x720000,
		{
			{ INSTPIC_ANM_m655 },
			{ INSTPIC_ANM_m655 }
		},
		{
			{ "movie/m655.thp", "", "" },
			{ "movie/m655.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m655, 1),
				MG_INST_MESS(m655, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m655, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m655, 1),
				MG_INST_MESS(m655, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m655, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        61,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M656,
		0x730000,
		{
			{ INSTPIC_ANM_m656 },
			{ INSTPIC_ANM_m656 }
		},
		{
			{ "movie/m656.thp", "", "" },
			{ "movie/m656.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m656, 1),
				MG_INST_MESS(m656, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m656, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m656, 1),
				MG_INST_MESS(m656, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m656, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        62,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_EASY,
		MG_RECORD_NONE,
		MG_NAME_M657,
		0x740000,
		{
			{ INSTPIC_ANM_m657 },
			{ INSTPIC_ANM_m657 }
		},
		{
			{ "movie/m657.thp", "", "" },
			{ "movie/m657.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m657, 1),
				MG_INST_MESS(m657, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m657, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m657, 1),
				MG_INST_MESS(m657, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m657, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        63,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M658,
		0x750000,
		{
			{ INSTPIC_ANM_m658 },
			{ INSTPIC_ANM_m658 }
		},
		{
			{ "movie/m658.thp", "", "" },
			{ "movie/m658.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m658, 1),
				MG_INST_MESS(m658, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m658, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m658, 1),
				MG_INST_MESS(m658, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m658, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        64,
        MG_TYPE_KETTOU,
		MG_FLAG_4P|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M659,
		0x760000,
		{
			{ INSTPIC_ANM_m659 },
			{ INSTPIC_ANM_m659 }
		},
		{
			{ "movie/m659.thp", "", "" },
			{ "movie/m659.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m659, 1),
				MG_INST_MESS(m659, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m659, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m659, 1),
				MG_INST_MESS(m659, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m659, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        65,
        MG_TYPE_KETTOU,
		MG_FLAG_DISABLE_DAY|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M660,
		0x770000,
		{
			{ INSTPIC_ANM_m660 },
			{ INSTPIC_ANM_m660 }
		},
		{
			{ "movie/m660.thp", "", "" },
			{ "movie/m660.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m660, 1),
				MG_INST_MESS(m660, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m660, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m660, 1),
				MG_INST_MESS(m660, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m660, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        66,
        MG_TYPE_KETTOU,
		MG_FLAG_DISABLE_DAY|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M661,
		0x780000,
		{
			{ INSTPIC_ANM_m661 },
			{ INSTPIC_ANM_m661 }
		},
		{
			{ "movie/m661.thp", "", "" },
			{ "movie/m661.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m661, 1),
				MG_INST_MESS(m661, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m661, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m661, 1),
				MG_INST_MESS(m661, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m661, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        67,
        MG_TYPE_KETTOU,
		MG_FLAG_DISABLE_DAY|MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M662,
		0x790000,
		{
			{ INSTPIC_ANM_m662 },
			{ INSTPIC_ANM_m662 }
		},
		{
			{ "movie/m662.thp", "", "" },
			{ "movie/m662.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m662, 1),
				MG_INST_MESS(m662, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m662, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m662, 1),
				MG_INST_MESS(m662, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m662, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        68,
        MG_TYPE_KETTOU,
		MG_FLAG_DISABLE_DAY|MG_FLAG_4P|MG_FLAG_PACK_WEIRD,
		MG_RECORD_NONE,
		MG_NAME_M663,
		0x7a0000,
		{
			{ INSTPIC_ANM_m663 },
			{ INSTPIC_ANM_m663 }
		},
		{
			{ "movie/m663.thp", "", "" },
			{ "movie/m663.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m663, 1),
				MG_INST_MESS(m663, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m663, 4),
				MG_INST_MESS(m663, 5),
			},
			{
				MG_INST_MESS(m663, 1),
				MG_INST_MESS(m663, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m663, 4),
				MG_INST_MESS(m663, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        69,
        MG_TYPE_BATTLE,
		MG_FLAG_HAS_NIGHT|MG_FLAG_4P|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M664,
		0x7b0000,
		{
			{ INSTPIC_ANM_m664 },
			{ INSTPIC_ANM_m664n }
		},
		{
			{ "movie/m664.thp", "", "" },
			{ "movie/m664n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m664, 1),
				MG_INST_MESS(m664, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m664, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m664, 1),
				MG_INST_MESS(m664, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m664, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        70,
        MG_TYPE_1VS3,
		MG_FLAG_MIC|MG_FLAG_4P|MG_FLAG_HAS_NIGHT|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M665,
		0x7c0000,
		{
			{ INSTPIC_ANM_m665 },
			{ INSTPIC_ANM_m665n }
		},
		{
			{ "movie/m665.thp", "", "" },
			{ "movie/m665n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m665, 1),
				MG_INST_MESS(m665, 2),
				MG_INST_MESS(m665, 3),
				MG_INST_MESS(m665, 4),
				MG_INST_MESS(m665, 5),
			},
			{
				MG_INST_MESS(m665, 1),
				MG_INST_MESS(m665, 2),
				MG_INST_MESS(m665, 3),
				MG_INST_MESS(m665, 4),
				MG_INST_MESS(m665, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        71,
        MG_TYPE_1VS3,
		MG_FLAG_MIC|MG_FLAG_4P|MG_FLAG_HAS_NIGHT|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M666,
		0x7d0000,
		{
			{ INSTPIC_ANM_m666 },
			{ INSTPIC_ANM_m666n }
		},
		{
			{ "movie/m666.thp", "", "" },
			{ "movie/m666n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m666, 1),
				MG_INST_MESS(m666, 2),
				MG_INST_MESS(m666, 3),
				MG_INST_MESS(m666, 4),
				MG_INST_MESS(m666, 5),
			},
			{
				MG_INST_MESS(m666, 1),
				MG_INST_MESS(m666, 2),
				MG_INST_MESS(m666, 3),
				MG_INST_MESS(m666, 4),
				MG_INST_MESS(m666, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        72,
        MG_TYPE_1VS3,
		MG_FLAG_MIC|MG_FLAG_4P|MG_FLAG_HAS_NIGHT|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M667,
		0x7e0000,
		{
			{ INSTPIC_ANM_m667 },
			{ INSTPIC_ANM_m667n }
		},
		{
			{ "movie/m667.thp", "", "" },
			{ "movie/m667n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m667, 1),
				MG_INST_MESS(m667, 2),
				MG_INST_MESS(m667, 3),
				MG_INST_MESS(m667, 4),
				MG_INST_MESS(m667, 5),
			},
			{
				MG_INST_MESS(m667, 1),
				MG_INST_MESS(m667, 2),
				MG_INST_MESS(m667, 3),
				MG_INST_MESS(m667, 4),
				MG_INST_MESS(m667, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        73,
        MG_TYPE_2VS2,
		MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M668,
		0x7f0000,
		{
			{ INSTPIC_ANM_m668 },
			{ INSTPIC_ANM_m668 }
		},
		{
			{ "movie/m668.thp", "", "" },
			{ "movie/m668.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m668, 1),
				MG_INST_MESS(m668, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m668, 4),
				MG_INST_MESS(m668, 5),
			},
			{
				MG_INST_MESS(m668, 1),
				MG_INST_MESS(m668, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m668, 4),
				MG_INST_MESS(m668, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        74,
        MG_TYPE_1VS3,
		MG_FLAG_MIC|MG_FLAG_4P|MG_FLAG_HAS_NIGHT|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M669,
		0x800000,
		{
			{ INSTPIC_ANM_m669 },
			{ INSTPIC_ANM_m669n }
		},
		{
			{ "movie/m669.thp", "", "" },
			{ "movie/m669n.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m669, 1),
				MG_INST_MESS(m669, 2),
				MG_INST_MESS(m669, 3),
				MG_INST_MESS(m669, 4),
				MG_INST_MESS(m669, 5),
			},
			{
				MG_INST_MESS(m669, 1),
				MG_INST_MESS(m669, 2),
				MG_INST_MESS(m669, 3),
				MG_INST_MESS(m669, 4),
				MG_INST_MESS(m669, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        75,
        MG_TYPE_1VS3,
		MG_FLAG_MIC|MG_FLAG_4P|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M670,
		0x810000,
		{
			{ INSTPIC_ANM_m670 },
			{ INSTPIC_ANM_m670 }
		},
		{
			{ "movie/m670.thp", "", "" },
			{ "movie/m670.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m670, 1),
				MG_INST_MESS(m670, 2),
				MG_INST_MESS(m670, 3),
				MG_INST_MESS(m670, 4),
				MG_INST_MESS(m670, 5),
			},
			{
				MG_INST_MESS(m670, 1),
				MG_INST_MESS(m670, 2),
				MG_INST_MESS(m670, 3),
				MG_INST_MESS(m670, 4),
				MG_INST_MESS(m670, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        76,
        MG_TYPE_KUPA,
		MG_FLAG_4P|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M671,
		0x820000,
		{
			{ INSTPIC_ANM_m671 },
			{ INSTPIC_ANM_m671 }
		},
		{
			{ "movie/m671.thp", "", "" },
			{ "movie/m671.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m671, 1),
				MG_INST_MESS(m671, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m671, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m671, 1),
				MG_INST_MESS(m671, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m671, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        77,
        MG_TYPE_KUPA,
		MG_FLAG_4P|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M672,
		0x830000,
		{
			{ INSTPIC_ANM_m672 },
			{ INSTPIC_ANM_m672 }
		},
		{
			{ "movie/m672.thp", "", "" },
			{ "movie/m672.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m672, 1),
				MG_INST_MESS(m673, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m672, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m672, 1),
				MG_INST_MESS(m672, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m672, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        78,
        MG_TYPE_KUPA,
		MG_FLAG_4P|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M673,
		0x840000,
		{
			{ INSTPIC_ANM_m673 },
			{ INSTPIC_ANM_m673 }
		},
		{
			{ "movie/m673.thp", "", "" },
			{ "movie/m673.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m673, 1),
				MG_INST_MESS(m673, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m673, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m673, 1),
				MG_INST_MESS(m673, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m673, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        79,
        MG_TYPE_DONKEY,
		MG_FLAG_4P|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M674,
		0x850000,
		{
			{ INSTPIC_ANM_m674 },
			{ INSTPIC_ANM_m674 }
		},
		{
			{ "movie/m674.thp", "", "" },
			{ "movie/m674.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m674, 1),
				MG_INST_MESS(m674, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m674, 4),
				MG_INST_MESS(m674, 5),
			},
			{
				MG_INST_MESS(m674, 1),
				MG_INST_MESS(m674, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m674, 4),
				MG_INST_MESS(m674, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        80,
        MG_TYPE_DONKEY,
		MG_FLAG_4P|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M675,
		0x860000,
		{
			{ INSTPIC_ANM_m675 },
			{ INSTPIC_ANM_m675 }
		},
		{
			{ "movie/m675.thp", "", "" },
			{ "movie/m675.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m675, 1),
				MG_INST_MESS(m675, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m675, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m675, 1),
				MG_INST_MESS(m675, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m675, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        81,
        MG_TYPE_DONKEY,
		MG_FLAG_4P|MG_FLAG_PACK_ALL,
		MG_RECORD_NONE,
		MG_NAME_M676,
		0x870000,
		{
			{ INSTPIC_ANM_m676 },
			{ INSTPIC_ANM_m676 }
		},
		{
			{ "movie/m676.thp", "", "" },
			{ "movie/m676.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m676, 1),
				MG_INST_MESS(m676, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m676, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m676, 1),
				MG_INST_MESS(m676, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m676, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        82,
        MG_TYPE_LAST,
		MG_FLAG_RARE,
		MG_RECORD_NONE,
		MG_NAME_M677,
		0x880000,
		{
			{ INSTPIC_ANM_m677e },
			{ INSTPIC_ANM_m677e }
		},
		{
			{ "movie/m677e.thp", "", "" },
			{ "movie/m677e.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m677, 1),
				MG_INST_MESS(m677, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m677, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m677, 1),
				MG_INST_MESS(m677, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m677, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        83,
        MG_TYPE_678,
		MG_FLAG_RARE,
		MG_RECORD_NONE,
		MG_NAME_M678,
		0x890000,
		{
			{ INSTPIC_ANM_m678e_1, INSTPIC_ANM_m678 },
			{ INSTPIC_ANM_m678e_1, INSTPIC_ANM_m678 }
		},
		{
			{ "movie/m678e_1.thp", "movie/m678.thp", "" },
			{ "movie/m678e_1.thp", "movie/m678.thp", "" },
		},
		{
			{
				MG_INST_MESS(m678, 1),
				MG_INST_MESS(m678, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m678, 4),
				MG_INST_MESS(m678, 5),
			},
			{
				MG_INST_MESS(m678_1, 1),
				MG_INST_MESS(m678_1, 2),
				MG_INST_MESS(m678_1, 3),
				MG_INST_MESS(m678_1, 4),
				MG_INST_MESS(m678_1, 5),
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        84,
        MG_TYPE_LAST,
		MG_FLAG_RARE,
		MG_RECORD_NONE,
		MG_NAME_M679,
		0x8a0000,
		{
			{ INSTPIC_ANM_m679 },
			{ INSTPIC_ANM_m679 }
		},
		{
			{ "movie/m679.thp", "", "" },
			{ "movie/m679.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m679, 1),
				MG_INST_MESS(m679, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m679, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m679, 1),
				MG_INST_MESS(m679, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m679, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        85,
        MG_TYPE_BATTLE,
		MG_FLAG_4P|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_SKILL,
		MG_RECORD_NONE,
		MG_NAME_M680,
		0x8b0000,
		{
			{ INSTPIC_ANM_m680 },
			{ INSTPIC_ANM_m680 }
		},
		{
			{ "movie/m680.thp", "", "" },
			{ "movie/m680.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m680, 1),
				MG_INST_MESS(m680, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m680, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m680, 1),
				MG_INST_MESS(m680, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m680, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        86,
        MG_TYPE_BATTLE,
		MG_FLAG_4P|MG_FLAG_PACK_EASY|MG_FLAG_PACK_WEIRD|MG_FLAG_PACK_ACTION,
		MG_RECORD_NONE,
		MG_NAME_M681,
		0x8c0000,
		{
			{ INSTPIC_ANM_m681 },
			{ INSTPIC_ANM_m681 }
		},
		{
			{ "movie/m681.thp", "", "" },
			{ "movie/m681.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m681, 1),
				MG_INST_MESS(m681, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m681, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m681, 1),
				MG_INST_MESS(m681, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m681, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        87,
        MG_TYPE_LAST,
		MG_FLAG_RARE,
		MG_RECORD_NONE,
		MG_NAME_M699,
		0x8d0000,
		{
			{ INSTPIC_ANM_m699 },
			{ INSTPIC_ANM_m699 }
		},
		{
			{ "movie/m699.thp", "", "" },
			{ "movie/m699.thp", "", "" },
		},
		{
			{
				MG_INST_MESS(m699, 1),
				MG_INST_MESS(m699, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m699, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m699, 1),
				MG_INST_MESS(m699, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m699, 4),
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS(m601, 1),
				MG_INST_MESS(m601, 2),
				MG_INST_MESS_NULL,
				MG_INST_MESS(m601, 4),
				MG_INST_MESS_NULL,
			}
		}
    },
	{
        -1,
        MG_TYPE_4P,
		0,
		MG_RECORD_NONE,
		0,
		0,
		{
			{ 0 },
			{ 0 }
		},
		{
			{ "", "", "" },
			{ "", "", "" },
		},
		{
			{
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
			},
			{
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
				MG_INST_MESS_NULL,
			}
		}
    },
};

#undef MG_INST_MESS
#undef MG_INST_DECA_MESS
