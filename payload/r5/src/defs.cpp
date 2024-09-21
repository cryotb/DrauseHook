#include "inc/include.h"

__declspec(noinline) u64 decrypt_constant(u64 value)
{
	volatile u64 rs = __ROR8__(value ^ BUILD_SEED, 2); 
	return rs;
}
