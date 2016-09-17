import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.UUID;


public class STXProtocol
{
	ByteOrder m_byteOrder = ByteOrder.nativeOrder();
	int m_nCurentWriteOffset = 0;
	int m_nBufferLen = 1024;			//Content size (without prefix)
	byte[] m_pBuffer = new byte[1024];
	int m_nCurentReadOffset = 0;
	byte m_nPrefixLen = 1;
        byte _crc = 0;
        
        final byte _maxPrefixLength = 8;
	
	final byte STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE = 16;
	
	final byte STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX = (byte)0x80;
	final byte STXPROTOCOL_DATA_TYPE_FLAG_SECOND_PREFIX = (byte)0x40;
	final byte STXPROTOCOL_DATA_TYPE_FLAG_PAIR = (byte)0x20;
	final byte STXPROTOCOL_DATA_TYPE_INVALID =	0;
	final byte STXPROTOCOL_DATA_TYPE_BYTE = 	1;
	final byte STXPROTOCOL_DATA_TYPE_WORD = 	2;
	final byte STXPROTOCOL_DATA_TYPE_DWORD = 	3;
	final byte STXPROTOCOL_DATA_TYPE_I64 = 		4;
	final byte STXPROTOCOL_DATA_TYPE_UTF8 = 	(5 | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX);
	final byte STXPROTOCOL_DATA_TYPE_UNICODE = 	(6 | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX);
	final byte STXPROTOCOL_DATA_TYPE_GUID = 	7;
	final byte STXPROTOCOL_DATA_TYPE_OBJECT	=	(8 | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX);
	final byte STXPROTOCOL_DATA_TYPE_RAW =		(9 | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX);
	final byte STXPROTOCOL_DATA_TYPE_FLOAT = 	10;
	final byte STXPROTOCOL_DATA_TYPE_DOUBLE = 	11;
	final byte STXPROTOCOL_DATA_TYPE_UTF8_PAIR = 		(STXPROTOCOL_DATA_TYPE_UTF8 | STXPROTOCOL_DATA_TYPE_FLAG_PAIR | STXPROTOCOL_DATA_TYPE_FLAG_SECOND_PREFIX);
	final byte STXPROTOCOL_DATA_TYPE_UNICODE_PAIR = 	(STXPROTOCOL_DATA_TYPE_UNICODE | STXPROTOCOL_DATA_TYPE_FLAG_PAIR | STXPROTOCOL_DATA_TYPE_FLAG_SECOND_PREFIX);
	final byte STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR = 	(STXPROTOCOL_DATA_TYPE_DWORD | STXPROTOCOL_DATA_TYPE_FLAG_PAIR | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX);
	
	byte[] GetCompleteData()
	{
		byte nPrefixLen = GetCompactIntegerLen(m_nCurentWriteOffset);
		byte[] byAll = new byte[m_nCurentWriteOffset + nPrefixLen];
		int nLength = m_nCurentWriteOffset;
		int nIndex = 0;
		while(nLength > 0)
		{
			byte nByteToWrite = (byte)(nLength & 0x0000007F);
			nLength >>= 7;
			if(nLength > 0)
				nByteToWrite |= 0x80;

			byAll[nIndex] = nByteToWrite;
			nIndex++;
		}
		
		System.arraycopy(m_pBuffer, 0, byAll, nPrefixLen, m_nCurentWriteOffset);
		return byAll;
	}
	static byte GetCompactIntegerLen( int nValue )
	{
		if(nValue == 0)
			return 1;

		byte nByteLen = 0;
		while(nValue > 0)
		{
			nValue >>= 7;
			nByteLen++;
		}
		return nByteLen;
	}
	int Expand( int nAdd )
	{
		// 根据需要，动态增加缓冲区空间
		while(m_nCurentWriteOffset + nAdd > m_nBufferLen)
		{
			int nNewBufferLen = m_nBufferLen * 2;
			byte[] newBuffer = new byte[nNewBufferLen];
			System.arraycopy(m_pBuffer, 0, newBuffer, 0, m_nBufferLen);
			m_pBuffer = newBuffer;
			m_nBufferLen = nNewBufferLen;
		}
		return nAdd;
	}
	void UpdatePrefix()
	{
            int contentLength = m_nCurentWriteOffset - _maxPrefixLength;
            byte prefixLen = GetCompactIntegerLen(contentLength);
            int prefixStartPos = _maxPrefixLength - prefixLen - 1;
            int prefixEndPos = _maxPrefixLength - 1;
 
            m_nPrefixLen = prefixLen;
            if(contentLength == 0)
            {
                m_pBuffer[prefixEndPos] = (byte)0;
            }

		while(contentLength > 0)
		{
			byte nByteToWrite = (byte)(contentLength & 0x0000007F);
			contentLength >>= 7;
			if(contentLength > 0)
				nByteToWrite |= 0x80;

			m_pBuffer[prefixEndPos] = nByteToWrite;
                        prefixEndPos--;
		}
	}
	
	public STXProtocol()
	{
	}
	public STXProtocol(byte[] initialData)
	{
		Decode(initialData);
	}
	
	int DecodeCompactInteger(byte[] initialData, int offset, byte[] prefixLen )
	{
		if(initialData == null)
			return -1;

		byte nDecodedBytes = 0;

		int nValue = 0;
		int nValueUnit = 0;
		while(((initialData[offset + nDecodedBytes]) & 0x80) == 0x80)
		{
			nValueUnit = (initialData[offset + nDecodedBytes] & 0x7F);
			nValueUnit <<= nDecodedBytes;
			nValue += nValueUnit;
			nDecodedBytes++;

			if(nDecodedBytes >= STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE)
				return -1;
		}
		nValueUnit = (initialData[offset + nDecodedBytes] & 0x7F);
		nValueUnit <<= (nDecodedBytes * 7);
		nValue += nValueUnit;
		nDecodedBytes++;
		if(nDecodedBytes >= STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE)
			return -1;

		if(prefixLen != null)
			prefixLen[0] = nDecodedBytes;

		return nValue;
	}
	private void ResetPosition()
	{
		m_nCurentWriteOffset = _maxPrefixLength;
		m_nCurentReadOffset = _maxPrefixLength;
	}
	public boolean Decode(byte[] initialData)
	{
		byte []nLengthBytes = new byte[1];
		int nObjectContentLength = DecodeCompactInteger(initialData, 0, nLengthBytes);
		if(nObjectContentLength == -1)
			return false;
		
		if(initialData.length < nObjectContentLength + nLengthBytes[0])
			return false;

		ResetPosition();
		Expand(nObjectContentLength + _maxPrefixLength);
                InternalWriteRawData(initialData, 0, _maxPrefixLength - nLengthBytes[0], nObjectContentLength + nLengthBytes[0]);
                m_nCurentWriteOffset = _maxPrefixLength + nObjectContentLength;
                //CRC skip 1 byte
                _crc = m_pBuffer[_maxPrefixLength];
                m_nCurentReadOffset = _maxPrefixLength + 1;

                m_nPrefixLen = nLengthBytes[0];
		
		return true;
	}
	
        private void InternalWriteRawData(byte []data, int offset, int offsetInternalBuffer, int length)
	{
            System.arraycopy(data, offset, m_pBuffer, offsetInternalBuffer, length);
	}
                
	private void InternalAppendRawData(byte []data)
	{
		InternalAppendRawData(data, 0, data.length);
	}
	private void InternalAppendRawData(byte []data, int offset, int length)
	{
            //Expand(data.length);
	    System.arraycopy(data, offset, m_pBuffer, m_nCurentWriteOffset, length);
	    m_nCurentWriteOffset += length;	
	    UpdatePrefix();
	}
	private void AppendRawByte(byte bVal)
	{
            Expand(1);
            m_pBuffer[m_nCurentWriteOffset] = bVal;
	    m_nCurentWriteOffset ++;    
	    UpdatePrefix();
	}
	private void WriteCompactInteger(int nLength)
	{
		Expand(GetCompactIntegerLen(nLength));
		if(nLength == 0)
		{
			AppendRawByte((byte)0);
		}

		while(nLength > 0)
		{
			byte nByteToWrite = (byte)(nLength & 0x0000007F);
			nLength >>= 7;
			if(nLength > 0)
				nByteToWrite |= 0x80;

			AppendRawByte(nByteToWrite);
		}
	}
	public void AppendData(int value)
	{
		int nLen = 4;
		ByteBuffer buffer = ByteBuffer.allocate(nLen).order(m_byteOrder);
	    buffer.putInt(value);
	    
	    AppendRawByte(STXPROTOCOL_DATA_TYPE_DWORD);
	    InternalAppendRawData(buffer.array());
	}
	public void AppendData(byte value)
	{   
	    AppendRawByte(STXPROTOCOL_DATA_TYPE_BYTE);
	    AppendRawByte(value);
	}
	public void AppendData(short value)
	{   
		int nLen = 2;
		ByteBuffer buffer = ByteBuffer.allocate(nLen).order(m_byteOrder);
	    buffer.putShort(value);

	    AppendRawByte(STXPROTOCOL_DATA_TYPE_WORD);
	    InternalAppendRawData(buffer.array());
	}
	public void AppendData(long value)
	{   
		int nLen = 8;
		ByteBuffer buffer = ByteBuffer.allocate(nLen).order(m_byteOrder);
	    buffer.putLong(value);

	    AppendRawByte(STXPROTOCOL_DATA_TYPE_I64);
	    InternalAppendRawData(buffer.array());
	}
	public void AppendData(float value)
	{   
		int nLen = 4;
		ByteBuffer buffer = ByteBuffer.allocate(nLen).order(m_byteOrder);
	    buffer.putFloat(value);

	    AppendRawByte(STXPROTOCOL_DATA_TYPE_FLOAT);
	    InternalAppendRawData(buffer.array());
	}
	public void AppendData(double value)
	{   
		int nLen = 8;
		ByteBuffer buffer = ByteBuffer.allocate(nLen).order(m_byteOrder);
	    buffer.putDouble(value);

	    AppendRawByte(STXPROTOCOL_DATA_TYPE_DOUBLE);
	    InternalAppendRawData(buffer.array());
	}
	public void AppendData(UUID value)
	{   
		ByteBuffer buffer = ByteBuffer.allocate(16).order(m_byteOrder);
	    buffer.putLong(value.getMostSignificantBits());
	    buffer.putLong(value.getLeastSignificantBits());
	   
	    AppendRawByte(STXPROTOCOL_DATA_TYPE_GUID);
	    InternalAppendRawData(buffer.array());
	}
	public void AppendData(String value) throws UnsupportedEncodingException
	{
		byte byData[] = value.getBytes("UTF-8");  
		int nLen = byData.length;
	    AppendRawByte(STXPROTOCOL_DATA_TYPE_UTF8);
		WriteCompactInteger(nLen);
		InternalAppendRawData(byData);    
	}
        public void AppendUnicodeString(String value) throws UnsupportedEncodingException
	{
		byte byData[] = value.getBytes("UTF-16LE");  
		int nLen = byData.length;
	    AppendRawByte(STXPROTOCOL_DATA_TYPE_UNICODE);
		WriteCompactInteger(nLen);
		InternalAppendRawData(byData);
	}
	public void AppendRawData(byte[] value)
	{   
		int nLen = value.length;
	    AppendRawByte(STXPROTOCOL_DATA_TYPE_RAW);
		WriteCompactInteger(nLen);
		InternalAppendRawData(value);    
	}
	//UTF-8 String Pair
	public void AppendStringPair(String value1, String value2) throws UnsupportedEncodingException
	{
	    AppendRawByte(STXPROTOCOL_DATA_TYPE_UNICODE_PAIR);
		byte byData1[] = value1.getBytes("UTF-8");  
		int nLen1 = byData1.length;
		WriteCompactInteger(nLen1);
		InternalAppendRawData(byData1);    

		byte byData2[] = value2.getBytes("UTF-8");  
		int nLen2 = byData2.length;
		WriteCompactInteger(nLen2);
		InternalAppendRawData(byData2);    
	}
	//UTF-8 String to DWORD pair
	public void AppendStringDWORDPair(String value1, int value2) throws UnsupportedEncodingException
	{
	    AppendRawByte(STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR);
		byte byData1[] = value1.getBytes("UTF-8");  
		int nLen1 = byData1.length;
		WriteCompactInteger(nLen1);
		InternalAppendRawData(byData1);    

		int nLen = 4;
		ByteBuffer buffer = ByteBuffer.allocate(nLen).order(m_byteOrder);
	    buffer.putInt(value2);
	    
	    InternalAppendRawData(buffer.array());
	}
	
	private boolean IsValidDataType( byte nType )
	{
		if(m_nCurentReadOffset >= m_nCurentWriteOffset)
			return false;

		byte nTypeInData =  m_pBuffer[m_nCurentReadOffset];
		return nTypeInData == nType;
	}
	
	boolean SkipTypeIndicator(byte[] pTypeSkipped)
	{
		if(m_nCurentWriteOffset - m_nCurentReadOffset > 0)
		{
			if(pTypeSkipped != null)
				pTypeSkipped[0] =  m_pBuffer[m_nCurentReadOffset];

			m_nCurentReadOffset++;
			return true;
		}
		return false;
	}
	int GetTypedDataLen( byte nType )
	{
		if((nType & STXPROTOCOL_DATA_TYPE_FLAG_PAIR) == STXPROTOCOL_DATA_TYPE_FLAG_PAIR)
			return GetTypedDataLen((byte)(nType & 0x0F));

		switch(nType)
		{
		case STXPROTOCOL_DATA_TYPE_BYTE:
			return 1;
		case STXPROTOCOL_DATA_TYPE_WORD:
			return 2;
		case STXPROTOCOL_DATA_TYPE_DWORD:
			return 4;
		case STXPROTOCOL_DATA_TYPE_I64:
			return 8;
		case STXPROTOCOL_DATA_TYPE_FLOAT:
			return 4;
		case STXPROTOCOL_DATA_TYPE_DOUBLE:
			return 8;
		case STXPROTOCOL_DATA_TYPE_GUID:
			return 16;
		}
		return 0;
	}
	void CheckDataAvailability( byte nType ) throws Exception
	{
		if(m_nCurentWriteOffset - m_nCurentReadOffset < GetTypedDataLen(nType))
		{
			throw new Exception("Not Enough Data To Read!");
		}
	}
	public byte GetNextByte() throws Exception
	{
		final byte nType = STXPROTOCOL_DATA_TYPE_BYTE;
		if(!IsValidDataType(nType))
		{
			return 0;
		}

		SkipTypeIndicator(null);
		CheckDataAvailability(nType);

		byte nValue =  m_pBuffer[m_nCurentReadOffset];
		m_nCurentReadOffset += 1;
		return nValue;
	}
	public short GetNextWORD() throws Exception
	{
		final byte nType = STXPROTOCOL_DATA_TYPE_WORD;
		if(!IsValidDataType(nType))
		{
			return 0;
		}

		SkipTypeIndicator(null);
		CheckDataAvailability(nType);

		
		ByteBuffer wrapped = ByteBuffer.wrap(m_pBuffer, m_nCurentReadOffset, 2);
		wrapped.order(m_byteOrder);
		short nValue = wrapped.getShort();
		m_nCurentReadOffset += 2;
		return nValue;
	}
	public int GetNextDWORD() throws Exception
	{
		final byte nType = STXPROTOCOL_DATA_TYPE_DWORD;
		if(!IsValidDataType(nType))
		{
			return 0;
		}

		SkipTypeIndicator(null);
		CheckDataAvailability(nType);

		
		ByteBuffer wrapped = ByteBuffer.wrap(m_pBuffer, m_nCurentReadOffset, 4);
		wrapped.order(m_byteOrder);
		int nValue = wrapped.getInt();
		m_nCurentReadOffset += 4;
		return nValue;
	}
	public long GetNextI64() throws Exception
	{
		final byte nType = STXPROTOCOL_DATA_TYPE_I64;
		if(!IsValidDataType(nType))
		{
			return 0;
		}

		SkipTypeIndicator(null);
		CheckDataAvailability(nType);
	
		ByteBuffer wrapped = ByteBuffer.wrap(m_pBuffer, m_nCurentReadOffset, 8);
		wrapped.order(m_byteOrder);
		long nValue = wrapped.getLong();
		m_nCurentReadOffset += 8;
		return nValue;
	}
	public float GetNextFloat() throws Exception
	{
		final byte nType = STXPROTOCOL_DATA_TYPE_FLOAT;
		if(!IsValidDataType(nType))
		{
			return 0;
		}

		SkipTypeIndicator(null);
		CheckDataAvailability(nType);
	
		ByteBuffer wrapped = ByteBuffer.wrap(m_pBuffer, m_nCurentReadOffset, 4);
		wrapped.order(m_byteOrder);
		float nValue = wrapped.getFloat();
		m_nCurentReadOffset += 4;
		return nValue;
	}
	public double GetNextDouble() throws Exception
	{
		final byte nType = STXPROTOCOL_DATA_TYPE_DOUBLE;
		if(!IsValidDataType(nType))
		{
			return 0;
		}

		SkipTypeIndicator(null);
		CheckDataAvailability(nType);
	
		ByteBuffer wrapped = ByteBuffer.wrap(m_pBuffer, m_nCurentReadOffset, 8);
		wrapped.order(m_byteOrder);
		double nValue = wrapped.getFloat();
		m_nCurentReadOffset += 8;
		return nValue;
	}
	public String GetNextString() throws Exception
	{
		String result = null;
		if(IsValidDataType(STXPROTOCOL_DATA_TYPE_UTF8))
		{
			SkipTypeIndicator(null);
			byte []nLengthBytes = new byte[1];
			int nStringLen = DecodeCompactInteger(m_pBuffer, m_nCurentReadOffset, nLengthBytes);
			if(nStringLen == -1)
			{
				throw new Exception("Invalid Length Prefix");
			}
			
			byte []stringData = new byte[nStringLen];
			System.arraycopy(m_pBuffer, m_nCurentReadOffset + nLengthBytes[0], stringData, 0, nStringLen);
			result = new String(stringData, "UTF-8");
			
			m_nCurentReadOffset += nLengthBytes[0];
			m_nCurentReadOffset += nStringLen;

		}
		else if(IsValidDataType(STXPROTOCOL_DATA_TYPE_UNICODE))
		{
                    	SkipTypeIndicator(null);
			byte []nLengthBytes = new byte[1];
			int nStringLen = DecodeCompactInteger(m_pBuffer, m_nCurentReadOffset, nLengthBytes);
			if(nStringLen == -1)
			{
				throw new Exception("Invalid Length Prefix");
			}
			
			byte []stringData = new byte[nStringLen];
			System.arraycopy(m_pBuffer, m_nCurentReadOffset + nLengthBytes[0], stringData, 0, nStringLen);
			result = new String(stringData, "UTF-16LE");
			
			m_nCurentReadOffset += nLengthBytes[0];
			m_nCurentReadOffset += nStringLen;
		}
		
		return result;
	}
        public int GetDataLength() {
            return m_nCurentWriteOffset - _maxPrefixLength + m_nPrefixLen;
        }
        
        public byte[] GetBuffer() {
            return m_pBuffer;
        }
        
        public int GetDataOffset() {
            return _maxPrefixLength - m_nPrefixLen;
        }
	
}
