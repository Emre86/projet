package applet;


import javacard.framework.*;
import javacard.security.*;
import javacardx.crypto.*;


public class TheApplet extends Applet {


	static final byte UPDATECARDKEY						= (byte)0x14;
	static final byte UNCIPHERFILEBYCARD				= (byte)0x13;
	static final byte CIPHERFILEBYCARD					= (byte)0x12;
	static final byte CIPHERANDUNCIPHERNAMEBYCARD		= (byte)0x11;
	static final byte READFILEFROMCARD					= (byte)0x10;
	static final byte WRITEFILETOCARD					= (byte)0x09;
	static final byte UPDATEWRITEPIN					= (byte)0x08;
	static final byte UPDATEREADPIN						= (byte)0x07;
	static final byte DISPLAYPINSECURITY				= (byte)0x06;
	static final byte DESACTIVATEACTIVATEPINSECURITY	= (byte)0x05;
	static final byte ENTERREADPIN						= (byte)0x04;
	static final byte ENTERWRITEPIN						= (byte)0x03;
	static final byte READNAMEFROMCARD					= (byte)0x02;
	static final byte WRITENAMETOCARD					= (byte)0x01;

	final static byte  PIN_VERIFY_WRITE   				= (byte) 0x20;
	final static byte  PIN_VERIFY_READ   				= (byte) 0x21;
	
	final static short SW_PIN_VERIFICATION_REQUIRED	    = 0x6301 ;
	final static short SW_VERIFIACTION_FAILED 			= 0x6300;
	
	static byte[] data               					= new byte[1024];
	static byte[] fichier								= new byte[32385 + 1];
	OwnerPIN pinWrite ;
	OwnerPIN pinRead ;
	
	/* Carte activer ou desactiver */

	final static byte activatePin = (byte) 0x41 ;
	final static byte desactivatePin = (byte) 0x40 ;
	
	static byte desactivateActivatePin = (byte) 0x41 ; 
	
	
	/* Variable BEGIN CONTINUE END   */
	
	static byte P1_NB_TOTAL = (byte) 0x00 ;
	static byte P2_N_PACKET = (byte) 0x00 ;
	static final byte P_LECTURE  = (byte) 0x60 ;
	static final byte P_DECODE = (byte) 0x70; 
	static final byte P_ENCODE = (byte) 0x80;
	
	/* CRYPTO */
	
	static final byte[] theDESKey = new byte[] { (byte)0xCA, (byte)0xCA, (byte)0xCA, (byte)0xCA, (byte)0xCA, (byte)0xCA, (byte)0xCA, (byte)0xCA };

    // Ciphers
    private Cipher cDES_ECB_NOPAD_enc, cDES_ECB_NOPAD_dec;

    // Key
    private Key secretDESKey;

    //Tests
    boolean keyDES, DES_ECB_NOPAD, DES_CBC_NOPAD;
	
	
	/* FIN DE CRYPTO */
	
	protected TheApplet() {
		byte[] pincodeWrite = {(byte)0x30,(byte)0x30,(byte)0x30,(byte)0x30}; // PIN code "0000"
		byte[] pincodeRead 	= {(byte)0x31,(byte)0x31,(byte)0x31,(byte)0x31}; // PIN code "1111"
		
		pinWrite = new OwnerPIN((byte)3,(byte)8);  				// 3 tries 8=Max Size
		pinWrite.update(pincodeWrite,(short)0,(byte)4); 				// from pincode, offset 0, length 4
	
	
		pinRead = new OwnerPIN((byte)3,(byte)8);  				// 3 tries 8=Max Size
		pinRead.update(pincodeRead,(short)0,(byte)4); 				// from pincode, offset 0, length 4
	
		initKeyDES(); 
	    initDES_ECB_NOPAD(); 
	
		this.register();
	}
    /*   CRYPTO   */
	private void initKeyDES() {
	    try {
		    secretDESKey = KeyBuilder.buildKey(KeyBuilder.TYPE_DES, KeyBuilder.LENGTH_DES, false);
		    ((DESKey)secretDESKey).setKey(theDESKey,(short)0);
		    keyDES = true;
	    } catch( Exception e ) {
		    keyDES = false;
	    }
    }

    private void initDES_ECB_NOPAD() {
	    if( keyDES ) try {
		    cDES_ECB_NOPAD_enc = Cipher.getInstance(Cipher.ALG_DES_ECB_NOPAD, false);
		    cDES_ECB_NOPAD_dec = Cipher.getInstance(Cipher.ALG_DES_ECB_NOPAD, false);
		    cDES_ECB_NOPAD_enc.init( secretDESKey, Cipher.MODE_ENCRYPT );
		    cDES_ECB_NOPAD_dec.init( secretDESKey, Cipher.MODE_DECRYPT );
		    DES_ECB_NOPAD = true;
	    } catch( Exception e ) {
		    DES_ECB_NOPAD = false;
	    }
    }
	
	/* FIN DE CRYPTO */
	
	public static void install(byte[] bArray, short bOffset, byte bLength) throws ISOException {
		new TheApplet();
	} 


	public boolean select() {
		return true;
	} 


	public void deselect() {
	}

	public void process(APDU apdu) throws ISOException {
		if( selectingApplet() == true )
			return;

		byte[] buffer = apdu.getBuffer();

		switch( buffer[1] ) 	{
			case UPDATECARDKEY: updateCardKey( apdu ); break;
			case UNCIPHERFILEBYCARD: uncipherFileByCard( apdu ); break;
			case CIPHERFILEBYCARD: cipherFileByCard( apdu ); break;
			case CIPHERANDUNCIPHERNAMEBYCARD: cipherAndUncipherNameByCard( apdu ); break;
			case READFILEFROMCARD: readFileFromCard( apdu ); break;
			case WRITEFILETOCARD: writeFileToCard( apdu ); break;
			case UPDATEWRITEPIN: updateWritePIN( apdu ); break;
			case UPDATEREADPIN: updateReadPIN( apdu ); break;
			case DISPLAYPINSECURITY: displayPINSecurity( apdu ); break;
			case DESACTIVATEACTIVATEPINSECURITY: desactivateActivatePINSecurity( apdu ); break;
			case ENTERREADPIN: enterReadPIN( apdu ); break;
			case ENTERWRITEPIN: enterWritePIN( apdu ); break;
			case READNAMEFROMCARD: readNameFromCard( apdu ); break;
			case WRITENAMETOCARD: writeNameToCard( apdu ); break;
			default: ISOException.throwIt(ISO7816.SW_INS_NOT_SUPPORTED);
		}
	}

	
	
	public void WriteVerify(APDU apdu){
		apdu.setIncomingAndReceive();
		byte[] buffer = apdu.getBuffer();
		if (!pinWrite.check ( buffer ,(byte)5,buffer[(byte)4]))
			ISOException.throwIt( SW_VERIFIACTION_FAILED);
	}
	
	public void ReadVerify(APDU apdu){
		apdu.setIncomingAndReceive();
		byte[] buffer = apdu.getBuffer();
		if (!pinRead.check ( buffer ,(byte)5,buffer[(byte)4]))
			ISOException.throwIt( SW_VERIFIACTION_FAILED);
	
	}
	
	void updateCardKey( APDU apdu ) {
	}


	void uncipherFileByCard( APDU apdu ) {
		Cipher cipher = cDES_ECB_NOPAD_dec ;
		short keyLength = KeyBuilder.LENGTH_DES ;
	
		apdu.setIncomingAndReceive();
		byte[] buffer = apdu.getBuffer();
		byte[] tmp = new byte[(short)buffer[4]];
	
		Util.arrayCopy(buffer,(short)5,tmp,(short)0,(short)buffer[4]);
		cipher.doFinal(tmp,(short)0,(short)tmp.length,buffer,(short)0);
		
		apdu.setOutgoingAndSend((short)0,(short)tmp.length);
	

	}


	void cipherFileByCard( APDU apdu ) {
		Cipher cipher = cDES_ECB_NOPAD_enc ;
		short keyLength = KeyBuilder.LENGTH_DES ;
	
		apdu.setIncomingAndReceive();
		byte[] buffer = apdu.getBuffer();
		byte[] tmp = new byte[(short)buffer[4]];
	
		Util.arrayCopy(buffer,(short)5,tmp,(short)0,(short)buffer[4]);
		cipher.doFinal(tmp,(short)0,(short)tmp.length,buffer,(short)0);
		
		apdu.setOutgoingAndSend((short)0,(short)tmp.length);
	
	}


	void cipherAndUncipherNameByCard( APDU apdu ) {
		Cipher cipher;
		short keyLength = KeyBuilder.LENGTH_DES ;
	
		apdu.setIncomingAndReceive();
		byte[] buffer = apdu.getBuffer();
		byte[] tmp = new byte[(short)buffer[4]];
		
		if ( buffer[2] == P_DECODE ){
			cipher = cDES_ECB_NOPAD_dec ;
		}
		else{
			cipher = cDES_ECB_NOPAD_enc ;
		}
		Util.arrayCopy(buffer,(short)5,tmp,(short)0,(short)buffer[4]);
		cipher.doFinal(tmp,(short)0,(short)tmp.length,buffer,(short)0);
		
		apdu.setOutgoingAndSend((short)0,(short)tmp.length);
	
	}


	void readFileFromCard( APDU apdu ) {
		
		short POSITION ;
		short VAL255 = (short) 255 ;
		apdu.setIncomingAndReceive();
		byte [] buffer = apdu.getBuffer();
		POSITION = (short) buffer[3] ;
		if ( buffer[2] != P_LECTURE ){
			Util.arrayCopy(fichier, (short)((short)1 + (short)(VAL255 * POSITION))  , buffer , (short)0 , (short) ( 255 & (short) 0x00ff) ); 
			apdu.setOutgoingAndSend((short) 0 , (short) 255 );
		}
		else {
			Util.arrayCopy(fichier, (short) 0 , buffer , (short)0 , (short) 1 ); 
			apdu.setOutgoingAndSend((short) 0 , (short) 1 );
		}
	}
	
	


	void writeFileToCard( APDU apdu ) {
		
		apdu.setIncomingAndReceive();
		byte [] buffer = apdu.getBuffer();
		
		P1_NB_TOTAL = buffer[2] ;
		P2_N_PACKET = buffer[3] ;
		fichier[0] = buffer[2];
		Util.arrayCopy(buffer, (byte) 5 , fichier ,(short) ((((short) P2_N_PACKET) * ((short)255)) + 1) , (short) (255 & (short) 0x00ff)); 
	}

	void updateWritePIN( APDU apdu ) {
		byte [] buffer ;
		byte [] pinWriteNewCode = { (byte)0x34, (byte)0x34, (byte)0x34, (byte)0x34 };
		if ( ! pinWrite.isValidated() ) 
				ISOException.throwIt(SW_PIN_VERIFICATION_REQUIRED);	
		apdu.setIncomingAndReceive();
		buffer = apdu.getBuffer();
		Util.arrayCopy(buffer,(byte)5,pinWriteNewCode,(short)0,(byte)4);
		pinWrite.update(pinWriteNewCode,(short)0,(byte)4); 
	}


	void updateReadPIN( APDU apdu ) {
		byte [] buffer ;
		byte [] pinReadNewCode = { (byte)0x34, (byte)0x34, (byte)0x34, (byte)0x34 };
		if ( ! pinRead.isValidated() ) 
				ISOException.throwIt(SW_PIN_VERIFICATION_REQUIRED);	
		apdu.setIncomingAndReceive();
		buffer = apdu.getBuffer();
		Util.arrayCopy(buffer,(byte)5,pinReadNewCode,(short)0,(byte)4);
		pinRead.update(pinReadNewCode,(short)0,(byte)4); 
		
	}


	void displayPINSecurity( APDU apdu ) {
		byte [] buffer = apdu.getBuffer();
		byte [] buf = { desactivateActivatePin };
		Util.arrayCopy(buf, (short) 0 , buffer , (short)0 , (byte) 1 ); 
		apdu.setOutgoingAndSend( (short) 0 , (byte) 1);
		/*
			FIXME
		
		if ( desactivateActivatePin == activatePin ) {
			buf = { 0x38 , 0x41 , 0x43 , 0x54 , 0x49 , 0x56 , 0x41 , 0x54, 0x45 } ;
			Util.arrayCopy(buf, (short) 0 , buffer , (short)0 , (byte) buf[0] ); 
		}
		else {
			buf = { 0x44 , 0x45 , 0x53 , 0x41 , 0x43 , 0x54 , 0x49, 0x56 , 0x41 , 0x54 , 0x45 } ;
		}
		apdu.setOutgoingAndSend( (short) 0 , (byte) 1);*/
	}


	void desactivateActivatePINSecurity( APDU apdu ) {
		if (desactivateActivatePin == activatePin ) {
			desactivateActivatePin = desactivatePin ;
		}
		else{
			desactivateActivatePin = activatePin ;
		}
	}


	void enterReadPIN( APDU apdu ) {
		ReadVerify(apdu);
	}


	void enterWritePIN( APDU apdu ) {
		WriteVerify(apdu);
	}


	void readNameFromCard( APDU apdu ) {
		if ( desactivateActivatePin == activatePin )
			if ( ! pinRead.isValidated() ) 
				ISOException.throwIt(SW_PIN_VERIFICATION_REQUIRED);				
		byte [] buffer = apdu.getBuffer();
		Util.arrayCopy(data, (short) 1 , buffer , (short)0 , (byte) data[0] ); 
		apdu.setOutgoingAndSend((short) 0 , data[0]);
	}
	


	void writeNameToCard( APDU apdu ) {
		if ( desactivateActivatePin == activatePin )
			if ( ! pinWrite.isValidated() ) 
				ISOException.throwIt(SW_PIN_VERIFICATION_REQUIRED);	
		apdu.setIncomingAndReceive();
		byte [] buffer = apdu.getBuffer();
		Util.arrayCopy(buffer,(byte)4,data,(short)0,(byte)(1+buffer[4]));
	}


}
