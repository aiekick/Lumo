# CS_ARCH_ARM, CS_MODE_ARM, None
0x91,0x08,0x20,0xfc = vfmal.f16 d0, s1, s2
0x91,0x08,0xa0,0xfc = vfmsl.f16 d0, s1, s2
0x52,0x08,0x21,0xfc = vfmal.f16 q0, d1, d2
0x52,0x08,0xa1,0xfc = vfmsl.f16 q0, d1, d2
0x99,0x08,0x00,0xfe = vfmal.f16 d0, s1, s2[1]
0x99,0x08,0x10,0xfe = vfmsl.f16 d0, s1, s2[1]
0x7a,0x08,0x01,0xfe = vfmal.f16 q0, d1, d2[3]
0x7a,0x08,0x11,0xfe = vfmsl.f16 q0, d1, d2[3]
