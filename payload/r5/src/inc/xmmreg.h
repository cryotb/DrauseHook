#pragma once

extern "C"
{
	 void read_xmm6(float* out);
	 void read_xmm7(float* out);
	 void read_xmm8(float* out);

	 void write_xmm6(float val);
	 void write_xmm7(float val);
	 void write_xmm8(float val);
}

