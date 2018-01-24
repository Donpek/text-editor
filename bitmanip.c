#define BYTES(amount) (8 * (amount))

internal b32
BitManipIsLittleEndian(void)
{
	u32 Bytes = 0x0102;
	return(!!(Bytes & 0x02));
}

internal u32
BitManipCountNonZeroBytes(u32 Bytes)
{
	u32 Result = 0;
	for(u32 ByteIndex = 0;
			ByteIndex < sizeof(Bytes);
			++ByteIndex)
	{
		if(Bytes & 0xff)
		{
			++Result;
			Bytes >>= BYTES(1);
		}
	}
	return(Result);
}

internal u32
BitManipReverseBytes(u32 Bytes)
{
	u32 ByteCount = BitManipCountNonZeroBytes(Bytes);
	u32 Result = 0;
	for(u32 ByteIndex = 0;
			ByteIndex < ByteCount;
			++ByteIndex)
	{
		u8 Byte = (Bytes >> (BYTES(1) * ByteIndex)) & 0xff;
		Result |= Byte << (BYTES(ByteCount - 1) - (BYTES(1) * ByteIndex));
	}
	return(Result);
}
