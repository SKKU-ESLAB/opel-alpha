package opel.android.comm;

import java.util.UUID;

public class OpelUtil {
	public static final String TAG = "CMFW";
	public static long char2long(char c){
		return c & 0x00000000000000FFL;
	}
	public static UUID name2uuid(String str){
		char[] inputString = str.toCharArray();
    	int length = inputString.length;
    	long[] longData = new long[2];
    	for(int i=0; i<2; i++){
    		longData[i] = 0;
    		if(i*8 < length)
    			longData[i] = char2long(inputString[i*8]) <<56;
    		if(i*8+1 < length)
    			longData[i] |= char2long(inputString[i*8+1]) << 48; 
    		if(i*8+2 < length)
    			longData[i] |= char2long(inputString[i*8+2]) << 40;
    		if(i*8+3 < length)
    			longData[i] |= char2long(inputString[i*8+3]) << 32;
    		if(i*8+4 < length)
    			longData[i] |= char2long(inputString[i*8+4]) << 24;
    		if(i*8+5 < length)
    			longData[i] |= char2long(inputString[i*8+5]) << 16;
    		if(i*8+6 < length)
    			longData[i] |= char2long(inputString[i*8+6]) << 8;
    		if(i*8+7 < length)
    			longData[i] |= char2long(inputString[i*8+7]);
    				
    	}
    	
    	return new UUID(longData[0], longData[1]);
	}
}
