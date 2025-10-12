//TODO: Find a way to query the max size of the used gpu

#pragma once

#define LUCUMA_FDTD_WG_SIZE_X 8
#define LUCUMA_FDTD_WG_SIZE_Y 8
#define LUCUMA_FDTD_WG_SIZE_Z 8

#define LUCUMA_FDTD_WG_SIZES LUCUMA_FDTD_WG_SIZE_X, LUCUMA_FDTD_WG_SIZE_Y, LUCUMA_FDTD_WG_SIZE_Z

#ifndef FDTD_FLOAT_T
#define FDTD_FLOAT_T float
#endif
